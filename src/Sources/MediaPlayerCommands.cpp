#include <Sources/MediaPlayer.h>

#include <Mixer.h>

#include <Control/VideoModeCommands.h>
#include <Control/VideoScalingCommands.h>
#include <Control/Generic.h>

namespace Cenital::Sources {

using namespace Zuazo;
using namespace Control;

class ClipConfigNode {
public:
	using Callback = Node::Callback;

	ClipConfigNode() = default;
	ClipConfigNode(Callback callback)
		: m_callback(std::move(callback))
	{
	}

	ClipConfigNode(const ClipConfigNode& other) = default;
	ClipConfigNode(ClipConfigNode&& other) = default;
	~ClipConfigNode() = default;

	ClipConfigNode&	operator=(const ClipConfigNode& other) = default;
	ClipConfigNode&	operator=(ClipConfigNode&& other) = default;

	void setCallback(Callback cbk) {
		m_callback = std::move(cbk);
	}

	const Callback& getCabllback() const noexcept {
		return m_callback;
	}

	void operator()(Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response ) const
	{
		const auto& tokens = request.getPayload();
		if(tokens.size() > level) {
			assert(typeid(base) == typeid(MediaPlayer));
			auto& mp = static_cast<MediaPlayer&>(base);
			const auto& clipName = tokens[level];

			//Try to find the requested clip
			auto* clip = mp.getClip(clipName);
			if(clip && m_callback) {
				//Success! Invoke the associated callback
				Utils::invokeIf(m_callback, *clip, request, level + 1, response);
			}
		}
	}

private:
	Callback		m_callback;

};



static void setClipState(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter<MediaPlayer::Clip, ClipBase::State>(
		&MediaPlayer::Clip::setState,
		base, request, level, response
	);
}

static void getClipState(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter<ClipBase::State, MediaPlayer::Clip>(
		&MediaPlayer::Clip::getState,
		base, request, level, response
	);
}

static void enumClipState(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	enumerate<ClipBase::State>(base, request, level, response);
}


static void setClipRepeat(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter<MediaPlayer::Clip, ClipBase::Repeat>(
		&MediaPlayer::Clip::setRepeat,
		base, request, level, response
	);
}

static void unsetClipRepeat(Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter<MediaPlayer::Clip>(
		[] (MediaPlayer::Clip& clip) {
			clip.setRepeat(ClipBase::Repeat::none);
		},
		base, request, level, response
	);
}

static void getClipRepeat(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter<ClipBase::Repeat, MediaPlayer::Clip>(
		&MediaPlayer::Clip::getRepeat,
		base, request, level, response
	);
}

static void enumClipRepeat(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	enumerate<ClipBase::Repeat>(base, request, level, response);
}


static void setClipSpeed(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeSetter<MediaPlayer::Clip, double>(
		&MediaPlayer::Clip::setPlaySpeed,
		base, request, level, response
	);
}

static void getClipSpeed(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response ) 
{
	invokeGetter<double, MediaPlayer::Clip>(
		&MediaPlayer::Clip::getPlaySpeed,
		base, request, level, response
	);
}





static void addClip(Zuazo::ZuazoBase& base, 
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

static void rmClip(	Zuazo::ZuazoBase& base, 
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

static void enumClips(	Zuazo::ZuazoBase& base, 
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

static void setClip(Zuazo::ZuazoBase& base, 
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

static void getClip(Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response ) 
{
	invokeGetter<std::string, MediaPlayer>(
		[] (const MediaPlayer& mp) -> std::string {
			const auto* clip = mp.getCurrentClip();
			return clip ? clip->getName() : "";
		},
		base, request, level, response
	);
}

static void unsetClip(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response ) 
{
	invokeSetter<MediaPlayer>(
		[] (MediaPlayer& mp) {
			const auto ret = mp.setCurrentClip("");
			assert(!ret); Utils::ignore(ret);
		},
		base, request, level, response
	);
}


void MediaPlayer::registerCommands(Node& node) {
	Node clipConfigNode({
		{ "state", 	makeAttributeNode(	setClipState,
										getClipState,
										enumClipState )},
		{ "repeat", makeAttributeNode(	setClipRepeat,
										getClipRepeat,
										enumClipRepeat,
										unsetClipRepeat )},
		{ "speed", 	makeAttributeNode(	setClipSpeed,
										getClipSpeed )},

	});

	Node clipNode({
		{ "add",	Sources::addClip },
		{ "rm",		Sources::rmClip },
		{ "enum",	Sources::enumClips },
		{ "set",	Sources::setClip },
		{ "get",	Sources::getClip },
		{ "unset",	Sources::unsetClip },
		{ "config",	ClipConfigNode(std::move(clipConfigNode)) }

	});

	Node configNode({
		{ "clip",	std::move(clipNode) },
	});

	Mixer::registerClass(
		node, 
		typeid(MediaPlayer), 
		"input-media-player", 
		invokeBaseConstructor<MediaPlayer>,
		std::move(configNode)
	);
}

}