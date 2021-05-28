/*#include <Sources/MediaPlayer.h>

#include <Mixer.h>

#include <Control/MixerNode.h>
#include <Control/Generic.h>

using namespace Zuazo;

namespace Cenital::Sources {

static void setState(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeSetter<MediaPlayer, MediaPlayer::State>(
		&MediaPlayer::setState,
		base, request, level, response
	);
}

static void getState(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeGetter<MediaPlayer::State, MediaPlayer>(
		&MediaPlayer::getState,
		base, request, level, response
	);
}


static void setRepeat(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeSetter<MediaPlayer, MediaPlayer::Repeat>(
		&MediaPlayer::setRepeat,
		base, request, level, response
	);
}

static void unsetRepeat(Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeSetter<MediaPlayer>(
		std::bind(std::mem_fn(&MediaPlayer::setRepeat), std::placeholders::_1, MediaPlayer::Repeat::NONE),
		base, request, level, response
	);
}

static void getRepeat(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeGetter<MediaPlayer::Repeat, MediaPlayer>(
		&MediaPlayer::getRepeat,
		base, request, level, response
	);
}


static void setSpeed(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeSetter<MediaPlayer, double>(
		&MediaPlayer::setPlaySpeed,
		base, request, level, response
	);
}

static void getSpeed(	Zuazo::ZuazoBase& base, 
						const Control::Message& request,
						size_t level,
						Control::Message& response ) 
{
	Control::invokeGetter<double, MediaPlayer>(
		&MediaPlayer::getPlaySpeed,
		base, request, level, response
	);
}





	
void MediaPlayer::registerCommands(Control::Node& node) {
	auto stateNode = Control::makeAttributeNode(
		Cenital::Sources::setState,
		Cenital::Sources::getState,
		Cenital::Control::enumerate<MediaPlayer::State>
	);

	auto repeatNode = Control::makeAttributeNode(
		Cenital::Sources::setRepeat,
		Cenital::Sources::getRepeat,
		Cenital::Control::enumerate<MediaPlayer::Repeat>,
		Cenital::Sources::unsetRepeat
	);

	auto speedNode = Control::makeAttributeNode(
		Cenital::Sources::setSpeed,
		Cenital::Sources::getSpeed
	);


	Control::MixerNode mediaPlayerNode(
		typeid(MediaPlayer),
		Control::invokeBaseConstructor<MediaPlayer, std::string>,
		Control::Node({
			{ "state", 			std::move(stateNode) },
			{ "repeat",			std::move(repeatNode) },
			{ "speed",			std::move(speedNode) }
		})
	);

	node.addPath("media-player", std::move(mediaPlayerNode));
}

}*/