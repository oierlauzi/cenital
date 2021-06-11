#include <MixEffect.h>

#include <Transitions/Mix.h>
#include <Transitions/DVE.h>
#include <Overlays/Keyer.h>

#include <zuazo/Player.h>
#include <zuazo/Signal/DummyPad.h>
#include <zuazo/Renderers/Compositor.h>
#include <zuazo/Layers/VideoSurface.h>

#include <array>
#include <vector>
#include <utility>
#include <bitset>

namespace Cenital {

using namespace Zuazo;

static void openHelper(ZuazoBase& base, std::unique_lock<Instance>* lock) {
	if(lock) {
		base.asyncOpen(*lock);
	} else {
		base.open();
	}
}

static void closeHelper(ZuazoBase& base, std::unique_lock<Instance>* lock) {
	if(lock) {
		base.asyncClose(*lock);
	} else {
		base.close();
	}
}



/*
 * MixEffectImpl
 */

struct MixEffectImpl {
	class Overlay {
	public:
		Overlay(std::unique_ptr<Overlays::Base> overlay = nullptr)
			: m_overlay(std::move(overlay))
			, m_state()
		{
		}
		Overlay(Overlay&& other) = default;
		~Overlay() = default;

		Overlay& operator=(Overlay&& other) = default;


		std::unique_ptr<Overlays::Base> setOverlay(std::unique_ptr<Overlays::Base> overlay) {
			std::swap(m_overlay, overlay);
			return overlay;
		}

		Overlays::Base* getOverlay() noexcept {
			return m_overlay.get();
		}

		const Overlays::Base* getOverlay() const noexcept {
			return m_overlay.get();
		}


		void setVisible(bool visibility) noexcept {
			m_state[VISIBLE_BIT] = visibility;
		}

		bool getVisible() const noexcept {
			return m_state[VISIBLE_BIT];
		}

		bool isRendered(MixEffect::OutputBus bus) const {
			bool result = (m_overlay != nullptr);

			if(result) {
				switch(bus) {
				case MixEffect::OutputBus::program:
					result = getVisible();
					break;

				case MixEffect::OutputBus::preview:
					//If visible and wont's transion
					//or not visible and will transition,
					//This is, XOR or inequality
					result = getVisible() != getTransition();
					break;

				default:
					throw std::runtime_error("Invalid bus");
				}
			}

			return result;
		}


		void setTransition(bool transition) noexcept {
			m_state[TRANSITION_ENABLED_BIT] = transition;
		}

		bool getTransition() const noexcept {
			return m_state[TRANSITION_ENABLED_BIT];
		}

	private:
		enum Bits {
			VISIBLE_BIT,
			TRANSITION_ENABLED_BIT,

			BIT_COUNT
		};

		std::unique_ptr<Overlays::Base>		m_overlay;
		std::bitset<BIT_COUNT>				m_state;

	};

	using Input = Signal::DummyPad<Video>;
	using Output = Signal::DummyPad<Video>;
	using Compositor = Renderers::Compositor;
	using VideoSurface = Layers::VideoSurface;
	using TransitionMap = std::unordered_map<std::string_view, std::unique_ptr<Transitions::Base>>;
	
	static constexpr auto UPDATE_PRIORITY = Instance::playerPriority; //Animation-like
	static constexpr auto OUTPUT_BUS_CNT = static_cast<size_t>(MixEffect::OutputBus::count);
	static constexpr auto OVERLAY_CNT = static_cast<size_t>(MixEffect::OverlaySlot::count);

	std::reference_wrapper<MixEffect>				owner;

	std::vector<Input>								inputs;
	std::array<Output, OUTPUT_BUS_CNT>				outputs;

	std::array<Compositor, OUTPUT_BUS_CNT> 			compositors;
	std::array<Compositor, OUTPUT_BUS_CNT> 			intermediateCompositors;
	Compositor&										referenceCompositor;

