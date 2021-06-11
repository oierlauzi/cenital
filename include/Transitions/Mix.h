#pragma once

#include "Base.h"
#include "../Control/Controller.h"

#include <zuazo/Utils/Pimpl.h>
#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>

namespace Cenital::Transitions {

struct MixImpl;
class Mix
	: private Zuazo::Utils::Pimpl<MixImpl>
	, public Base
{
	friend MixImpl;
public:
	enum class Effect : int {
		none = -1,

		mix,
		add,
		fade,
		
		//Add here

		count
	};

	Mix(Zuazo::Instance& instance,
		std::string name );

	Mix(const Mix& other) = delete;
	Mix(Mix&& other);
	virtual ~Mix();

	Mix&				operator=(const Mix& other) = delete;
	Mix&				operator=(Mix&& other);

	void				setEffect(Effect effect);
	Effect				getEffect() const noexcept;



	static void			registerCommands(Control::Controller& controller);	

};

ZUAZO_ENUM_ARITHMETIC_OPERATORS(Mix::Effect)
ZUAZO_ENUM_COMP_OPERATORS(Mix::Effect)

}



namespace Zuazo {

std::string_view toString(Cenital::Transitions::Mix::Effect effect) noexcept;
bool fromString(std::string_view str, Cenital::Transitions::Mix::Effect& effect);
std::ostream& operator<<(std::ostream& os, Cenital::Transitions::Mix::Effect effect);

namespace Utils {

template<typename T>
struct EnumTraits;

template<>
struct EnumTraits<Cenital::Transitions::Mix::Effect> {
	static constexpr Cenital::Transitions::Mix::Effect first() noexcept { 
		return Cenital::Transitions::Mix::Effect::none + static_cast<Cenital::Transitions::Mix::Effect>(1); 
	}
	static constexpr Cenital::Transitions::Mix::Effect last() noexcept { 
		return Cenital::Transitions::Mix::Effect::count - static_cast<Cenital::Transitions::Mix::Effect>(1);
	}
};

}

}
