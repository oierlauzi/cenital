#include <Sources/MediaPlayer.h>

#include <Mixer.h>

#include <Control/MixerNode.h>
#include <Control/Generic.h>

namespace Cenital::Sources {


	
void MediaPlayer::registerCommands(Control::Node& node) {
	Control::MixerNode mixerNode(
		typeid(MediaPlayer),
		Control::invokeBaseConstructor<MediaPlayer, std::string>,
		Control::Node({
			//TODO
		})
	);

	node.addPath("media-player", std::move(mixerNode));
}

}