	std::array<VideoSurface, OUTPUT_BUS_CNT> 		backgroundLayers;
	VideoSurface									intermediateLayer;

	TransitionMap									transitions;
	TransitionMap::iterator							selectedTransition;
	MixEffect::OutputBus							transitionSlot;

	std::array<std::vector<Overlay>, OVERLAY_CNT>	overlays;

	MixEffectImpl(	MixEffect& owner, 
					Instance& instance,
					const std::string& name )
		: owner(owner)
		, inputs{}
		, outputs{
			Output(owner, "pgmOut"), 
			Output(owner, "pvwOut") }
		, compositors{
			Compositor(instance, name + " - Program Compositor"),
			Compositor(instance, name + " - Preview Compositor") }
		, intermediateCompositors{
			Compositor(instance, name + " - Program Intermediate Compositor"),
			Compositor(instance, name + " - Preview Intermediate Compositor") }
		, referenceCompositor(compositors.front()) //Arbitrarily choosen
		, backgroundLayers{
			createBackgroundLayer(instance, name + " - Program Layer", referenceCompositor),
			createBackgroundLayer(instance, name + " - Preview Layer", referenceCompositor) }
		, intermediateLayer(createBackgroundLayer(instance, name + " - Intermediary Layer", referenceCompositor))
		, transitions()
		, selectedTransition(transitions.end())
		, transitionSlot(MixEffect::OutputBus::program)
		, overlays{}
	{
		//Route the signals
		for(size_t i = 0; i < OUTPUT_BUS_CNT; ++i) {
			outputs[i] << compositors[i];
		}

		//Configure the callbacks
		referenceCompositor.setViewportSizeCallback(
			std::bind(&MixEffectImpl::viewportSizeCallback, this, std::placeholders::_2)
		);
		
		//Configure the layers
		configureLayers(false);
	}

	~MixEffectImpl() = default;


	void moved(ZuazoBase& base) {
		owner = static_cast<MixEffect&>(base);

		for(auto& pad : inputs) {
			pad.setLayout(base);
		}

		for(auto& pad : outputs) {
			pad.setLayout(base);
		}
		
	}

	void open(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		auto& me = static_cast<MixEffect&>(base);
		assert(&owner.get() == &me);

		//Open everything
		for(size_t i = 0; i < OUTPUT_BUS_CNT; ++i) {
			openHelper(compositors[i], lock);
			openHelper(intermediateCompositors[i], lock);
			openHelper(backgroundLayers[i], lock);
		}
		openHelper(intermediateLayer, lock);

		for(auto& overlaySlot : overlays) {
			for(auto& overlay : overlaySlot) {
				if(overlay.getOverlay()) {
					openHelper(*overlay.getOverlay(), lock);
				}
			}
		}

		for(auto& transition : transitions) {
			if(transition.second) {
				if(transition.second) {
					openHelper(*transition.second, lock);
				}
			}
		}

		me.enableRegularUpdate(UPDATE_PRIORITY);
	}

	void asyncOpen(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		open(base, &lock);
		assert(lock.owns_lock());
	}

	void close(ZuazoBase& base, std::unique_lock<Instance>* lock = nullptr) {
		auto& me = static_cast<MixEffect&>(base);
		assert(&owner.get() == &me);
		
		me.disableRegularUpdate();

		//Close everyting
		for(size_t i = 0; i < OUTPUT_BUS_CNT; ++i) {
			closeHelper(compositors[i], lock);
			closeHelper(intermediateCompositors[i], lock);
			closeHelper(backgroundLayers[i], lock);
		}
		closeHelper(intermediateLayer, lock);

		for(auto& overlaySlot : overlays) {
			for(auto& overlay : overlaySlot) {
				if(overlay.getOverlay()) {
					closeHelper(*overlay.getOverlay(), lock);
				}
			}
		}

		for(auto& transition : transitions) {
			if(transition.second) {
				if(transition.second) {
					closeHelper(*transition.second, lock);
				}
			}
		}

	}

