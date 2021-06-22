#include <Consumers/Window.h>

#include <Mixer.h>
#include <Control/Generic.h>
#include <Control/VideoModeCommands.h>
#include <Control/VideoScalingCommands.h>


namespace Cenital::Consumers {

using namespace Zuazo;
using namespace Control;

static void setTitle(	Controller& controller,
						ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response )
{
	invokeSetter<Window, std::string>(
		&Window::setTitle,
		controller, base, request, level, response
	);
}

static void getTitle(	Controller& controller,
						ZuazoBase& base,  
						const Message& request,
						size_t level,
						Message& response )
{
	invokeGetter<std::string, Window>(
		&Window::getTitle,
		controller, base, request, level, response
	);
}


static void setSize(Controller& controller,
					ZuazoBase& base, 
					const Message& request,
					size_t level,
					Message& response )
{
	invokeSetter<Window, Math::Vec2i>(
		&Window::setSize,
		controller, base, request, level, response
	);
}

static void getSize(Controller& controller,
					ZuazoBase& base,
					const Message& request,
					size_t level,
					Message& response )
{
	invokeGetter<Math::Vec2i, Window>(
		&Window::getSize,
		controller, base, request, level, response
	);
}


static void setPosition(Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response )
{
	invokeSetter<Window, Math::Vec2i>(
		&Window::setPosition,
		controller, base, request, level, response
	);
}

static void getPosition(Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response )
{
	invokeGetter<Math::Vec2i, Window>(
		&Window::getPosition,
		controller, base, request, level, response
	);
}


static void setOpacity(	Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response )
{
	invokeSetter<Window, float>(
		&Window::setOpacity,
		controller, base, request, level, response
	);
}

static void getOpacity(	Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response )
{
	invokeGetter<float, Window>(
		&Window::getOpacity,
		controller, base, request, level, response
	);
}


static void setResizeable(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response )
{
	invokeSetter<Window, bool>(
		&Window::setResizeable,
		controller, base, request, level, response
	);
}

static void getResizeable(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response )
{
	invokeGetter<bool, Window>(
		&Window::getResizeable,
		controller, base, request, level, response
	);
}


static void setDecorated(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response )
{
	invokeSetter<Window, bool>(
		&Window::setDecorated,
		controller, base, request, level, response
	);
}

static void getDecorated(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response )
{
	invokeGetter<bool, Window>(
		&Window::getDecorated,
		controller, base, request, level, response
	);
}


static void setMonitor(	Controller&,
						ZuazoBase& base,
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

static void getMonitor(	Controller& controller,
						ZuazoBase& base,
						const Message& request,
						size_t level,
						Message& response )
{
	invokeGetter<std::string, Window>(
		[] (const Window& window) -> std::string {
			const auto monitor = window.getMonitor();
			return (monitor != Renderers::Window::NO_MONITOR) ? std::string(monitor.getName()) : "";
		},
		controller, base, request, level, response
	);
}

static void enumMonitor(Controller&,
						ZuazoBase&, 
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

static void unsetMonitor(	Controller& controller,
							ZuazoBase& base,
							const Message& request,
							size_t level,
							Message& response )
{
	invokeSetter<Window>(
		[] (Window& window) {
			window.setMonitor(Renderers::Window::NO_MONITOR, nullptr);
		},
		controller, base, request, level, response
	);
}



void Window::registerCommands(Controller& controller) {


	Node configNode({
		{ "title",				makeAttributeNode(	Consumers::setTitle,
													Consumers::getTitle ) },
		{ "size",				makeAttributeNode(	Consumers::setSize,
													Consumers::getSize ) },
		{ "position",			makeAttributeNode(	Consumers::setPosition,
													Consumers::getPosition ) },
		{ "opacity",			makeAttributeNode(	Consumers::setOpacity,
													Consumers::getOpacity ) },
		{ "resizable",			makeAttributeNode(	Consumers::setResizeable,
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
		VideoModeAttributes::all ;
	registerVideoModeCommands<Window>(configNode, videoModeWr, videoModeRd);

	constexpr auto videoScalingWr = 
		VideoScalingAttributes::mode |
		VideoScalingAttributes::filter ;
	constexpr auto videoScalingRd = 
		VideoScalingAttributes::mode |
		VideoScalingAttributes::filter ;
	registerVideoScalingCommands<Window>(configNode, videoScalingWr, videoScalingRd);

	//Register it
	auto& classIndex = controller.getClassIndex();
	classIndex.registerClass(
		typeid(Window),
		ClassIndex::Entry(
			"output-window",
			std::move(configNode),
			invokeBaseConstructor<Window, Math::Vec2f>,
			typeid(ZuazoBase)
		)	
	);
}

}