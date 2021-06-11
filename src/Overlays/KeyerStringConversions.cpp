#include <Overlays/Keyer.h>

#include <zuazo/StringConversions.h>

namespace Zuazo {

std::string_view toString(Cenital::Overlays::Keyer::LinearKeyChannel channel) noexcept {
	switch(channel){

	ZUAZO_ENUM2STR_CASE( Cenital::Overlays::Keyer::LinearKeyChannel, keyR )
	ZUAZO_ENUM2STR_CASE( Cenital::Overlays::Keyer::LinearKeyChannel, keyG )
	ZUAZO_ENUM2STR_CASE( Cenital::Overlays::Keyer::LinearKeyChannel, keyB )
	ZUAZO_ENUM2STR_CASE( Cenital::Overlays::Keyer::LinearKeyChannel, keyA )
	ZUAZO_ENUM2STR_CASE( Cenital::Overlays::Keyer::LinearKeyChannel, keyY )
	ZUAZO_ENUM2STR_CASE( Cenital::Overlays::Keyer::LinearKeyChannel, fillR )
	ZUAZO_ENUM2STR_CASE( Cenital::Overlays::Keyer::LinearKeyChannel, fillG )
	ZUAZO_ENUM2STR_CASE( Cenital::Overlays::Keyer::LinearKeyChannel, fillB )
	ZUAZO_ENUM2STR_CASE( Cenital::Overlays::Keyer::LinearKeyChannel, fillA )
	ZUAZO_ENUM2STR_CASE( Cenital::Overlays::Keyer::LinearKeyChannel, fillY )

	default: return "";
	}
}

bool fromString(std::string_view str, Cenital::Overlays::Keyer::LinearKeyChannel& channel) {
	return enumFromString(str, channel);
}

std::ostream& operator<<(std::ostream& os, Cenital::Overlays::Keyer::LinearKeyChannel channel) {
	return os << toString(channel);
}

}