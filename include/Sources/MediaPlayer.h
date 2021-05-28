#pragma once

#include "../Control/Node.h"

#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>
#include <zuazo/Utils/Pimpl.h>

namespace Cenital::Sources {

struct MediaPlayerImpl;
class MediaPlayer 
	: private Zuazo::Utils::Pimpl<MediaPlayerImpl>
	, public Zuazo::ZuazoBase
	, public Zuazo::VideoBase
{
	MediaPlayer(Zuazo::Instance& instance,
				std::string name );

	static void registerCommands(Control::Node& node);

private:

};
	
}