	void asyncClose(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		close(base, &lock);
		assert(lock.owns_lock());
	}

	void update() {
		auto* transition = getSelectedTransition();

		//Act as a player for the transition
		if(transition) {
			if(transition->isPlaying()) {
				const auto deltaTime = owner.get().getInstance().getDeltaT();

				//Advance the time for the transition
				transition->setRepeat(ClipBase::Repeat::none); //Ensure that it won't repeat
				transition->advanceNormalSpeed(deltaTime);

				//If transition has ended, cut. This will also stop and rewind the transition
				if(transition->getTime().time_since_epoch() == transition->getDuration()) {
					finishTransition();
				}
			}
		}

	}


	void setVideoMode(VideoBase& base, const VideoMode& videoMode) {
		auto& me = static_cast<MixEffect&>(base);
		assert(&owner.get() == &me);

		//Intermediate compositors will use a variant of the original videoMode
		VideoMode intermediateVideoMode = videoMode;
		intermediateVideoMode.setColorModel(Utils::MustBe<ColorModel>(ColorModel::rgb));
		intermediateVideoMode.setColorTransferFunction(Utils::MustBe<ColorTransferFunction>(ColorTransferFunction::linear));
		intermediateVideoMode.setColorSubsampling(Utils::MustBe<ColorSubsampling>(ColorSubsampling::rb444));
		intermediateVideoMode.setColorRange(Utils::MustBe<ColorRange>(ColorRange::full));
		intermediateVideoMode.setColorFormat(Utils::MustBe<ColorFormat>(ColorFormat::R16fG16fB16fA16f)); //TODO Maybe choose another one

		for(size_t i = 0; i < OUTPUT_BUS_CNT; ++i) {
			compositors[i].setVideoMode(videoMode);
			intermediateCompositors[i].setVideoMode(intermediateVideoMode);
		}
	}

	const std::vector<VideoMode>& getVideoModeCompatibility() const noexcept {
		return referenceCompositor.getVideoModeCompatibility();
	}

	VideoMode videoModeNegotiationCallback(const std::vector<VideoMode>& compatibility) {
		auto& me = owner.get();
		me.setVideoModeCompatibility(compatibility); //Will call the underlaying callback
		return me.getVideoMode();
	}



	void setScalingMode(ScalingMode scalingMode) {
		for(auto& layer : backgroundLayers) {
			layer.setScalingMode(scalingMode);
		}
	}

	void setScalingFilter(ScalingFilter scalingFilter) {
		for(auto& layer : backgroundLayers) {
			layer.setScalingFilter(scalingFilter);
		}
	}


	void setInputCount(MixEffect& mixEffect, size_t count) {
		if(inputs.size() != count) {
			assert(&mixEffect == &owner.get());

			//Remove all the pads as they might be reallocated
			for(auto& input : inputs) {
				mixEffect.removePad(input.getInput());
			}

			//Resize the array and rename the new pads (if any)
			if(inputs.size() > count) {
				//We'll remove elements
				inputs.erase(
					std::next(inputs.cbegin(), count), 
					inputs.cend()
				);
			} else {
				//We'll add elements
				inputs.reserve(count);
				while(inputs.size() < count) {
					inputs.emplace_back(mixEffect, Signal::makeInputName<Video>(inputs.size()));
				}
			}

			//Register all the pads again
			for(auto& input : inputs) {
				mixEffect.registerPad(input.getInput());
			}
		}

		assert(inputs.size() == count);
	}

	size_t getInputCount() const noexcept {
		return inputs.size();
	}

	MixEffect::Input& getInput(size_t idx) {
		return inputs.at(idx).getInput();
	}

	const MixEffect::Input& getInput(size_t idx) const {
		return inputs.at(idx).getInput();
	}


