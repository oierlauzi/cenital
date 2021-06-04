#include <Consumers/Window.h>

#include <Mixer.h>
#include <Control/Generic.h>
#include <Control/VideoModeCommands.h>
#include <Control/VideoScalingCommands.h>


namespace Cenital::Consumers {

using namespace Zuazo;
using namespace Control;

static void setTitle(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response )
{
	invokeSetter<Window, std::string>(
		&Window::setTitle,
		base, request, level, response
	);
}

static void getTitle(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response )
{
	invokeGetter<std::string, Window>(
		&Window::getTitle,
		base, request, level, response
	);
}


static void setSize(Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response )
{
	invokeSetter<Window, Math::Vec2i>(
		&Window::setSize,
		base, request, level, response
	);
}

static void getSize(Zuazo::ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response )
{
	invokeGetter<Math::Vec2i, Window>(
		&Window::getSize,
		base, request, level, response
	);
}


static void setPosition(Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response )
{
	invokeSetter<Window, Math::Vec2i>(
		&Window::setPosition,
		base, request, level, response
	);
}

static void getPosition(Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response )
{
	invokeGetter<Math::Vec2i, Window>(
		&Window::getPosition,
		base, request, level, response
	);
}


static void setOpacity(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response )
{
	invokeSetter<Window, float>(
		&Window::setOpacity,
		base, request, level, response
	);
}

static void getOpacity(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response )
{
	invokeGetter<float, Window>(
		&Window::getOpacity,
		base, request, level, response
	);
}


static void setResizeable(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	invokeSetter<Window, bool>(
		&Window::setResizeable,
		base, request, level, response
	);
}

static void getResizeable(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	invokeGetter<bool, Window>(
		&Window::getResizeable,
		base, request, level, response
	);
}


static void setDecorated(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	invokeSetter<Window, bool>(
		&Window::setDecorated,
		base, request, level, response
	);
}

static void getDecorated(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	invokeGetter<bool, Window>(
		&Window::getDecorated,
		base, request, level, response
	);
}


static void setMonitor(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response )
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level + 1) {
		const auto& monitorName = tokens[level];

		//Find the requested monitor
		const auto monitors = Window::getMonitors();
		const auto ite = std::find_if(
			monitors.cbegin(), monitors.cend(),
			[&monitorName] (Window::Monitor monitor) -> bool {
				assert(monitor != Renderers::Window::NO_MONITOR);
				return monitor.getName() == monitorName;
			}
		);

		if(ite != monitors.cend()) {
			//Found it!
			assert(typeid(base) == typeid(Window));
			auto& window = static_cast<Window&>(base);

			//Get the current mode
			const auto mode = ite->getMode();

			//Set the monitor in the window
			window.setMonitor(*ite, &mode);

			//Elaborate the response
			response.getPayload() = tokens;
			response.setType(Message::Type::broadcast);
		}
	}
}

static void getMonitor(	Zuazo::ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response )
{
	invokeGetter<std::string, Window>(
		[] (const Window& window) -> std::string {
			const auto monitor = window.getMonitor();
			return (monitor != Renderers::Window::NO_MONITOR) ? std::string(monitor.getName()) : "";
		},
		base, request, level, response
	);
}

static void enumMonitor(Zuazo::ZuazoBase&, 
						const Message& request,
						size_t level,
						Message& response )
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level) {
		const auto monitors = Window::getMonitors();

		auto& payload = response.getPayload();
		payload.clear();
		payload.reserve(monitors.size());
		std::transform(
			monitors.cbegin(), monitors.cend(),
			std::back_inserter(payload),
			[] (const Window::Monitor& mon) -> std::string {
				return std::string(mon.getName());
			}
		);
		response.setType(Message::Type::response);
	}
}

static void unsetMonitor(	Zuazo::ZuazoBase& base, 
							const Message& request,
							size_t level,
							Message& response )
{
	invokeSetter<Window>(
		[] (Window& window) {
			window.setMonitor(Renderers::Window::NO_MONITOR, nullptr);
		},
		base, request, level, response
	);
}



void Window::registerCommands(Node& node) {


	Node configNode({
		{ "title",				makeAttributeNode(	Consumers::setTitle,
													Consumers::getTitle ) },
		{ "size",				makeAttributeNode(	Consumers::setSize,
													Consumers::getSize ) },
		{ "position",			makeAttributeNode(	Consumers::setPosition,
													Consumers::getPosition ) },
		{ "opacity",			makeAttributeNode(	Consumers::setOpacity,
													Consumers::getOpacity ) },
		{ "resizeable",			makeAttributeNode(	Consumers::setResizeable,
													Consumers::getResizeable ) },
		{ "decorated",			makeAttributeNode(	Consumers::setDecorated,
													Consumers::getDecorated ) },
		{ "monitor",			makeAttributeNode(	Consumers::setMonitor,
													Consumers::getMonitor,
													Consumers::enumMonitor,
													Consumers::unsetMonitor ) },

	});

	constexpr auto videoModeWr = 
		VideoModeAttributes::frameRate |
		VideoModeAttributes::colorPrimaries |
		VideoModeAttributes::colorTransferFunction |
		VideoModeAttributes::colorFormat ;
	constexpr auto videoModeRd = 
		VideoModeAttributes::frameRate |
		VideoModeAttributes::resolution |
		VideoModeAttributes::colorPrimaries |
		VideoModeAttributes::colorTransferFunction |
		VideoModeAttributes::colorFormat ;
	registerVideoModeCommands<Window>(configNode, videoModeWr, videoModeRd);

	constexpr auto videoScalingWr = 
		VideoScalingAttributes::mode |
		VideoScalingAttributes::filter ;
	constexpr auto videoScalingRd = 
		VideoScalingAttributes::mode |
		VideoScalingAttributes::filter ;
	registerVideoScalingCommands<Window>(configNode, videoScalingWr, videoScalingRd);

	Mixer::registerClass(
		node, 
		typeid(Window), 
		"output-window", 
		invokeBaseConstructor<Window, Math::Vec2i>,
		std::move(configNode)
	);
}

}