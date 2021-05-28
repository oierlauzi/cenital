#pragma once

#include "Shapes.h"
#include "Control/Node.h"

#include <zuazo/Utils/Pimpl.h>
#include <zuazo/Utils/BufferView.h>
#include <zuazo/ZuazoBase.h>
#include <zuazo/LayerBase.h>
#include <zuazo/Video.h>

namespace Cenital {

struct KeyerImpl;
class Keyer
	: private Zuazo::Utils::Pimpl<KeyerImpl>
	, public Zuazo::ZuazoBase
	, public Zuazo::LayerBase
	, public Zuazo::VideoScalerBase
{
	friend KeyerImpl;
public:

	enum class LinearKeyChannel {
		keyR,
		keyG,
		keyB,
		keyA,
		keyY,
		fillR,
		fillG,
		fillB,
		fillA,
		fillY

	};

	Keyer(	Zuazo::Instance& instance,
			std::string name,
			Zuazo::Math::Vec2f size );

	Keyer(const Keyer& other) = delete;
	Keyer(Keyer&& other);
	virtual ~Keyer();

	Keyer&									operator=(const Keyer& other) = delete;
	Keyer&									operator=(Keyer&& other);


	//Cropping
	void									setSize(Zuazo::Math::Vec2f size);
	Zuazo::Math::Vec2f						getSize() const noexcept;

	void									setCrop(Zuazo::Utils::BufferView<const Shape> shapes);
	Zuazo::Utils::BufferView<const Shape>	getCrop() const noexcept;


	//Luma key
	void									setLumaKeyEnabled(bool ena);
	bool									getLumaKeyEnabled() const noexcept;

	void									setLumaKeyInverted(bool inv);
	bool									getLumaKeyInverted() const noexcept;

	void									setLumaKeyMinThreshold(float threshold);
	float									getLumaKeyMinThreshold() const noexcept;

	void									setLumaKeyMaxThreshold(float threshold);
	float									getLumaKeyMaxThreshold() const noexcept;

	//Chroma key
	void									setChromaKeyEnabled(bool ena);
	bool									getChromaKeyEnabled() const noexcept;

	void									setChromaKeyHue(float hue);
	float									getChromaKeyHue() const noexcept;

	void									setChromaKeyHueThreshold(float threshold);
	float									getChromaKeyHueThreshold() const noexcept;

	void									setChromaKeyHueSmoothness(float smoothness);
	float									getChromaKeyHueSmoothness() const noexcept;

	void									setChromaKeySaturationThreshold(float threshold);
	float									getChromaKeySaturationThreshold() const noexcept;

	void									setChromaKeySaturationSmoothness(float smoothness);
	float									getChromaKeySaturationSmoothness() const noexcept;

	void									setChromaKeyValueThreshold(float threshold);
	float									getChromaKeyValueThreshold() const noexcept;

	void									setChromaKeyValueSmoothness(float smoothness);
	float									getChromaKeyValueSmoothness() const noexcept;


	//Linear key
	void									setLinearKeyEnabled(bool ena);
	bool									getLinearKeyEnabled() const noexcept;

	void									setLinearKeyInverted(bool inv);
	bool									getLinearKeyInverted() const noexcept;

	void									setLinearKeyChannel(LinearKeyChannel ch);
	LinearKeyChannel						getLinearKeyChannel() const noexcept;	



	static void								registerCommands(Control::Node& node);				

};

ZUAZO_ENUM_ARITHMETIC_OPERATORS(Cenital::Keyer::LinearKeyChannel)
ZUAZO_ENUM_COMP_OPERATORS(Cenital::Keyer::LinearKeyChannel)
}



namespace Zuazo {

std::string_view toString(Cenital::Keyer::LinearKeyChannel channel) noexcept;
bool fromString(std::string_view str, Cenital::Keyer::LinearKeyChannel& channel);
std::ostream& operator<<(std::ostream& os, Cenital::Keyer::LinearKeyChannel channel);

namespace Utils {

template<typename T>
struct EnumTraits;

template<>
struct EnumTraits<Cenital::Keyer::LinearKeyChannel> {
	static constexpr Cenital::Keyer::LinearKeyChannel first() noexcept { 
		return Cenital::Keyer::LinearKeyChannel::keyR; 
	}
	static constexpr Cenital::Keyer::LinearKeyChannel last() noexcept { 
		return Cenital::Keyer::LinearKeyChannel::fillY; 
	}
};

}

}
