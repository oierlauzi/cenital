#include <Transitions/Mix.h>

#include <zuazo/Signal/DummyPad.h>
#include <zuazo/Math/Trigonometry.h>
#include <zuazo/Layers/VideoSurface.h>

namespace Cenital::Transitions {

using namespace Zuazo;

struct MixImpl {
	using Input = Signal::DummyPad<Zuazo::Video>;
	using VideoSurface = Layers::VideoSurface;

	std::reference_wrapper<Mix>	owner;

	Input									prevIn;
	Input									postIn;
	
	VideoSurface							prevSurface;
	VideoSurface							postSurface;

	std::array<RendererBase::LayerRef, 2>	layerReferences;

	Mix::Effect								effect;
	

	MixImpl(Mix& owner, Instance& instance)
		: owner(owner)
		, prevIn(owner, "prevIn")
		, postIn(owner, "postIn")
		, prevSurface(instance, "prevSurface", Math::Vec2f())
		, postSurface(instance, "postSurface", Math::Vec2f())
		, layerReferences{ postSurface, prevSurface }
		, effect(Mix::Effect::mix)
	{
		//Route the signals permanently
		prevSurface << prevIn;
		postSurface << postIn;

		//Configure the permanent parameters of the surfaces
		prevSurface.setScalingMode(ScalingMode::stretch); //defaults
		postSurface.setScalingMode(ScalingMode::stretch); //defaults
		prevSurface.setRenderingLayer(RenderingLayer::background); //Not the default value
		postSurface.setRenderingLayer(RenderingLayer::background); //Not the default value
	}

	~MixImpl() = default;

	void moved(ZuazoBase& base) {
		owner = static_cast<Mix&>(base);
		prevIn.setLayout(base);
		postIn.setLayout(base);
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
		case Mix::Effect::mix:
			configureMix(progress);
			break;

		case Mix::Effect::add:
			configureAdd(progress);
			break;

		case Mix::Effect::fade:
			configureBlack(progress);
			break;

		default:
			break;
		}
	}

	void sizeCallback(Base&, Math::Vec2f size) {
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



	void setEffect(Mix::Effect effect) {
		this->effect = effect;
		updateCallback();
	}

	Mix::Effect getEffect() const noexcept {
		return effect;
	}

private:
	void configureMix(float progress) {
		//Set complementary gains. As they are adding, this
		//results in a mixing effect
		prevSurface.setBlendingMode(BlendingMode::add);
		postSurface.setBlendingMode(BlendingMode::add);
		prevSurface.setOpacity(1 - progress);
		postSurface.setOpacity(0 + progress);
	}

	void configureAdd(float progress) {
		prevSurface.setBlendingMode(BlendingMode::add);
		postSurface.setBlendingMode(BlendingMode::add);
		prevSurface.setOpacity(Math::min(2*(1 - progress), 1.0f));
		postSurface.setOpacity(Math::min(2*(0 + progress), 1.0f));
	}

	void configureBlack(float progress) {
		prevSurface.setBlendingMode(BlendingMode::add);
		postSurface.setBlendingMode(BlendingMode::add);
		prevSurface.setOpacity(Math::max(+1 - 2*progress, 0.0f));
		postSurface.setOpacity(Math::max(-1 + 2*progress, 0.0f));
	}

};


Mix::Mix(	Instance& instance,
								std::string name )
	: Utils::Pimpl<MixImpl>({}, *this, instance)
	, Base(
		instance,
		std::move(name),
		(*this)->prevIn.getInput(),
		(*this)->postIn.getInput(),
		(*this)->layerReferences,
		std::bind(&MixImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&MixImpl::open, std::ref(**this), std::placeholders::_1),
		std::bind(&MixImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MixImpl::close, std::ref(**this), std::placeholders::_1),
		std::bind(&MixImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&MixImpl::updateCallback, std::ref(**this)),
		std::bind(&MixImpl::sizeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
{
	//Leave it in a known state
	(*this)->updateCallback();
}

Mix::Mix(Mix&& other) = default;
Mix::~Mix() = default;

Mix& Mix::operator=(Mix&& other) = default;



void Mix::setEffect(Effect effect) {
	(*this)->setEffect(effect);
}

Mix::Effect Mix::getEffect() const noexcept {
	return (*this)->getEffect();
}

}