	MixEffect::Output& getOutput(MixEffect::OutputBus bus) noexcept {
		return outputs.at(static_cast<size_t>(bus)).getOutput();
	}

	const MixEffect::Output& getOutput(MixEffect::OutputBus bus) const noexcept {
		return outputs.at(static_cast<size_t>(bus)).getOutput();
	}


	void setBackground(MixEffect::OutputBus bus, size_t idx) {
		setSource(backgroundLayers.at(static_cast<size_t>(bus)).getInput(), idx);
	}

	size_t getBackground(MixEffect::OutputBus bus) const noexcept {
		return getSource(backgroundLayers.at(static_cast<size_t>(bus)).getInput());
	}


	void cut() {
		//Get the current sources
		auto src0 = backgroundLayers[0].getInput().getSource();
		auto src1 = backgroundLayers[1].getInput().getSource();

		//Write the sources swapping them
		backgroundLayers[0].getInput().setSource(src1);
		backgroundLayers[1].getInput().setSource(src0);

		//Change the states of the overlays
		for(auto& overlaySlot : overlays) {
			for(auto& overlay : overlaySlot) {
				if(overlay.getTransition()) {
					overlay.setVisible(!overlay.getVisible());
				}
			}
		}

		//Reset the transition
		setTransitionBar(0.0f);
	}

	void transition() {
		auto* transition = getSelectedTransition();		

		if(transition) {
			//Start playing
			transition->play();

			//Configure the layers if necessary
			if(!isTransitionConfigured()) {
				configureLayers(true);
			}
		} else {
			//No transition, simply cut
			cut();
		}
	}


	void setTransitionBar(float progress) {
		auto* transition = getSelectedTransition();	

		if(progress >= 1.0f) {
			//If we got to the end, reset the transition
			finishTransition();
		} else if(transition) {
			//Pause if a autotransition was taking place
			transition->pause();

			//Configure the selected transition and layers accordingly
			if(progress > 0) {
				//Advance the transition upto the point
				transition->setProgress(progress);

				//Configure the layers if necessary
				if(!isTransitionConfigured()) {
					configureLayers(true);
				}
			} else {
				//Rewind to the beggining
				transition->setTime(TimePoint());

				//Ensure that the layers are configured without a transition in place
				if(isTransitionConfigured()) {
					configureLayers(false);
				}
			}
		}
	}

	float getTransitionBar() const noexcept {
		auto* transition = getSelectedTransition();	
		return transition ? transition->getProgress() : 0;
	}


	void setTransitionSlot(MixEffect::OutputBus bus) {
		if(transitionSlot != bus) {
			transitionSlot = bus;

			//Reconfigure the layers if necessary
			if(isTransitionConfigured()) {
				configureLayers(true);
			}
		}
	}

	MixEffect::OutputBus getTransitionSlot() const noexcept {
		return transitionSlot;
	}


	void addTransition(std::unique_ptr<Transitions::Base> transition) {
		auto& mixEffect = owner.get();

		if(transition) {
			//Set same "openess"
			if(mixEffect.isOpen() != transition->isOpen()) {
				if(mixEffect.isOpen()) {
					transition->open();
				} else {
					transition->close();
				}
			}

			//Tie its size to the renderer's viewport size
			transition->setSize(referenceCompositor.getViewportSize());

			//Route the intermediate compositions to the transition
			transition->getPrevIn() << intermediateCompositors[static_cast<size_t>(MixEffect::OutputBus::program)];
			transition->getPostIn() << intermediateCompositors[static_cast<size_t>(MixEffect::OutputBus::preview)];

			//Store the reference to the transition, as rehashing might invalidate iterators
			const auto* selected = getSelectedTransition();
			transitions.emplace(transition->getName(), std::move(transition));
			setSelectedTransition(selected ? transitions.find(selected->getName()) : transitions.end());
		}
	}

