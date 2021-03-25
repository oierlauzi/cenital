#pragma once

#include "TransitionBase.h"

#include <zuazo/Utils/Pimpl.h>
#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>

namespace Cenital {

struct MixTransitionImpl;
class MixTransition
	: private Zuazo::Utils::Pimpl<MixTransitionImpl>
	, public Zuazo::ZuazoBase
	, public TransitionBase
{
	friend MixTransitionImpl;
public:
	enum class Effect {
		MIX,
		ADD,
		BLACK,
		
		//Add here

		COUNT
	};

	MixTransition(	Zuazo::Instance& instance,
					std::string name );

	MixTransition(const MixTransition& other) = delete;
	MixTransition(MixTransition&& other);
	virtual ~MixTransition();

	MixTransition&							operator=(const MixTransition& other) = delete;
	MixTransition&							operator=(MixTransition&& other);

	void									setEffect(Effect effect);
	Effect									getEffect() const noexcept;

};

}