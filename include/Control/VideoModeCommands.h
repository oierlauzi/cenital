#pragma once

#include <zuazo/Macros.h>
#include <zuazo/Video.h>

namespace Cenital::Control {

enum class VideoModeAttributes : int {
	none 					= 0,

	frameRate 				= Zuazo::Utils::bit(0),
	resolution 				= Zuazo::Utils::bit(1),
	pixelAspectRatio 		= Zuazo::Utils::bit(2),
	colorPrimaries 			= Zuazo::Utils::bit(3),
	colorModel 				= Zuazo::Utils::bit(4),
	colorTransferFunction	= Zuazo::Utils::bit(5),
	colorSubsampling 		= Zuazo::Utils::bit(6),
	colorRange 				= Zuazo::Utils::bit(7),
	colorFormat 			= Zuazo::Utils::bit(8),

};

ZUAZO_ENUM_BIT_OPERATORS(VideoModeAttributes)

constexpr bool test(VideoModeAttributes bitfield, VideoModeAttributes bits);


void setNegotiator(Zuazo::VideoBase& element, Zuazo::DefaultVideoModeNegotiator negotiator);

const Zuazo::DefaultVideoModeNegotiator& getNegotiator(const Zuazo::VideoBase& element);

template<typename T>
void registerVideoModeCommands(	Node& node,
								VideoModeAttributes wr,
								VideoModeAttributes rd,
								const std::string& parentPath = "video-mode" );

}

#include "VideoModeCommands.inl"