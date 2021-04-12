#include <MixEffect.h>

#include <zuazo/Player.h>
#include <zuazo/Signal/DummyPad.h>
#include <zuazo/Processors/Compositor.h>
#include <zuazo/Processors/Layers/VideoSurface.h>

#include <vector>
#include <utility>

namespace Cenital {

using namespace Zuazo;

/*
 * MixEffectImpl
 */

struct MixEffectImpl {
	using Input = Signal::DummyPad<Video>;
	using Output = Signal::DummyPad<Video>;

	static constexpr auto UPDATE_PRIORITY = Instance::PLAYER_PRIORITY; //Animation-like


	std::reference_wrapper<MixEffect>	owner;

	std::vector<Input>					inputs;
	Output								pgmOut;
	Output								pvwOut;

	std::unique_ptr<TransitionBase>		transition;

	Processors::Compositor				pgmCompositor;
	Processors::Compositor				pvwCompositor;
	Processors::Compositor				pgmIntermediateCompositor;
	Processors::Compositor				pvwIntermediateCompositor;
	Processors::Compositor&				referenceCompositor;
	Processors::Layers::VideoSurface	pgmLayer;
	Processors::Layers::VideoSurface	pvwLayer;
	Processors::Layers::VideoSurface	intermediaryLayer;


	MixEffectImpl(	MixEffect& owner, 
					Instance& instance,
					const std::string& name )
		: owner(owner)
		, inputs{}
		, pgmOut("pgmOut")
		, pvwOut("pvwOut")
		, transition()
		, pgmCompositor(instance, name + " - Program Compositor")
		, pvwCompositor(instance, name + " - Preview Compositor")
		, pgmIntermediateCompositor(instance, name + " - Program Intermediate Compositor")
		, pvwIntermediateCompositor(instance, name + " - Preview Intermediate Compositor")
		, referenceCompositor(pgmCompositor) //Arbitrarily choosen
		, pgmLayer(createBackgroundLayer(instance, name + " - Program Layer", referenceCompositor))
		, pvwLayer(createBackgroundLayer(instance, name + " - Preview Layer", referenceCompositor))
		, intermediaryLayer(createBackgroundLayer(instance, name + " - Intermediary Layer", referenceCompositor))
	{
		//Route the signals
		pgmOut << pgmCompositor;
		pvwOut << pvwCompositor;

		//Configure the callbacks
		referenceCompositor.setViewportSizeCallback(
			std::bind(&MixEffectImpl::viewportSizeCallback, this, std::placeholders::_2)
		);
		referenceCompositor.setVideoModeNegotiationCallback(
			std::bind(&MixEffectImpl::videoModeNegotiationCallback, this, std::placeholders::_2)
		);
	}

	~MixEffectImpl() = default;


	void moved(ZuazoBase& base) {
		owner = static_cast<MixEffect&>(base);
	}

	void open(ZuazoBase& base) {
		auto& me = static_cast<MixEffect&>(base);
		assert(&owner.get() == &me); Utils::ignore(me);

		pgmCompositor.open();
		pvwCompositor.open();
		pgmIntermediateCompositor.open();
		pvwIntermediateCompositor.open();
		pgmLayer.open();
		pvwLayer.open();

		if(transition) {
			transition->open();
		}

		me.enableRegularUpdate(UPDATE_PRIORITY);
	}

	void asyncOpen(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		auto& me = static_cast<MixEffect&>(base);
		assert(&owner.get() == &me); Utils::ignore(me);
		assert(lock.owns_lock());
		
		pgmCompositor.asyncOpen(lock);
		pvwCompositor.asyncOpen(lock);
		pgmIntermediateCompositor.asyncOpen(lock);
		pvwIntermediateCompositor.asyncOpen(lock);
		pgmLayer.asyncOpen(lock);
		pvwLayer.asyncOpen(lock);

		if(transition) {
			transition->asyncOpen(lock);
		}

		me.enableRegularUpdate(UPDATE_PRIORITY);

		assert(lock.owns_lock());
	}

	void close(ZuazoBase& base) {
		auto& me = static_cast<MixEffect&>(base);
		assert(&owner.get() == &me); Utils::ignore(me);
		
		me.disableRegularUpdate();

		//Close everyting
		pgmCompositor.close();
		pvwCompositor.close();
		pgmIntermediateCompositor.close();
		pvwIntermediateCompositor.close();
		pgmLayer.close();
		pvwLayer.close();

		if(transition) {
			transition->close();
		}

	}

	void asyncClose(ZuazoBase& base, std::unique_lock<Instance>& lock) {
		auto& me = static_cast<MixEffect&>(base);
		assert(&owner.get() == &me); Utils::ignore(me);
		assert(lock.owns_lock());

		me.disableRegularUpdate();

		//Close everyting
		pgmCompositor.asyncClose(lock);
		pvwCompositor.asyncClose(lock);
		pgmIntermediateCompositor.asyncClose(lock);
		pvwIntermediateCompositor.asyncClose(lock);
		pgmLayer.asyncClose(lock);
		pvwLayer.asyncClose(lock);

		if(transition) {
			transition->asyncClose(lock);
		}

		assert(lock.owns_lock());
	}

	void update() {
		//Act as a player for both transitions
		const auto deltaTime = owner.get().getInstance().getDeltaT();
		if(transition) {
			transition->advance(deltaTime);
		}

	}


