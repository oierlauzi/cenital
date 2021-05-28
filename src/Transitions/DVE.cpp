#include <Transitions/DVE.h>

#include <zuazo/StringConversions.h>
#include <zuazo/Signal/DummyPad.h>
#include <zuazo/Math/Trigonometry.h>
#include <zuazo/Layers/VideoSurface.h>

namespace Cenital::Transitions {

using namespace Zuazo;

struct DVEImpl {
	using Input = Signal::DummyPad<Zuazo::Video>;
	using VideoSurface = Layers::VideoSurface;

	std::reference_wrapper<DVE>	owner;

	Input									prevIn;
	Input									postIn;
	
	VideoSurface							prevSurface;
	VideoSurface							postSurface;

	std::array<RendererBase::LayerRef, 2>	layerReferences;

	float									angle;
	DVE::Effect								effect;
	

	DVEImpl(DVE& owner, Instance& instance)
		: owner(owner)
		, prevIn(owner, "prevIn")
		, postIn(owner, "postIn")
		, prevSurface(instance, "prevSurface", Math::Vec2f())
		, postSurface(instance, "postSurface", Math::Vec2f())
		, layerReferences{ postSurface, prevSurface }
		, angle(0.0f)
		, effect(DVE::Effect::uncover)
	{
		//Route the signals permanently
		prevSurface << prevIn;
		postSurface << postIn;

		//Configure the permanent parameters of the surfaces
		prevSurface.setOpacity(1.0f); //defaults
		postSurface.setOpacity(1.0f); //defaults
		prevSurface.setScalingMode(ScalingMode::stretch); //defaults
		postSurface.setScalingMode(ScalingMode::stretch); //defaults
		prevSurface.setBlendingMode(BlendingMode::write); //Not the default value
		postSurface.setBlendingMode(BlendingMode::write); //Not the default value
		prevSurface.setRenderingLayer(RenderingLayer::background); //Not the default value
		postSurface.setRenderingLayer(RenderingLayer::background); //Not the default value
	}

	~DVEImpl() = default;

	void moved(ZuazoBase& base) {
		owner = static_cast<DVE&>(base);
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
		auto& dveTransition = owner.get();
		
		//Obtain the progress
		const auto progress = static_cast<float>(dveTransition.getProgress());

		//Obtain the viewport size
		const auto viewportSize = dveTransition.getSize();
		assert(prevSurface.getSize() == viewportSize);
		assert(postSurface.getSize() == viewportSize);

		//Configure the layers
		switch (effect) {
		case DVE::Effect::uncover:
			configureSlide(viewportSize, progress, true, false);
			break;

		case DVE::Effect::cover:
			configureSlide(viewportSize, progress, false, true);
			break;

		case DVE::Effect::slide:
			configureSlide(viewportSize, progress, true, true);
			break;

		case DVE::Effect::rotate3D:
			configureRotate3D(progress);
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


	void setAngle(float angle) {
		this->angle = angle;
		updateCallback();
	}

	float getAngle() const noexcept {
		return angle;
	}


	void setEffect(DVE::Effect effect) {
		this->effect = effect;
		updateCallback();
	}

	DVE::Effect getEffect() const noexcept {
		return effect;
	}

private:
	void configureSlide(Math::Vec2f viewportSize, float progress, bool prevAnim, bool postAnim) {
		//At least one of the layers must be animated
		assert(prevAnim || postAnim);

		//Calculate the length of the viewpot's diagonal
		const auto viewportLen = Math::length(viewportSize);

		//Obtain the axis on which the transition is performed
		//As vulkan's Y axis is inverted, use a "-" in the sin
		const auto angle = Math::deg2rad(this->angle);
		const auto direction = Math::Vec2f(Math::cos(angle), -Math::sin(angle));
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

	void configureRotate3D(float progress) {
		//Incactive transform is set when the frame is not visible
		constexpr Math::Transformf inactiveTransform(
			Math::Vec3f(0),
			Math::Quaternionf(),
			Math::Vec3f(0)
		);

		//Obtain the axis on which the transition is performed. 
		//The quadrant angle is used as it makes it more intuitive
		const auto axisAngle = Math::deg2rad(this->angle);
		const Math::Vec3f axis(Math::sin(axisAngle), Math::cos(axisAngle), 0);		

		//Configure the transformations
		if(progress < 0.5f) {
			//Prev surface is only visible on the first half
			const float rotationAngle = progress*M_PI;
			const Math::Transformf transform(
				Math::Vec3f(0),
				Math::rotateAbout(axis, rotationAngle, Math::normalized),
				Math::Vec3f(1)
			);
			prevSurface.setTransform(transform);
		} else {
			prevSurface.setTransform(inactiveTransform);
		}

		if(progress > 0.5f) {
			//Post surface is only visible on the second half. Rotation angle is inverted
			//so that the result is not flipped
			const float rotationAngle = (1-progress)*M_PI;
			const Math::Transformf transform(
				Math::Vec3f(0),
				Math::rotateAbout(axis, rotationAngle, Math::normalized),
				Math::Vec3f(1)
			);
			postSurface.setTransform(transform);
		} else {
			postSurface.setTransform(inactiveTransform);
		}
	}

};


DVE::DVE(	Instance& instance,
			std::string name )
	: Utils::Pimpl<DVEImpl>({}, *this, instance)
	, Base(
		instance,
		std::move(name),
		(*this)->prevIn.getInput(),
		(*this)->postIn.getInput(),
		(*this)->layerReferences,
		std::bind(&DVEImpl::moved, std::ref(**this), std::placeholders::_1),
		std::bind(&DVEImpl::open, std::ref(**this), std::placeholders::_1),
		std::bind(&DVEImpl::asyncOpen, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&DVEImpl::close, std::ref(**this), std::placeholders::_1),
		std::bind(&DVEImpl::asyncClose, std::ref(**this), std::placeholders::_1, std::placeholders::_2),
		std::bind(&DVEImpl::updateCallback, std::ref(**this)),
		std::bind(&DVEImpl::sizeCallback, std::ref(**this), std::placeholders::_1, std::placeholders::_2) )
{
	//Leave it in a known state
	(*this)->updateCallback();
}

DVE::DVE(DVE&& other) = default;
DVE::~DVE() = default;

DVE& DVE::operator=(DVE&& other) = default;

void DVE::setScalingFilter(Zuazo::ScalingFilter filter) {
	(*this)->setScalingFilter(filter);
}

Zuazo::ScalingFilter DVE::getScalingFilter() const noexcept {
	return (*this)->getScalingFilter();
}


void DVE::setAngle(float angle) {
	(*this)->setAngle(angle);
}

float DVE::getAngle() const noexcept {
	return (*this)->getAngle();
}



void DVE::setEffect(Effect effect) {
	(*this)->setEffect(effect);
}

DVE::Effect DVE::getEffect() const noexcept {
	return (*this)->getEffect();
}

}