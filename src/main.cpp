#include "Mixer.h"
#include "Controller.h"

#include "MixEffect.h"

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
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

using namespace Cenital;

//TODO Remove
static void messageCallback(websocketpp::server<websocketpp::config::asio>* s, 
							websocketpp::connection_hdl hdl, 
							websocketpp::server<websocketpp::config::asio>::message_ptr msg ) 
{
	std::cout << "on_message called with hdl: " << hdl.lock().get()
			  << " and message: " << msg->get_payload()
			  << std::endl;

	// check for a special command to instruct the server to stop listening so
	// it can be cleanly exited.
	if (msg->get_payload() == "stop-listening") {
		s->stop_listening();
		return;
	}

	try {
		s->send(hdl, msg->get_payload(), msg->get_opcode());
	} catch (websocketpp::exception const & e) {
		std::cout << "Echo failed because: "
				  << "(" << e.what() << ")" << std::endl;
	}
}



static std::unique_ptr<websocketpp::server<websocketpp::config::asio>> 
createWebSocket(boost::asio::io_service& ios,
				uint16_t port ) 
{
	std::unique_ptr<websocketpp::server<websocketpp::config::asio>> result;

	//Only create if the port is valid
	if(port > 0) {
		//Create the server object
		result = Zuazo::Utils::makeUnique<websocketpp::server<websocketpp::config::asio>>();
		
		//Initialize Asio
		result->init_asio(&ios);

		//Register the message callback //TODO
		result->set_message_handler(
			std::bind(&messageCallback, result.get(), std::placeholders::_1, std::placeholders::_2)
		);

		//Configure the port
		result->listen(port);

		//Start the server accept loop
		result->start_accept();
	}

	return result;
}

static void registerCommands(Controller& controller) {
	auto& root = controller.getRootNode();

	//Register the commands we know ahead of time
	Mixer::registerCommands(root);
	//MixEffect::registerCommands(root);
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
		"Port used by the web socket. 0 disables it. Default: 80",//Description
		false, 								//Required
		80, 								//Default value
		"port", 							//Type description
		cmd									//Command parser
	);

	//Create XORs between arguments

	//Parse the arguments
	cmd.parse(argc, argv);



	/*****************************
	 *    Zuazo Instantiating    *
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
	 *   Mixer  and controller   *
	 * 	    instantiation        *
	 *****************************/

	//Create the mixer
	Mixer mixer(instance, "Application");
	mixer.asyncOpen(lock);

	Controller controller(mixer);
	registerCommands(controller);



	/*****************************
	 *   Service Instantiating   *
	 *****************************/

	//Create Boost's I/O service
	boost::asio::io_service ios;

	//Instantiate the web socket
	const auto wsServer = createWebSocket(ios, webSocketPortArg.getValue());

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