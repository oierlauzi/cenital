#include <DVETransition.h>

#include <zuazo/Signal/DummyPad.h>
#include <zuazo/Math/Trigonometry.h>
#include <zuazo/Processors/Layers/VideoSurface.h>

namespace Cenital {

using namespace Zuazo;

struct DVETransitionImpl {
	using Input = Signal::DummyPad<Zuazo::Video>;
	using VideoSurface = Processors::Layers::VideoSurface;

	std::reference_wrapper<DVETransition>	owner;

	Input									prevIn;
	Input									postIn;
	
	VideoSurface							prevSurface;
	VideoSurface							postSurface;

	std::array<RendererBase::LayerRef, 2>	layerReferences;

	float									angle;
	DVETransition::Effect					effect;
	

	DVETransitionImpl(DVETransition& owner, Instance& instance)
		: owner(owner)
		, prevIn("prevIn")
		, postIn("postIn")
		, prevSurface(instance, "prevSurface", nullptr, Math::Vec2f())
		, postSurface(instance, "postSurface", nullptr, Math::Vec2f())
		, layerReferences{ postSurface, prevSurface }
		, angle(0.0f)
		, effect(DVETransition::Effect::UNCOVER)
	{
		//Route the signals permanently
		prevSurface << prevIn;
		postSurface << postIn;

		//Configure the permanent parameters of the surfaces
		prevSurface.setOpacity(1.0f); //defaults
		postSurface.setOpacity(1.0f); //defaults
		prevSurface.setScalingMode(ScalingMode::STRETCH); //defaults
		postSurface.setScalingMode(ScalingMode::STRETCH); //defaults
		prevSurface.setBlendingMode(BlendingMode::WRITE); //Not the default value
		postSurface.setBlendingMode(BlendingMode::WRITE); //Not the default value
		prevSurface.setRenderingLayer(RenderingLayer::BACKGROUND); //Not the default value
		postSurface.setRenderingLayer(RenderingLayer::BACKGROUND); //Not the default value
	}

	~DVETransitionImpl() = default;

	void moved(ZuazoBase& base) {
		owner = static_cast<DVETransition&>(base);
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



	void refreshCallback(ClipBase& base) {
		auto& dveTransition = static_cast<DVETransition&>(base);
		assert(&owner.get() == &dveTransition);
		
		//Obtain the progress
		const auto progress = static_cast<float>(dveTransition.getProgress());

		//Obtain the viewport size
		const auto viewportSize = dveTransition.getSize();
		assert(prevSurface.getSize() == viewportSize);
		assert(postSurface.getSize() == viewportSize);

		//Configure the layers
		switch (effect) {
		case DVETransition::Effect::UNCOVER:
			configureSlide(viewportSize, progress, true, false);
			break;

		case DVETransition::Effect::COVER:
			configureSlide(viewportSize, progress, false, true);
			break;

		case DVETransition::Effect::SLIDE:
			configureSlide(viewportSize, progress, true, true);
			break;

		default:
			break;
		}
	}

	void rendererCallback(TransitionBase&, const RendererBase* renderer) {
		prevSurface.setRenderer(renderer);
		postSurface.setRenderer(renderer);
	}

	void sizeCallback(TransitionBase&, Math::Vec2f size) {
		prevSurface.setSize(size);
		postSurface.setSize(size);
		refreshCallback(owner);
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


	void setAngle(float angle) {
		this->angle = angle;
		refreshCallback(owner);
	}

	float getAngle() const noexcept {
		return angle;
	}


	void setEffect(DVETransition::Effect effect) {
		this->effect = effect;
		refreshCallback(owner);
	}

	DVETransition::Effect getEffect() const noexcept {
		return effect;
	}

private:
	void configureSlide(Math::Vec2f viewportSize, float progress, bool prevAnim, bool postAnim) {
		//At least one of the layers must be animated
		assert(prevAnim || postAnim);

		//Calculate the length of the viewpot's diagonal
		const auto viewportLen = Math::length(viewportSize);

		//Obtain the axis on which the transition is performed
		const auto angle = Math::deg2rad(-this->angle); //"-" because Vulkan's Y axis is inverted
		const auto direction = Math::Vec2f(Math::cos(angle), Math::sin(angle));
		const auto axis = Math::clamp(
			viewportLen*direction,
			-viewportSize,
			+viewportSize
		);

		//Set next layer's postion if necessary
		if(prevAnim) {
			const auto pos = Math::lerp(Math::Vec2f(0), +axis, progress);

			//Obtain the transform
			const Math::Transformf transform(
				Math::Vec3f(pos, 0)
			);
			prevSurface.setTransform(transform);
		}
		
		//Set post layer's postion if necessary
		if(postAnim) {
			const auto pos = Math::lerp(-axis, Math::Vec2f(0), progress);

			//Obtain the transform
			const Math::Transformf transform(
				Math::Vec3f(pos, 0)
			);
			postSurface.setTransform(transform);
		}

		//Reorder if necessary. If both are animated, we dont care about the ordering
		if(prevAnim != postAnim) {
			if(prevAnim) {
				//Prev is animated, so it must be on top
				layerReferences = { postSurface, prevSurface };
			} else {
				//Post is animated, so it must be on top
				layerReferences = { prevSurface, postSurface };
			}
		}
		
	}

};


DVETransition::DVETransition(	Instance& instance,
								std::string name )
	: Utils::Pimpl<DVETransitionImpl>({}, *this, instance)
	, Zuazo::ZuazoBase(
		instance,
		std::move(name),
		{},
		std::bind(&DVETransitionImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&DVETransitionImpl::open, std::ref(**this), std::placeholders::_1),
		std::bind(&DVETransitionImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&DVETransitionImpl::close, std::ref(**this), std::placeholders::_1),
		std::bind(&DVETransitionImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
	, TransitionBase(
		(*this)->prevIn.getInput(),
		(*this)->postIn.getInput(),
		(*this)->layerReferences,
		std::bind(&DVETransitionImpl::refreshCallback, std::ref(**this), std::placeholders::_1),
		std::bind(&DVETransitionImpl::rendererCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&DVETransitionImpl::sizeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
{
	//Register pads
	ZuazoBase::registerPad(getPrevIn());
	ZuazoBase::registerPad(getPostIn());

	//Leave it in a known state
	(*this)->refreshCallback(*this);
}

DVETransition::DVETransition(DVETransition&& other) = default;
DVETransition::~DVETransition() = default;

DVETransition& DVETransition::operator=(DVETransition&& other) = default;

void DVETransition::setScalingFilter(Zuazo::ScalingFilter filter) {
	(*this)->setScalingFilter(filter);
}

Zuazo::ScalingFilter DVETransition::getScalingFilter() const noexcept {
	return (*this)->getScalingFilter();
}


void DVETransition::setAngle(float angle) {
	(*this)->setAngle(angle);
}

float DVETransition::getAngle() const noexcept {
	return (*this)->getAngle();
}



void DVETransition::setEffect(Effect effect) {
	(*this)->setEffect(effect);
}

DVETransition::Effect DVETransition::getEffect() const noexcept {
	return (*this)->getEffect();
}

}