	std::unique_ptr<Transitions::Base> removeTransition(std::string_view name) {
		std::unique_ptr<Transitions::Base> result;

		//Find the element to be deleted
		const auto ite = transitions.find(name);
		if(ite != transitions.cend()) {
			//If it is selected, unselect it
			if(ite == selectedTransition) {
				setSelectedTransition(transitions.end());
			}

			//Before destroying save the object
			result = std::move(ite->second);

			transitions.erase(ite);
		}

		return result;		
	}

	std::vector<Transitions::Base*> getTransitions() const {
		std::vector<Transitions::Base*> result;
		result.reserve(transitions.size());

		std::transform(
			transitions.cbegin(), transitions.cend(),
			std::back_inserter(result),
			[] (const TransitionMap::value_type& transition) -> Transitions::Base* {
				return transition.second.get();
			}
		);

		return result;
	}

	Transitions::Base* getTransition(std::string_view name) const {
		const auto ite = transitions.find(name);
		return ite != transitions.cend() ? ite->second.get() : nullptr;
	}


	void setSelectedTransition(TransitionMap::iterator ite) {
		//Ensure the previous transition is not playing
		auto* transition = getSelectedTransition();
		if(transition) {
			transition->stop();
		}

		//Select the new one
		selectedTransition = ite;

		//If transition is enabled, configure the new one
		if(isTransitionConfigured()) {
			configureLayers(true);
		}
	}

	void setSelectedTransition(std::string_view name) {
		setSelectedTransition(transitions.find(name));
	}

	Transitions::Base* getSelectedTransition() const noexcept {
		return (selectedTransition != transitions.end()) ? selectedTransition->second.get() : nullptr;
	}



	void setOverlayCount(MixEffect::OverlaySlot slot, size_t count) {
		constexpr std::array<const char*, OVERLAY_CNT> SLOT_NAMES = {
			"USK ", //Mind the space at the end
			"DSK "
		};

		const auto& me = owner.get();
		auto& selection = overlays.at(static_cast<size_t>(slot));
		const auto namePrefix = SLOT_NAMES.at(static_cast<size_t>(slot));

		//Take the appropiate action to resize. Note that only one of the following
		//statements will have effect (neither of them if count == size)

		//Add elements if necessary
		selection.reserve(count);
		while(selection.size() < count) {
			selection.emplace_back(
				Utils::makeUnique<Overlays::Keyer>(
					me.getInstance(),
					std::string(namePrefix) + toString(selection.size()),
					referenceCompositor.getViewportSize()
				)
			);

			//Set same openess
			if(me.isOpen()) {
				selection.back().getOverlay()->open();
			}
		}

		//Remove elements if necessary
		selection.erase(std::next(selection.cbegin(), count), selection.cend());

		assert(selection.size() == count);
	}

	size_t getOverlayCount(MixEffect::OverlaySlot slot) const {
		return overlays.at(static_cast<size_t>(slot)).size();
	}

	std::unique_ptr<Overlays::Base> setOverlay(MixEffect::OverlaySlot slot, size_t idx, std::unique_ptr<Overlays::Base> overlay) {
		auto result = findOverlay(slot, idx).setOverlay(std::move(overlay));
		configureLayers(isTransitionConfigured());
		return result;
	}

	Overlays::Base* getOverlay(MixEffect::OverlaySlot slot, size_t idx) {
		return findOverlay(slot, idx).getOverlay();
	}

	const Overlays::Base* getOverlay(MixEffect::OverlaySlot slot, size_t idx) const {
		return findOverlay(slot, idx).getOverlay();
	}

	void setOverlayVisible(MixEffect::OverlaySlot slot, size_t idx, bool visible) {
		findOverlay(slot, idx).setVisible(visible);
		configureLayers(isTransitionConfigured());
	}

	bool getOverlayVisible(MixEffect::OverlaySlot slot, size_t idx) const {
		return findOverlay(slot, idx).getVisible();
	}

