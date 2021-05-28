#include <Transitions/DVE.h>

#include <zuazo/StringConversions.h>

namespace Zuazo {

std::string_view toString(Cenital::Transitions::DVE::Effect effect) noexcept {
	switch(effect){

	ZUAZO_ENUM2STR_CASE( Cenital::Transitions::DVE::Effect, uncover )
	ZUAZO_ENUM2STR_CASE( Cenital::Transitions::DVE::Effect, cover )
	ZUAZO_ENUM2STR_CASE( Cenital::Transitions::DVE::Effect, slide )
	ZUAZO_ENUM2STR_CASE( Cenital::Transitions::DVE::Effect, rotate3D )

	default: return "";
	}
}

bool fromString(std::string_view str, Cenital::Transitions::DVE::Effect& effect) {
	return enumFromString(str, effect);
}

std::ostream& operator<<(std::ostream& os, Cenital::Transitions::DVE::Effect effect) {
	return os << toString(effect);
}

}