#pragma once

#include <zuazo/Macros.h>
#include <zuazo/Video.h>

namespace Cenital::Control {

enum class VideoScalingAttributes : int {
	none 				= 0,

	mode 				= Zuazo::Utils::bit(0),
	filter 				= Zuazo::Utils::bit(1),

	all					= 0b11
};

ZUAZO_ENUM_BIT_OPERATORS(VideoScalingAttributes)

constexpr bool test(VideoScalingAttributes bitfield, VideoScalingAttributes bits);


template<typename T>
void registerVideoScalingModeAttribute(	Node& node,
										bool wr,
										bool rd,
										const std::string& parentPath = "video-scaling" );

template<typename T>
void registerVideoScalingFilterAttribute(	Node& node,
											bool wr,
											bool rd,
											const std::string& parentPath = "video-scaling" );

template<typename T>
void registerVideoScalingCommands(	Node& node,
									VideoScalingAttributes wr,
									VideoScalingAttributes rd,
									const std::string& parentPath = "video-scaling" );

}

#include "VideoScalingCommands.inl"