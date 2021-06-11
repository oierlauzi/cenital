#pragma once

#include "Base.h"
#include "../Control/Controller.h"

#include <zuazo/Utils/Pimpl.h>
#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>

namespace Cenital::Transitions {

struct DVEImpl;
class DVE
	: private Zuazo::Utils::Pimpl<DVEImpl>
	, public Base
{
	friend DVEImpl;
public:
	enum class Effect : int {
		none = -1,

		uncover,
		cover,
		slide,
		rotate3D,
		
		//Add here

		count
	};

	DVE(Zuazo::Instance& instance,
		std::string name );

	DVE(const DVE& other) = delete;
	DVE(DVE&& other);
	virtual ~DVE();

	DVE&							operator=(const DVE& other) = delete;
	DVE&							operator=(DVE&& other);

	void							setScalingFilter(Zuazo::ScalingFilter filter); 
	Zuazo::ScalingFilter			getScalingFilter() const noexcept;

	void							setAngle(float angle);
	float							getAngle() const noexcept;

	void							setEffect(Effect effect);
	Effect							getEffect() const noexcept;



	static void						registerCommands(Control::Controller& controller);	

};

ZUAZO_ENUM_ARITHMETIC_OPERATORS(DVE::Effect)
ZUAZO_ENUM_COMP_OPERATORS(DVE::Effect)	

}



namespace Zuazo {

std::string_view toString(Cenital::Transitions::DVE::Effect effect) noexcept;
bool fromString(std::string_view str, Cenital::Transitions::DVE::Effect& effect);
std::ostream& operator<<(std::ostream& os, Cenital::Transitions::DVE::Effect effect);

namespace Utils {

template<typename T>
struct EnumTraits;

template<>
struct EnumTraits<Cenital::Transitions::DVE::Effect> {
	static constexpr Cenital::Transitions::DVE::Effect first() noexcept { 
		return Cenital::Transitions::DVE::Effect::none + static_cast<Cenital::Transitions::DVE::Effect>(1); 
	}
	static constexpr Cenital::Transitions::DVE::Effect last() noexcept { 
		return Cenital::Transitions::DVE::Effect::count - static_cast<Cenital::Transitions::DVE::Effect>(1);
	}
};

}

}