	void setOverlayTransition(MixEffect::OverlaySlot slot, size_t idx, bool transition) {
		findOverlay(slot, idx).setTransition(transition);
		configureLayers(isTransitionConfigured());
	}

	bool getOverlayTransition(MixEffect::OverlaySlot slot, size_t idx) const {
		return findOverlay(slot, idx).getTransition();
	}

	void setOverlaySignal(MixEffect::OverlaySlot slot, size_t overlay, std::string_view port, size_t idx) {
		setSource(Signal::getInput<Video>(*getOverlay(slot, overlay), port), idx);
	}

	size_t getOverlaySignal(MixEffect::OverlaySlot slot, size_t overlay, std::string_view port) const noexcept {
		return getSource(Signal::getInput<Video>(*getOverlay(slot, overlay), port));
	}


private:
	void setSource(Signal::PadProxy<Signal::Input<Video>>& pad, size_t idx) {
		if(idx < inputs.size()) {
			pad << inputs.at(idx);
		} else {
			pad << Signal::noSignal;
		}
	}

	size_t getSource(const Signal::PadProxy<Signal::Input<Video>>& pad) const noexcept {
		size_t result = MixEffect::NO_SIGNAL;

		const auto source = pad.getSource();
		if(source) {
			//Find the source among the inputs. This could be done
			//with some ptr trickery in O(1) time, however, it would
			//be ugly. Hope the compiler does it
			const auto ite = std::find_if(
				inputs.cbegin(), inputs.cend(),
				[source] (const Input& input) {
					return &input.getOutput() == source;
				}
			);

			//The input should be valid
			assert(ite != inputs.cend());

			result = std::distance(inputs.cbegin(), ite);
		}

		return result;
	}


	Overlay& findOverlay(MixEffect::OverlaySlot slot, size_t idx) {
		return overlays.at(static_cast<size_t>(slot)).at(idx);
	}

	const Overlay& findOverlay(MixEffect::OverlaySlot slot, size_t idx) const {
		return overlays.at(static_cast<size_t>(slot)).at(idx);
	}
	

	bool isTransitionConfigured() const noexcept {
		return intermediateLayer.getInput().getSource() != nullptr;
	}

	void configureLayers(bool useTransition) {
		std::vector<RendererBase::LayerRef> layers;
		auto* transition = getSelectedTransition();

		if(transition && useTransition) {
			//A transition is in progress. Configure USK and DSK separately
			for(size_t i = 0; i < OUTPUT_BUS_CNT; ++i) {
				const auto outputBus = static_cast<MixEffect::OutputBus>(i);

				//Obtain the upsetream layers
				layers = { backgroundLayers[i] };

				for(auto& overlay : overlays[static_cast<size_t>(MixEffect::OverlaySlot::upstream)]) {
					if(overlay.isRendered(outputBus)) {
						layers.emplace_back(*overlay.getOverlay());
					}
				}

				intermediateCompositors[i].setLayers(layers);

				//Obtain the downstream layers
				if(outputBus == transitionSlot) {
					//Transition is active on this bus
					const auto transitionLayers = transition->getLayers();
					layers.clear();
					layers.insert(layers.cend(), transitionLayers.cbegin(), transitionLayers.cend());
				} else {
					//Transition is not active on this layer. Use the intermediate layer
					layers = { intermediateLayer };
					intermediateLayer << intermediateCompositors[i];
				}

				for(auto& overlay : overlays[static_cast<size_t>(MixEffect::OverlaySlot::downstream)]) {
					if(overlay.isRendered(outputBus)) {
						layers.emplace_back(*overlay.getOverlay());
					}
				}

				compositors[i].setLayers(layers);
			}
			
		} else {
			//Configure the downstream and upstream composition altogether
			for(size_t i = 0; i < OUTPUT_BUS_CNT; ++i) {
				const auto outputBus = static_cast<MixEffect::OutputBus>(i);

				//Obtain the layers
				layers = { backgroundLayers[i] };
				
				for(auto& overlaySlot : overlays) {
					for(auto& overlay : overlaySlot) {
						if(overlay.isRendered(outputBus)) {
							layers.emplace_back(*overlay.getOverlay());
						}
					}
				}

				//Write changes
				compositors[i].setLayers(layers);
			}

			//Signal that the intermediate compositing is not used
			intermediateLayer.getInput() << Signal::noSignal;
		}
	}

