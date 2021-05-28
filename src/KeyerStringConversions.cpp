#include <Keyer.h>

#include <zuazo/StringConversions.h>

namespace Zuazo {

std::string_view toString(Cenital::Keyer::LinearKeyChannel channel) noexcept {
	switch(channel){

	ZUAZO_ENUM2STR_CASE( Cenital::Keyer::LinearKeyChannel, keyR )
	ZUAZO_ENUM2STR_CASE( Cenital::Keyer::LinearKeyChannel, keyG )
	ZUAZO_ENUM2STR_CASE( Cenital::Keyer::LinearKeyChannel, keyB )
	ZUAZO_ENUM2STR_CASE( Cenital::Keyer::LinearKeyChannel, keyA )
	ZUAZO_ENUM2STR_CASE( Cenital::Keyer::LinearKeyChannel, keyY )
	ZUAZO_ENUM2STR_CASE( Cenital::Keyer::LinearKeyChannel, fillR )
	ZUAZO_ENUM2STR_CASE( Cenital::Keyer::LinearKeyChannel, fillG )
	ZUAZO_ENUM2STR_CASE( Cenital::Keyer::LinearKeyChannel, fillB )
	ZUAZO_ENUM2STR_CASE( Cenital::Keyer::LinearKeyChannel, fillA )
	ZUAZO_ENUM2STR_CASE( Cenital::Keyer::LinearKeyChannel, fillY )

	default: return "";
	}
}

bool fromString(std::string_view str, Cenital::Keyer::LinearKeyChannel& channel) {
	return enumFromString(str, channel);
}

std::ostream& operator<<(std::ostream& os, Cenital::Keyer::LinearKeyChannel channel) {
	return os << toString(channel);
}

}