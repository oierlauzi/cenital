#include "Mixer.h"

#include "MixEffect.h"
#include "Sources/MediaPlayer.h"

#include "Control/Controller.h"
#include "Control/CLIView.h"
#include "Control/WebSocketServer.h"
#include "Control/TCPServer.h"

#include <zuazo/Instance.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Modules/FFmpeg.h>
#include <zuazo/Modules/Magick.h>
#include <zuazo/Modules/NDI.h>
#include <zuazo/Modules/Compositor.h>

#include <iostream>
#include <string>
#include <mutex>
#include <thread>
#include <cstdint>

#include <tclap/CmdLine.h>

using namespace Cenital;

static void registerCommands(Control::Controller& controller) {
	auto& root = controller.getRootNode();

	//Register the commands we know ahead of time
	Mixer::registerCommands(root);
	MixEffect::registerCommands(root);
	Sources::MediaPlayer::registerCommands(root);
}



static std::unique_ptr<Control::WebSocketServer> createWebSocketServer(	boost::asio::io_service& ios,
																		uint16_t port,
																		Control::CLIView& cliView )
{
	std::unique_ptr<Control::WebSocketServer> result;

	if(port > 0) { //0 port is used to disable the service
		result = Zuazo::Utils::makeUnique<Control::WebSocketServer>(ios, port);

		result->setConnectionOpenCallback(
			[&cliView, &server = *result] (Control::WebSocketServer::SessionPtr session) -> void {
				cliView.addListener(
					[&server, session] (const std::string& msg) -> void {
						server.send(session, msg);
					}
				);
			}
		);
		//TODO no close callback
		result->setMessageCallback(
			[&cliView, &srv = *result] (Control::WebSocketServer::SessionPtr session, Control::WebSocketServer::Message msg) {
				std::string response;
				cliView.parse(msg->get_payload(), response);
				srv.send(std::move(session), std::move(response));
			}
		);

		result->startAccept();
	}

	return result;
}

static std::unique_ptr<Control::TCPServer> createTCPServer(	boost::asio::io_service& ios,
															uint16_t port,
															Control::CLIView& cliView  )
{
	std::unique_ptr<Control::TCPServer> result;

	if(port > 0) { //0 port is used to disable the service
		result = Zuazo::Utils::makeUnique<Control::TCPServer>(ios, port);

		result->setConnectionOpenCallback(
			[&cliView, &server = *result] (Control::TCPServer::SessionPtr session) -> void {
				cliView.addListener(
					[&server, session] (const std::string& msg) -> void {
						server.send(session, msg);
					}
				);
			}
		);
		//TODO no close callback
		result->setMessageCallback(
			[&cliView, &srv = *result] (Control::TCPServer::SessionPtr session, std::string msg) {
				std::string response;
				cliView.parse(msg, response);
				srv.send(std::move(session), std::move(response));
			}
		);

		result->startAccept();
	}

	return result;
}



static void wait(std::unique_lock<Zuazo::Instance>& lock, std::string_view keyword) {
	//Show running message
	std::cerr << "Running... Type \"" << keyword << "\" and press ENTER to terminate" << std::endl;

	//Wait until the keyword is entered
	lock.unlock();
	std::string response;
	do {
		std::getline(std::cin, response);
	} while(response != keyword);
	lock.lock();
}





int main(int argc, const char* const* argv) {
	constexpr const char* appName = "Cenital";
	constexpr Zuazo::Version version(0, 1, 0);

	/*****************************
	 *      Argument parsing     *
	 *****************************/

	//Parse input arguments
	//Create the parser
	TCLAP::CmdLine cmd(
		appName, 							//Message
		' ', 								//Delimiter
		Zuazo::toString(version), 			//Version
		true								//Help
	);

	//Create the arguments
	TCLAP::SwitchArg verboseArg(
		"v", "verbose", 					//Arguments
		"Show debug information in stderr", //Description
		cmd 								//Command parser
	);
	TCLAP::ValueArg<uint16_t> webSocketPortArg(
		"w", "web-socket-port", 			//Arguments
		"Port used by the WebSocket CLI. 0 disables it. Default: 80",//Description
		false, 								//Required
		80, 								//Default value
		"port", 							//Type description
		cmd									//Command parser
	);
	TCLAP::ValueArg<uint16_t> tcpPortArg(
		"t", "tcp-port", 					//Arguments
		"Port used by the TCP CLI. 0 disables it. Default: 9600",//Description
		false, 								//Required
		9600, 								//Default value
		"port", 							//Type description
		cmd									//Command parser
	);
	

	//Create XORs between arguments

	//Parse the arguments
	cmd.parse(argc, argv);



	/*****************************
	 *    Zuazo instantiation    *
	 *****************************/

	//Instantiate the Zuazo library
	const auto verbosity = 	verboseArg.getValue() ? 
							Zuazo::Verbosity::GEQ_VERBOSE : 
							Zuazo::Verbosity::GEQ_ERROR ;

	Zuazo::Instance::ApplicationInfo::Modules modules {
		Zuazo::Modules::Window::get(),
		Zuazo::Modules::FFmpeg::get(),
		Zuazo::Modules::Magick::get(),
		Zuazo::Modules::NDI::get(),
		Zuazo::Modules::Compositor::get(),
	};

	Zuazo::Instance::ApplicationInfo appInfo(
		appName,					//Application name
		version,					//Application version
		verbosity,					//Verbosity
		std::move(modules)			//Enabled modules
	);

	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);



	/*****************************
	 *    Mixer instantiation    *
	 *****************************/

	//Create the mixer
	Mixer mixer(instance, "Application");
	mixer.asyncOpen(lock);



	/*****************************
	 *    Controller and views   *
	 *       instantiation       *
	 *****************************/

	Control::Controller controller(mixer);
	registerCommands(controller);

	Control::CLIView cliView(controller);
	controller.addView(cliView);



	/*****************************
	 *   Service instantiation   *
	 *****************************/

	//Create Boost's I/O service
	boost::asio::io_service ios;

	//Instantiate all the services
	const auto webSocketServer = createWebSocketServer(
		ios, 
		webSocketPortArg.getValue(),
		cliView
	);
	const auto tcpServer = createTCPServer(
		ios, 
		tcpPortArg.getValue(),
		cliView
	);

	//Create a thread for running the services
	std::thread serviceThread(
		[&ios] () {
			ios.run();
		}	
	);

	

	/*****************************
	 *  Wait completion and exit *
	 *****************************/

	//Wait until quitting is requested by the user
	wait(lock, "quit");

	//Finish the service
	ios.stop();
	serviceThread.join();

	return 0;
}