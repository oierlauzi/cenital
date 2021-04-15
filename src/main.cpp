#include "Mixer.h"

#include <zuazo/Instance.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Modules/FFmpeg.h>
#include <zuazo/Modules/Magick.h>
#include <zuazo/Modules/NDI.h>
#include <zuazo/Modules/Compositor.h>

#include <iostream>
#include <string>
#include <mutex>
#include <cstdint>

//For parsing argv
#include <tclap/CmdLine.h> 

using namespace Cenital;

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
		"Port used by the web socket", 		//Description
		false, 								//Required
		0, 									//Default value
		"Valid port number (1-65535)", 		//Type description
		cmd									//Command parser
	);

	//Create XORs between arguments

	//Parse the arguments
	cmd.parse(argc, argv);


	//Instantiate the Zuazo library
	const auto verbosity = 	verboseArg.getValue() ? 
							Zuazo::Verbosity::GEQ_VERBOSE : 
							Zuazo::Verbosity::GEQ_ERROR ;

	Zuazo::Instance::ApplicationInfo::Modules modules {
		Zuazo::Modules::Window::get(),
		Zuazo::Modules::FFmpeg::get(),
		Zuazo::Modules::Magick::get(),
		//Zuazo::Modules::NDI::get(), //TODO
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

	//Create the mixer
	Mixer mixer(instance, "Application");
	mixer.asyncOpen(lock);

	//Wait until quitting is requested by the user
	wait(lock, "quit");

	return 0;
}