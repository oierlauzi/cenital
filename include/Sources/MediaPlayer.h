#pragma once

#include "../Control/Node.h"

#include <zuazo/Sources/FFmpegClip.h>

namespace Cenital::Sources {

struct MediaPlayer : Zuazo::Sources::FFmpegClip {
	using Zuazo::Sources::FFmpegClip::FFmpegClip;

	static void registerCommands(Control::Node& node);
};
	
}