#include <MixTransition.h>

#include <zuazo/Signal/DummyPad.h>
#include <zuazo/Math/Trigonometry.h>
#include <zuazo/Layers/VideoSurface.h>

namespace Cenital {

using namespace Zuazo;

struct MixTransitionImpl {
	using Input = Signal::DummyPad<Zuazo::Video>;
	using VideoSurface = Layers::VideoSurface;

	std::reference_wrapper<MixTransition>	owner;

	Input									prevIn;
	Input									postIn;
	
	VideoSurface							prevSurface;
	VideoSurface							postSurface;

	std::array<RendererBase::LayerRef, 2>	layerReferences;

	MixTransition::Effect					effect;
	

	MixTransitionImpl(MixTransition& owner, Instance& instance)
		: owner(owner)
		, prevIn("prevIn")
		, postIn("postIn")
		, prevSurface(instance, "prevSurface", Math::Vec2f())
		, postSurface(instance, "postSurface", Math::Vec2f())
		, layerReferences{ postSurface, prevSurface }
		, effect(MixTransition::Effect::MIX)
	{
		//Route the signals permanently
		prevSurface << prevIn;
		postSurface << postIn;

		//Configure the permanent parameters of the surfaces
		prevSurface.setScalingMode(ScalingMode::STRETCH); //defaults
		postSurface.setScalingMode(ScalingMode::STRETCH); //defaults
		prevSurface.setRenderingLayer(RenderingLayer::BACKGROUND); //Not the default value
		postSurface.setRenderingLayer(RenderingLayer::BACKGROUND); //Not the default value
	}

	~MixTransitionImpl() = default;

	void moved(ZuazoBase& base) {
		owner = static_cast<MixTransition&>(base);
	}

	void open(ZuazoBase&) {
		prevSurface.open();
		postSurface.open();
	}

	void asyncOpen(ZuazoBase&, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		prevSurface.asyncOpen(lock);
		postSurface.asyncOpen(lock);
		assert(lock.owns_lock());
	}


	void close(ZuazoBase&) {
		prevSurface.close();
		postSurface.close();
	}

	void asyncClose(ZuazoBase&, std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		prevSurface.asyncClose(lock);
		postSurface.asyncClose(lock);
		assert(lock.owns_lock());
	}



	void updateCallback() {
		auto& mixTransition = owner.get();
		
		//Obtain the progress
		const auto progress = static_cast<float>(mixTransition.getProgress());

		//Obtain the viewport size
		const auto viewportSize = mixTransition.getSize();
		assert(prevSurface.getSize() == viewportSize);
		assert(postSurface.getSize() == viewportSize);
		Utils::ignore(viewportSize);

		//Configure the layers
		switch (effect) {
		case MixTransition::Effect::MIX:
			configureMix(progress);
			break;

		case MixTransition::Effect::ADD:
			configureAdd(progress);
			break;

		case MixTransition::Effect::BLACK:
			configureBlack(progress);
			break;

		default:
			break;
		}
	}

	void sizeCallback(TransitionBase&, Math::Vec2f size) {
		prevSurface.setSize(size);
		postSurface.setSize(size);
		updateCallback();
	}



	void setScalingFilter(Zuazo::ScalingFilter filter) {
		prevSurface.setScalingFilter(filter);
		postSurface.setScalingFilter(filter);
	}

	Zuazo::ScalingFilter getScalingFilter() const noexcept {
		const auto result = prevSurface.getScalingFilter();
		assert(result == postSurface.getScalingFilter());
		return result;
	}



	void setEffect(MixTransition::Effect effect) {
		this->effect = effect;
		updateCallback();
	}

	MixTransition::Effect getEffect() const noexcept {
		return effect;
	}

private:
	void configureMix(float progress) {
		//Set complementary gains. As they are adding, this
		//results in a mixing effect
		prevSurface.setBlendingMode(BlendingMode::ADD);
		postSurface.setBlendingMode(BlendingMode::ADD);
		prevSurface.setOpacity(1 - progress);
		postSurface.setOpacity(0 + progress);
	}

	void configureAdd(float progress) {
		prevSurface.setBlendingMode(BlendingMode::ADD);
		postSurface.setBlendingMode(BlendingMode::ADD);
		prevSurface.setOpacity(Math::min(2*(1 - progress), 1.0f));
		postSurface.setOpacity(Math::min(2*(0 + progress), 1.0f));
	}

	void configureBlack(float progress) {
		prevSurface.setBlendingMode(BlendingMode::ADD);
		postSurface.setBlendingMode(BlendingMode::ADD);
		prevSurface.setOpacity(Math::max(+1 - 2*progress, 0.0f));
		postSurface.setOpacity(Math::max(-1 + 2*progress, 0.0f));
	}

};


MixTransition::MixTransition(	Instance& instance,
								std::string name )
	: Utils::Pimpl<MixTransitionImpl>({}, *this, instance)
	, TransitionBase(
		instance,
		std::move(name),
		(*this)->prevIn.getInput(),
		(*this)->postIn.getInput(),
		(*this)->layerReferences,
		std::bind(&MixTransitionImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&MixTransitionImpl::open, std::ref(**this), std::placeholders::_1),
		std::bind(&MixTransitionImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MixTransitionImpl::close, std::ref(**this), std::placeholders::_1),
		std::bind(&MixTransitionImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MixTransitionImpl::updateCallback, std::ref(**this)),
		std::bind(&MixTransitionImpl::sizeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
{
	//Leave it in a known state
	(*this)->updateCallback();
}

MixTransition::MixTransition(MixTransition&& other) = default;
MixTransition::~MixTransition() = default;

MixTransition& MixTransition::operator=(MixTransition&& other) = default;



void MixTransition::setEffect(Effect effect) {
	(*this)->setEffect(effect);
}

MixTransition::Effect MixTransition::getEffect() const noexcept {
	return (*this)->getEffect();
}

}