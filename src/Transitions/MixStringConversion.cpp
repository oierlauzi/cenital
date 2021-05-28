#include <Transitions/Mix.h>

#include <zuazo/StringConversions.h>

namespace Zuazo {

std::string_view toString(Cenital::Transitions::Mix::Effect effect) noexcept {
	switch(effect){

	ZUAZO_ENUM2STR_CASE( Cenital::Transitions::Mix::Effect, mix )
	ZUAZO_ENUM2STR_CASE( Cenital::Transitions::Mix::Effect, add )
	ZUAZO_ENUM2STR_CASE( Cenital::Transitions::Mix::Effect, fade )

	default: return "";
	}
}

bool fromString(std::string_view str, Cenital::Transitions::Mix::Effect& effect) {
	return enumFromString(str, effect);
}

std::ostream& operator<<(std::ostream& os, Cenital::Transitions::Mix::Effect effect) {
	return os << toString(effect);
}

}