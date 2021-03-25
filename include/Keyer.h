#pragma once

#include "Shapes.h"

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
		KEY_R,
		KEY_G,
		KEY_B,
		KEY_A,
		KEY_Y,
		FILL_R,
		FILL_G,
		FILL_B,
		FILL_A,
		FILL_Y

	};

	Keyer(	Zuazo::Instance& instance,
			std::string name,
			const Zuazo::RendererBase* renderer,
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

	void									setChromaKeyLuminosityThreshold(float threshold);
	float									getChromaKeyLuminosityThreshold() const noexcept;

	void									setChromaKeyLuminositySmoothness(float smoothness);
	float									getChromaKeyLuminositySmoothness() const noexcept;


	//Linear key
	void									setLinearKeyEnabled(bool ena);
	bool									getLinearKeyEnabled() const noexcept;

	void									setLinearKeyInverted(bool inv);
	bool									getLinearKeyInverted() const noexcept;

	void									setLinearKeyChannel(LinearKeyChannel ch);
	LinearKeyChannel						getLinearKeyChannel() const noexcept;					

};

}