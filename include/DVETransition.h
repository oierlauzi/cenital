#pragma once

#include "TransitionBase.h"

#include <zuazo/Utils/Pimpl.h>
#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>

namespace Cenital {

struct DVETransitionImpl;
class DVETransition
	: private Zuazo::Utils::Pimpl<DVETransitionImpl>
	, public TransitionBase
{
	friend DVETransitionImpl;
public:
	enum class Effect {
		UNCOVER,
		COVER,
		SLIDE,
		ROTATE_3D,
		
		//Add here

		COUNT
	};

	DVETransition(	Zuazo::Instance& instance,
					std::string name );

	DVETransition(const DVETransition& other) = delete;
	DVETransition(DVETransition&& other);
	virtual ~DVETransition();

	DVETransition&							operator=(const DVETransition& other) = delete;
	DVETransition&							operator=(DVETransition&& other);

	void									setScalingFilter(Zuazo::ScalingFilter filter); 
	Zuazo::ScalingFilter					getScalingFilter() const noexcept;

	void									setAngle(float angle);
	float									getAngle() const noexcept;

	void									setEffect(Effect effect);
	Effect									getEffect() const noexcept;

};

}