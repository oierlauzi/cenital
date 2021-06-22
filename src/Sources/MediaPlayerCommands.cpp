#include <Sources/MediaPlayer.h>

#include <Mixer.h>

#include <Control/ElementNode.h>
#include <Control/VideoModeCommands.h>
#include <Control/VideoScalingCommands.h>
#include <Control/Generic.h>

namespace Cenital::Sources {

using namespace Zuazo;
using namespace Control;

static void setClipState(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter<MediaPlayer::Clip, ClipBase::State>(
		&MediaPlayer::Clip::setState,
		controller, base, request, level, response
	);
}

static void getClipState(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter<ClipBase::State, MediaPlayer::Clip>(
		&MediaPlayer::Clip::getState,
		controller, base, request, level, response
	);
}

static void enumClipState(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	enumerate<ClipBase::State>(controller, base, request, level, response);
}


static void setClipRepeat(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter<MediaPlayer::Clip, ClipBase::Repeat>(
		&MediaPlayer::Clip::setRepeat,
		controller, base, request, level, response
	);
}

static void unsetClipRepeat(Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter<MediaPlayer::Clip>(
		[] (MediaPlayer::Clip& clip) {
			clip.setRepeat(ClipBase::Repeat::none);
		},
		controller, base, request, level, response
	);
}

static void getClipRepeat(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter<ClipBase::Repeat, MediaPlayer::Clip>(
		&MediaPlayer::Clip::getRepeat,
		controller, base, request, level, response
	);
}

static void enumClipRepeat(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	enumerate<ClipBase::Repeat>(controller, base, request, level, response);
}


static void setClipSpeed(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter<MediaPlayer::Clip, double>(
		&MediaPlayer::Clip::setPlaySpeed,
		controller, base, request, level, response
	);
}

static void getClipSpeed(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter<double, MediaPlayer::Clip>(
		&MediaPlayer::Clip::getPlaySpeed,
		controller, base, request, level, response
	);
}


static void setClipTime(Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeSetter<MediaPlayer::Clip, Duration>(
		[] (MediaPlayer::Clip& clip, Duration dur) {
			clip.setTime(TimePoint(dur));
		},
		controller, base, request, level, response
	);
}

static void getClipTime(Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeGetter<Duration, MediaPlayer::Clip>(
		[] (MediaPlayer::Clip& clip) -> Duration {
			return clip.getTime().time_since_epoch();
		},
		controller, base, request, level, response
	);
}

static void getClipDuration(Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter<Duration, MediaPlayer::Clip>(
		&ClipBase::getDuration,
		controller, base, request, level, response
	);
}




static void addClip(Controller&,
					ZuazoBase& base,
					const Message& request,
					size_t level,
					Message& response ) 
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level + 1) {
		assert(typeid(base) == typeid(MediaPlayer));
		auto& mp = static_cast<MediaPlayer&>(base);

		//Try to add the clip
		const auto& clipName = tokens[level];
		const auto ret = mp.addClip(clipName);

		//Elaborate the response
		if(ret) {
			response.setType(Message::Type::broadcast);
			response.getPayload() = tokens;
		}
	}
}

static void rmClip(	Controller&,
					ZuazoBase& base,
					const Message& request,
					size_t level,
					Message& response ) 
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level + 1) {
		assert(typeid(base) == typeid(MediaPlayer));
		auto& mp = static_cast<MediaPlayer&>(base);

		//Try to add the clip
		const auto& clipName = tokens[level];
		const auto ret = mp.rmClip(clipName);

		//Elaborate the response
		if(ret) {
			response.setType(Message::Type::broadcast);
			response.getPayload() = tokens;
		}
	}
}

static void enumClips(	Controller&,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response ) 
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level) {
		assert(typeid(base) == typeid(MediaPlayer));
		const auto& mp = static_cast<const MediaPlayer&>(base);

		const auto clips = mp.getClips();
		
		std::vector<std::string>& payload = response.getPayload();
		payload.clear();
		payload.reserve(clips.size());
		std::transform(
			clips.cbegin(), clips.cend(),
			std::back_inserter(payload),
			[] (const MediaPlayer::Clip& clip) -> std::string {
				return clip.getName();
			}
		);

		response.setType(Message::Type::response);
	}
}

static Zuazo::ZuazoBase* getClip(Zuazo::ZuazoBase& base, const std::string clipName) {
	assert(typeid(base) == typeid(MediaPlayer));
	auto& mp = static_cast<MediaPlayer&>(base);
	return mp.getClip(clipName);
}

static void setCurrentClip(	Controller&,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level + 1) {
		assert(typeid(base) == typeid(MediaPlayer));
		auto& mp = static_cast<MediaPlayer&>(base);

		//Try to set the clip
		const auto& clipName = tokens[level];
		const auto ret = mp.setCurrentClip(clipName);

		//Elaborate the response
		if(ret) {
			response.setType(Message::Type::broadcast);
			response.getPayload() = tokens;
		}
	}
}

static void getCurrentClip(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter<std::string, MediaPlayer>(
		[] (const MediaPlayer& mp) -> std::string {
			const auto* clip = mp.getCurrentClip();
			return clip ? clip->getName() : "";
		},
		controller, base, request, level, response
	);
}

static void unsetCurrentClip(	Controller& controller,
								ZuazoBase& base,
								const Message& request,
								size_t level,
								Message& response ) 
{
	invokeSetter<MediaPlayer>(
		[] (MediaPlayer& mp) {
			const auto ret = mp.setCurrentClip("");
			assert(!ret); Utils::ignore(ret);
		},
		controller, base, request, level, response
	);
}





void MediaPlayer::registerCommands(Controller& controller) {
	Node clipConfigNode({
		{ "state", 		makeAttributeNode(	setClipState,
											getClipState,
											enumClipState )},
		{ "repeat", 	makeAttributeNode(	setClipRepeat,
											getClipRepeat,
											enumClipRepeat,
											unsetClipRepeat )},
		{ "speed", 		makeAttributeNode(	setClipSpeed,
											getClipSpeed )},
		{ "time", 		makeAttributeNode(	setClipTime,
											getClipTime )},
		{ "duration", 	makeAttributeNode(	{},
											getClipDuration )},

	});

	//Register the clip. It should not be instantiated
	//Do all de possible to prevent referencing it
	auto& classIndex = controller.getClassIndex();
	classIndex.registerClass(
		typeid(MediaPlayer::Clip),
		ClassIndex::Entry(
			"input-media-player-clip",
			std::move(clipConfigNode),
			{},
			typeid(void)
		)	
	);


	Node clipNode({
		{ "add",	Sources::addClip },
		{ "rm",		Sources::rmClip },
		{ "enum",	Sources::enumClips },
		{ "set",	Sources::setCurrentClip },
		{ "get",	Sources::getCurrentClip },
		{ "unset",	Sources::unsetCurrentClip },
		{ "config",	ElementNode(Sources::getClip) }

	});

	Node configNode({
		{ "clip",	std::move(clipNode) },
	});

	//Register it
	classIndex.registerClass(
		typeid(MediaPlayer),
		ClassIndex::Entry(
			"input-media-player",
			std::move(configNode),
			invokeBaseConstructor<MediaPlayer>,
			typeid(ZuazoBase)
		)	
	);
}

}