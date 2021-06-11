#pragma once

#include "../Control/Controller.h"

#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>
#include <zuazo/Sources/FFmpegClip.h>
#include <zuazo/Signal/SourceLayout.h>
#include <zuazo/Utils/Pimpl.h>

namespace Cenital::Sources {

struct MediaPlayerImpl;
class MediaPlayer 
	: private Zuazo::Utils::Pimpl<MediaPlayerImpl>
	, public Zuazo::ZuazoBase
	, public Zuazo::Signal::SourceLayout<Zuazo::Video>
{
	friend MediaPlayerImpl;
public:
	using Clip = Zuazo::Sources::FFmpegClip;

	MediaPlayer(Zuazo::Instance& instance,
				std::string name );

	virtual ~MediaPlayer();

	bool 											addClip(std::string name, std::string path);
	bool 											addClip(std::string path);
	bool 											rmClip(std::string_view name);
	std::vector<std::reference_wrapper<Clip>> 		getClips();
	std::vector<std::reference_wrapper<const Clip>> getClips() const;
	Clip*											getClip(std::string_view name);
	const Clip* 									getClip(std::string_view name) const;

	
	bool											setCurrentClip(std::string_view name);
	Clip*											getCurrentClip() noexcept;
	const Clip*										getCurrentClip() const noexcept;



	static void 									registerCommands(Control::Controller& controller);

private:

};
	
}