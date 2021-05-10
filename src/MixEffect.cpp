#include <MixEffect.h>

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
		Overlay(std::unique_ptr<Keyer> overlay = nullptr)
			: m_overlay(std::move(overlay))
			, m_state()
		{
		}
		Overlay(Overlay&& other) = default;
		~Overlay() = default;

		Overlay& operator=(Overlay&& other) = default;


		void setOverlay(std::unique_ptr<Keyer> overlay) {
			m_overlay = std::move(overlay);
		}

		Keyer* getOverlay() noexcept {
			return m_overlay.get();
		}

		const Keyer* getOverlay() const noexcept {
			return m_overlay.get();
		}


		void setVisible(bool visibility) noexcept {
			m_state[VISIBLE_BIT] = visibility;
		}

		bool getVisible() const noexcept {
			return m_state[VISIBLE_BIT];
		}

		bool getVisible(MixEffect::OutputBus bus) const {
			switch(bus) {
			case MixEffect::OutputBus::PROGRAM:
				return getVisible();
			case MixEffect::OutputBus::PREVIEW:
				return getVisible() != getTransition(); //xor
			default:
				throw std::runtime_error("Invalid bus");
			}
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

		std::unique_ptr<Keyer>				m_overlay;
		std::bitset<BIT_COUNT>				m_state;

	};

	using Input = Signal::DummyPad<Video>;
	using Output = Signal::DummyPad<Video>;
	using Compositor = Renderers::Compositor;
	using VideoSurface = Layers::VideoSurface;
	
	static constexpr auto UPDATE_PRIORITY = Instance::PLAYER_PRIORITY; //Animation-like
	static constexpr auto OUTPUT_BUS_CNT = static_cast<size_t>(MixEffect::OutputBus::COUNT);
	static constexpr auto OVERLAY_CNT = static_cast<size_t>(MixEffect::OverlaySlot::COUNT);

	std::reference_wrapper<MixEffect>				owner;

	std::vector<Input>								inputs;
	std::array<Output, OUTPUT_BUS_CNT>				outputs;

	std::array<Compositor, OUTPUT_BUS_CNT> 			compositors;
	std::array<Compositor, OUTPUT_BUS_CNT> 			intermediateCompositors;
	Compositor&										referenceCompositor;

	std::array<VideoSurface, OUTPUT_BUS_CNT> 		backgroundLayers;
	VideoSurface									intermediateLayer;

	std::unique_ptr<TransitionBase>					transition;
	MixEffect::OutputBus							transitionSlot;

	std::array<std::vector<Overlay>, OVERLAY_CNT>	overlays;

	MixEffectImpl(	MixEffect& owner, 
					Instance& instance,
					const std::string& name )
		: owner(owner)
		, inputs{}
		, outputs{
			Output("pgmOut"), 
			Output("pvwOut") }
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
		, transition()
		, transitionSlot(MixEffect::OutputBus::PROGRAM)
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

		if(transition) {
			openHelper(*transition, lock);
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

		if(transition) {
			closeHelper(*transition, lock);
		}

	}

	void asyncClose(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		close(base, &lock);
		assert(lock.owns_lock());
	}

	void update() {
		//Act as a player for the transition
		const auto deltaTime = owner.get().getInstance().getDeltaT();
		if(transition) {
			if(transition->isPlaying()) {
				//Advance the time for the transition
				transition->setRepeat(ClipBase::Repeat::NONE); //Ensure that it won't repeat
				transition->advanceNormalSpeed(deltaTime);

				//If transition has ended, cut. This will also stop and rewind the transition
				if(transition->getTime().time_since_epoch() == transition->getDuration()) {
					cut();
					assert(transition->isPlaying() == false);
					assert(transition->getTime() == Zuazo::TimePoint());
				}

			}
		}

	}


	void videoModeCallback(VideoBase& base, const VideoMode& videoMode) {
		auto& me = static_cast<MixEffect&>(base);
		assert(&owner.get() == &me);

		//Intermediate compositors will use a variant of the original videoMode
		VideoMode intermediateVideoMode = videoMode;
		intermediateVideoMode.setColorModel(Utils::MustBe<ColorModel>(ColorModel::RGB));
		intermediateVideoMode.setColorTransferFunction(Utils::MustBe<ColorTransferFunction>(ColorTransferFunction::LINEAR));
		intermediateVideoMode.setColorSubsampling(Utils::MustBe<ColorSubsampling>(ColorSubsampling::RB_444));
		intermediateVideoMode.setColorRange(Utils::MustBe<ColorRange>(ColorRange::FULL));
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



	void setInputCount(size_t count) {
		if(inputs.size() != count) {
			auto& mixEffect = owner.get();

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
					inputs.emplace_back(Signal::makeInputName<Video>(inputs.size()));
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
		auto& bkgdInput = backgroundLayers.at(static_cast<size_t>(bus)).getInput();

		if(idx < inputs.size()) {
			bkgdInput << inputs[idx];
		} else {
			bkgdInput << Signal::noSignal;
		}
	}

	size_t getBackground(MixEffect::OutputBus bus) const noexcept {
		size_t result = MixEffect::NO_SIGNAL;
		auto& bkgdInput = backgroundLayers.at(static_cast<size_t>(bus)).getInput();

		const auto source = bkgdInput.getSource();
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

		//Stop the transition just in case
		if(transition) {
			transition->stop();
		}

		//Ensure that the layers are configured without a transition in place
		if(isTransitionEnabled()) {
			configureLayers(false);
		}
	}

	void playTransition() {
		if(transition) {
			//Start playing
			transition->play();

			//Configure the layers if necessary
			if(!isTransitionEnabled()) {
				configureLayers(true);
			}
		} else {
			//No transition, simply cut
			cut();
		}
	}



	void setTransitionProgress(float progress) {
		if(progress >= 1.0f) {
			//If we got to the end, cut. This will reset the transition
			cut();
		} else if (progress >= 0.0f && transition) {
			//Pause if a autotransition was taking place
			transition->pause();

			//Advance the transition upto the point
			transition->setProgress(progress);
			
			//Configure the layers if necessary
			if(!isTransitionEnabled()) {
				configureLayers(true);
			}
		} 
	}

	float getTransitionProgress() const noexcept {
		return transition ? transition->getProgress() : 0;
	}



	void setTransitionSlot(MixEffect::OutputBus bus) {
		if(transitionSlot != bus) {
			transitionSlot = bus;

			//Reconfigure the layers if necessary
			if(isTransitionEnabled()) {
				configureLayers(true);
			}
		}
	}

	MixEffect::OutputBus getTransitionSlot() const noexcept {
		return transitionSlot;
	}

	std::unique_ptr<TransitionBase> setTransition(std::unique_ptr<TransitionBase> next) {
		auto& mixEffect = owner.get();

		//Configure the transition to match the setup
		if(next) {
			//Set same "openess"
			if(mixEffect.isOpen() != next->isOpen()) {
				if(mixEffect.isOpen()) {
					next->open();
				} else {
					next->close();
				}
			}

			//Tie its size to the renderer's viewport size
			next->setSize(referenceCompositor.getViewportSize());

			//Route the intermediate compositions to the transition
			next->getPrevIn() << intermediateCompositors[static_cast<size_t>(MixEffect::OutputBus::PROGRAM)];
			next->getPostIn() << intermediateCompositors[static_cast<size_t>(MixEffect::OutputBus::PREVIEW)];
		}

		//Write the changes and return the old transition
		std::swap(next, transition);
		
		//Reset the old one
		if(next) {
			next->getPrevIn() << Signal::noSignal;
			next->getPostIn() << Signal::noSignal;
		}

		return next;
	}

	const TransitionBase* getTransition() const noexcept {
		return transition.get();
	}



	void setOverlayCount(MixEffect::OverlaySlot slot, size_t count) {
		constexpr std::array<const char*, OVERLAY_CNT> SLOT_NAMES = {
			"USK",
			"DSK"
		};

		const auto& me = owner.get();
		auto& selection = overlays.at(static_cast<size_t>(slot));
		const auto selectionName = SLOT_NAMES.at(static_cast<size_t>(slot));

		//Take the appropiate action to resize. Note that only one of the following
		//statements will have effect (neither of them if count == size)

		//Add elements if necessary
		selection.reserve(count);
		while(selection.size() < count) {
			selection.emplace_back(
				Utils::makeUnique<Keyer>(
					me.getInstance(),
					me.getName() + " - " + selectionName + " " + toString(selection.size()),
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

	Keyer& getOverlay(MixEffect::OverlaySlot slot, size_t idx) {
		return *findOverlay(slot, idx).getOverlay();
	}

	const Keyer& getOverlay(MixEffect::OverlaySlot slot, size_t idx) const {
		return *findOverlay(slot, idx).getOverlay();
	}

	void setOverlayVisible(MixEffect::OverlaySlot slot, size_t idx, bool visible) {
		findOverlay(slot, idx).setVisible(visible);
		configureLayers(isTransitionEnabled());
	}

	bool getOverlayVisible(MixEffect::OverlaySlot slot, size_t idx) const {
		return findOverlay(slot, idx).getVisible();
	}

	void setOverlayTransition(MixEffect::OverlaySlot slot, size_t idx, bool transition) {
		findOverlay(slot, idx).setTransition(transition);
		configureLayers(isTransitionEnabled());
	}

	bool getOverlayTransition(MixEffect::OverlaySlot slot, size_t idx) const {
		return findOverlay(slot, idx).getTransition();
	}

private:
	Overlay& findOverlay(MixEffect::OverlaySlot slot, size_t idx) {
		return overlays.at(static_cast<size_t>(slot)).at(idx);
	}

	const Overlay& findOverlay(MixEffect::OverlaySlot slot, size_t idx) const {
		return overlays.at(static_cast<size_t>(slot)).at(idx);
	}

	void configureLayers(bool useTransition) {
		std::vector<RendererBase::LayerRef> layers;

		if(transition && useTransition) {
			//A transition is in progress. Configure USK and DSK separately
			for(size_t i = 0; i < OUTPUT_BUS_CNT; ++i) {
				const auto outputBus = static_cast<MixEffect::OutputBus>(i);

				//Obtain the upsetream layers
				layers = { backgroundLayers[i] };

				for(auto& overlay : overlays[static_cast<size_t>(MixEffect::OverlaySlot::UPSTREAM)]) {
					if(overlay.getVisible(outputBus)) {
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

				for(auto& overlay : overlays[static_cast<size_t>(MixEffect::OverlaySlot::DOWNSTREAM)]) {
					if(overlay.getVisible(outputBus)) {
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
						if(overlay.getVisible(outputBus)) {
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

	void viewportSizeCallback(Math::Vec2f size) {
		for(auto& bkgdLayer : backgroundLayers) {
			bkgdLayer.setSize(size);
		}
		intermediateLayer.setSize(size);

		if(transition) {
			transition->setSize(size);
		}
	}

	bool isTransitionEnabled() const noexcept {
		return intermediateLayer.getInput().getSource() != nullptr;
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
		result.setBlendingMode(BlendingMode::WRITE);
		result.setRenderingLayer(RenderingLayer::BACKGROUND);

		return result;
	}

};
	


/*
 * MixEffect
 */

MixEffect::MixEffect(	Zuazo::Instance& instance,
						std::string name )
	: Zuazo::Utils::Pimpl<MixEffectImpl>({}, *this, instance, name)
	, Zuazo::ZuazoBase(
		instance,
		std::move(name),
		{},
		std::bind(&MixEffectImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&MixEffectImpl::open, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&MixEffectImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MixEffectImpl::close, std::ref(**this), std::placeholders::_1, nullptr),
		std::bind(&MixEffectImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MixEffectImpl::update, std::ref(**this)) )
	, Zuazo::VideoBase(
		std::bind(&MixEffectImpl::videoModeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
{
	//Register output pads
	registerPad(getProgramOutput());
	registerPad(getPreviewOutput());

	//Set compatibility to a known state
	setVideoModeCompatibility((*this)->getVideoModeCompatibility());

	//Configure the callbacks. Note this callbacks need to be set here 
	//as they make use of this class, so setting them on the PIMPL
	//instantiation will cause a segfault
	(*this)->referenceCompositor.setVideoModeNegotiationCallback(
		std::bind(&MixEffectImpl::videoModeNegotiationCallback, this->get(), std::placeholders::_2)
	);
}

MixEffect::MixEffect(MixEffect&& other) = default;

MixEffect::~MixEffect() = default;

MixEffect& MixEffect::operator=(MixEffect&& other) = default;



void MixEffect::setInputCount(size_t count) {
	(*this)->setInputCount(count);
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

MixEffect::Output& MixEffect::getProgramOutput() noexcept {
	return getOutput(OutputBus::PROGRAM);
}

const MixEffect::Output& MixEffect::getProgramOutput() const noexcept {
	return getOutput(OutputBus::PROGRAM);
}

MixEffect::Output& MixEffect::getPreviewOutput() noexcept {
	return getOutput(OutputBus::PREVIEW);
}

const MixEffect::Output& MixEffect::getPreviewOutput() const noexcept {
	return getOutput(OutputBus::PREVIEW);
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


void MixEffect::setTransitionSlot(OutputBus bus) {
	(*this)->setTransitionSlot(bus);
}

MixEffect::OutputBus MixEffect::getTransitionSlot() const noexcept {
	return (*this)->getTransitionSlot();
}

std::unique_ptr<TransitionBase> MixEffect::setTransition(std::unique_ptr<TransitionBase> transition) {
	return (*this)->setTransition(std::move(transition));
}

const TransitionBase* MixEffect::getTransition() const noexcept {
	return (*this)->getTransition();
}


void MixEffect::setOverlayCount(OverlaySlot slot, size_t count) {
	(*this)->setOverlayCount(slot, count);
}

size_t MixEffect::getOverlayCount(OverlaySlot slot) const {
	return (*this)->getOverlayCount(slot);
}

Keyer& MixEffect::getOverlay(OverlaySlot slot, size_t idx) {
	return (*this)->getOverlay(slot, idx);
}

const Keyer& MixEffect::getOverlay(OverlaySlot slot, size_t idx) const {
	return (*this)->getOverlay(slot, idx);
}

void MixEffect::setOverlayVisible(OverlaySlot slot, size_t idx, bool visible) {
	return (*this)->setOverlayVisible(slot, idx, visible);
}

bool MixEffect::getOverlayVisible(OverlaySlot slot, size_t idx) const {
	return (*this)->getOverlayVisible(slot, idx);
}

void MixEffect::setOverlayTransition(OverlaySlot slot, size_t idx, bool transition) {
	return (*this)->setOverlayTransition(slot, idx, transition);
}

bool MixEffect::getOverlayTransition(OverlaySlot slot, size_t idx) const {
	return (*this)->getOverlayTransition(slot, idx);
}

}