	void videoModeCallback(VideoBase& base, const VideoMode& videoMode) {
		auto& me = static_cast<MixEffect&>(base);
		assert(&owner.get() == &me);

		pgmCompositor.setVideoMode(videoMode);
		pvwCompositor.setVideoMode(videoMode);
		pgmIntermediateCompositor.setVideoMode(videoMode);
		pvwIntermediateCompositor.setVideoMode(videoMode);
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
			if(inputs.size() < count) {
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
		if(idx >= inputs.size()) {
			throw std::runtime_error("Invalid input index");
		}

		return inputs[idx].getInput();
	}

	const MixEffect::Input& getInput(size_t idx) const {
		if(idx >= inputs.size()) {
			throw std::runtime_error("Invalid input index");
		}

		return inputs[idx].getInput();
	}


	MixEffect::Output& getProgramOutput() noexcept {
		return pgmOut.getOutput();
	}

	const MixEffect::Output& getProgramOutput() const noexcept {
		return pgmOut.getOutput();
	}

	MixEffect::Output& getPreviewOutput() noexcept {
		return pvwOut.getOutput();
	}

	const MixEffect::Output& getPreviewOutput() const noexcept {
		return pvwOut.getOutput();
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
			next->getPrevIn() << pgmIntermediateCompositor;
			next->getPostIn() << pvwIntermediateCompositor;
		}

		//Write the changes and return the old transition
		std::swap(next, transition);
		
		//Reset the old one
		next->getPrevIn() << Signal::noSignal;
		next->getPostIn() << Signal::noSignal;

		return next;
	}

	const TransitionBase* getTransition() const noexcept {
		return transition.get();
	}


	void setProgram(size_t idx) {
		setInputSignal(pgmLayer.getInput(), idx);
	}

	size_t getProgram() const noexcept {
		return getInputSignal(pgmLayer.getInput());
	}


	void setPreview(size_t idx) {
		setInputSignal(pvwLayer.getInput(), idx);
	}

	size_t getPreview() const noexcept {
		return getInputSignal(pvwLayer.getInput());
	}


	void cut() {
		//Get the current sources
		auto src0 = pgmLayer.getInput().getSource();
		auto src1 = pvwLayer.getInput().getSource();

		//Write the sources swapping them
		pgmLayer.getInput().setSource(src1);
		pvwLayer.getInput().setSource(src0);
	}

private:
	void configureLayers() {
		if(transition) {
			//Configure the upstream composition
			pgmIntermediateCompositor.setLayers({pgmLayer});
			pvwIntermediateCompositor.setLayers({pvwLayer});

			//Configure downstream composition
			pgmCompositor.setLayers(transition->getLayers());
			pvwCompositor.setLayers({intermediaryLayer});
			intermediaryLayer << pvwIntermediateCompositor;
			
		} else {
			//Configure the downstream and upstream composition altogether
			pgmCompositor.setLayers({pgmLayer});
			pvwCompositor.setLayers({pvwLayer});
		}
	}

	void setInputSignal(Signal::Layout::PadProxy<Signal::Input<Video>>& input, 
						size_t idx ) 
	{
		if(idx < inputs.size()) {
			input << inputs[idx];
		} else {
			input << Signal::noSignal;
		}
	}

	size_t getInputSignal(const Signal::Layout::PadProxy<Signal::Input<Video>>& input) const {
		size_t result = MixEffect::NO_SIGNAL;

		const auto source = input.getSource();
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


	void viewportSizeCallback(Math::Vec2f size) {
		pgmLayer.setSize(size);
		pvwLayer.setSize(size);
		intermediaryLayer.setSize(size);

		if(transition) {
			transition->setSize(size);
		}
	}



	static Processors::Layers::VideoSurface createBackgroundLayer(	Instance& instance, 
																	std::string name,
																	const Processors::Compositor& renderer ) 
	{
		Processors::Layers::VideoSurface result(
			instance,
			name,
			&renderer,
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
		std::bind(&MixEffectImpl::open, std::ref(**this), std::placeholders::_1),
		std::bind(&MixEffectImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MixEffectImpl::close, std::ref(**this), std::placeholders::_1),
		std::bind(&MixEffectImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		{} )
	, Zuazo::VideoBase(
		std::bind(&MixEffectImpl::videoModeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
{
	//Register output pads
	registerPad(getProgramOutput());
	registerPad(getPreviewOutput());

	//Set compatibility to a known state
	setVideoModeCompatibility((*this)->getVideoModeCompatibility());
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



MixEffect::Output& MixEffect::getProgramOutput() noexcept {
	return (*this)->getProgramOutput();
}

const MixEffect::Output& MixEffect::getProgramOutput() const noexcept {
	return (*this)->getProgramOutput();
}

MixEffect::Output& MixEffect::getPreviewOutput() noexcept {
	return (*this)->getPreviewOutput();
}

const MixEffect::Output& MixEffect::getPreviewOutput() const noexcept {
	return (*this)->getPreviewOutput();
}


std::unique_ptr<TransitionBase> MixEffect::setTransition(std::unique_ptr<TransitionBase> transition) {
	return (*this)->setTransition(std::move(transition));
}

const TransitionBase* MixEffect::getTransition() const noexcept {
	return (*this)->getTransition();
}


void MixEffect::setUpstreamOverlayCount(size_t count) {
	(void)count;//TODO
}

size_t MixEffect::getUpstreamOverlayCount() const noexcept {
	return 0; //TODO
}


void MixEffect::setDownstreamOverlayCount(size_t count) {
	(void)count;//TODO
}

size_t MixEffect::getDownstreamOverlayCount() const noexcept {
	return 0; //TODO
}


void MixEffect::setProgram(size_t idx) {
	(*this)->setProgram(idx);
}

size_t MixEffect::getProgram() const noexcept {
	return (*this)->getProgram();
}


void MixEffect::setPreview(size_t idx) {
	(*this)->setPreview(idx);
}

size_t MixEffect::getPreview() const noexcept {
	return	(*this)->getPreview();
}


void MixEffect::cut() {
	(*this)->cut();
}

}