	void finishTransition() {
		auto* transition = getSelectedTransition();

		if(transitionSlot == MixEffect::OutputBus::program) {
			//As the transition is in program, cut between sources.
			//This will rewind and stop it
			cut();
		} else if(transition) {
			//Reset the transition
			setTransitionBar(0.0f);
		}

		assert(transition->isPlaying() == false);
		assert(transition->getTime() == Zuazo::TimePoint());
	}


	void viewportSizeCallback(Math::Vec2f size) {
		for(auto& bkgdLayer : backgroundLayers) {
			bkgdLayer.setSize(size);
		}
		intermediateLayer.setSize(size);

		for(auto& transition : transitions) {
			if(transition.second) {
				transition.second->setSize(size);
			}
		}
	}


	static VideoSurface createBackgroundLayer(	Instance& instance, 
												std::string name,
												const Compositor& renderer ) 
	{
		VideoSurface result(
			instance,
			name,
			renderer.getViewportSize()
		);
		
		//Configure it
		result.setBlendingMode(BlendingMode::write);
		result.setRenderingLayer(RenderingLayer::background);

		return result;
	}

};
	


/*
 * MixEffect
 */

MixEffect::MixEffect(	Zuazo::Instance& instance,
						std::string name )
	: Utils::Pimpl<MixEffectImpl>({}, *this, instance, name)
	, ZuazoBase(
		instance,
		std::move(name),
		{},
		std::bind(&MixEffectImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&MixEffectImpl::open, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&MixEffectImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MixEffectImpl::close, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&MixEffectImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MixEffectImpl::update, std::ref(**this)) )
	, VideoBase(
		std::bind(&MixEffectImpl::setVideoMode, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
	, VideoScalerBase(
		std::bind(&MixEffectImpl::setScalingMode, std::ref(**this), std::placeholders::_2),
		std::bind(&MixEffectImpl::setScalingFilter, std::ref(**this), std::placeholders::_2)
	)
{
	//Register output pads
	registerPad(getOutput(OutputBus::program));
	registerPad(getOutput(OutputBus::preview));

	//Set compatibility to a known state
	setVideoModeCompatibility((*this)->getVideoModeCompatibility());

	//Configure the callbacks. Note this callbacks need to be set here 
	//as they make use of this class, so setting them on the PIMPL
	//instantiation will cause a segfault
	(*this)->referenceCompositor.setVideoModeNegotiationCallback(
		std::bind(&MixEffectImpl::videoModeNegotiationCallback, this->get(), std::placeholders::_2)
	);

	//Add the default transitions
	addTransition(Utils::makeUnique<Transitions::Mix>(instance, "Mix"));
	addTransition(Utils::makeUnique<Transitions::DVE>(instance, "DVE"));
	setSelectedTransition("Mix");
}

MixEffect::MixEffect(MixEffect&& other) = default;

MixEffect::~MixEffect() = default;

MixEffect& MixEffect::operator=(MixEffect&& other) = default;



void MixEffect::setInputCount(size_t count) {
	(*this)->setInputCount(*this, count);
}

size_t MixEffect::getInputCount() const noexcept {
	return (*this)->getInputCount();
}

MixEffect::Input& MixEffect::getInput(size_t idx) {
	return (*this)->getInput(idx);
}

const MixEffect::Input& MixEffect::getInput(size_t idx) const {
	return (*this)->getInput(idx);
}


MixEffect::Output& MixEffect::getOutput(OutputBus bus) {
	return (*this)->getOutput(bus);
}

const MixEffect::Output& MixEffect::getOutput(OutputBus bus) const {
	return (*this)->getOutput(bus);
}



void MixEffect::setBackground(OutputBus bus, size_t idx) {
	(*this)->setBackground(bus, idx);
}

size_t MixEffect::getBackground(OutputBus bus) const noexcept {
	return (*this)->getBackground(bus);
}



void MixEffect::cut() {
	(*this)->cut();
}

void MixEffect::transition() {
	(*this)->transition();
}


void MixEffect::setTransitionBar(float progress) {
	(*this)->setTransitionBar(progress);
}

float MixEffect::getTransitionBar() const noexcept {
	return (*this)->getTransitionBar();
}


void MixEffect::setTransitionSlot(OutputBus slot) {
	(*this)->setTransitionSlot(slot);
}

MixEffect::OutputBus MixEffect::getTransitionSlot() const noexcept {
	return (*this)->getTransitionSlot();
}



void MixEffect::addTransition(std::unique_ptr<Transitions::Base> transition) {
	(*this)->addTransition(std::move(transition));
}

std::unique_ptr<Transitions::Base> MixEffect::removeTransition(std::string_view name) {
	return (*this)->removeTransition(name);
}

std::vector<Transitions::Base*> MixEffect::getTransitions() {
	return (*this)->getTransitions();
}

std::vector<const Transitions::Base*> MixEffect::getTransitions() const {
	return reinterpret_cast<std::vector<const Transitions::Base*>&&>((*this)->getTransitions()); //HACK
}

Transitions::Base* MixEffect::getTransition(std::string_view name) {
	return (*this)->getTransition(name);
}

const Transitions::Base* MixEffect::getTransition(std::string_view name) const {
	return (*this)->getTransition(name);
}

void MixEffect::setSelectedTransition(std::string_view name) {
	(*this)->setSelectedTransition(name);
}	

Transitions::Base* MixEffect::getSelectedTransition() noexcept {
	return (*this)->getSelectedTransition();
}

const Transitions::Base* MixEffect::getSelectedTransition() const noexcept {
	return (*this)->getSelectedTransition();
}



void MixEffect::setOverlayCount(OverlaySlot slot, size_t count) {
	(*this)->setOverlayCount(slot, count);
}

size_t MixEffect::getOverlayCount(OverlaySlot slot) const {
	return (*this)->getOverlayCount(slot);
}

Overlays::Base* MixEffect::getOverlay(OverlaySlot slot, size_t idx) {
	return (*this)->getOverlay(slot, idx);
}

const Overlays::Base* MixEffect::getOverlay(OverlaySlot slot, size_t idx) const {
	return (*this)->getOverlay(slot, idx);
}

void MixEffect::setOverlayVisible(OverlaySlot slot, size_t idx, bool visible) {
	(*this)->setOverlayVisible(slot, idx, visible);
}

bool MixEffect::getOverlayVisible(OverlaySlot slot, size_t idx) const {
	return (*this)->getOverlayVisible(slot, idx);
}

void MixEffect::setOverlayTransition(OverlaySlot slot, size_t idx, bool transition) {
	(*this)->setOverlayTransition(slot, idx, transition);
}

bool MixEffect::getOverlayTransition(OverlaySlot slot, size_t idx) const {
	return (*this)->getOverlayTransition(slot, idx);
}

void MixEffect::setOverlaySignal(OverlaySlot slot, size_t overlay, std::string_view port, size_t idx) {
	(*this)->setOverlaySignal(slot, overlay, port, idx);
}

size_t MixEffect::getOverlaySignal(OverlaySlot slot, size_t overlay, std::string_view port) const noexcept {
	return (*this)->getOverlaySignal(slot, overlay, port);
}

}