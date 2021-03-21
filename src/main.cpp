#include "Keyer.h" //TODO remove from here

#include <zuazo/Instance.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Modules/FFmpeg.h>
#include <zuazo/Modules/Compositor.h>


#include <zuazo/Consumers/WindowRenderer.h> //TODO remove from here
#include <zuazo/Sources/FFmpegClip.h>
#include <zuazo/Signal/Input.h>
#include <zuazo/Player.h>

#include <iostream>


using namespace Cenital;

int main(int argc, const char* argv[]) {
	if(argc != 2) {
		std::terminate();
	} 

	//Instantiate the Zuazo library
	Zuazo::Instance::ApplicationInfo::Modules modules {
		Zuazo::Modules::Window::get(),
		Zuazo::Modules::FFmpeg::get(),
		Zuazo::Modules::Compositor::get(),
	};

	Zuazo::Instance::ApplicationInfo appInfo(
		"Cenital",					//Application name
		Zuazo::Version(0, 1, 0),	//Application version
		Zuazo::Verbosity::GEQ_INFO,	//Verbosity
		std::move(modules)			//Enabled modules
	);

	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);


	//Construct the desired parameters
	const Zuazo::VideoMode windowVideoMode(
		Zuazo::Utils::MustBe<Zuazo::Rate>(Zuazo::Rate(25, 1)), //Just specify the desired rate
		Zuazo::Utils::Any<Zuazo::Resolution>(),
		Zuazo::Utils::Any<Zuazo::AspectRatio>(),
		Zuazo::Utils::Any<Zuazo::ColorPrimaries>(),
		Zuazo::Utils::Any<Zuazo::ColorModel>(),
		Zuazo::Utils::Any<Zuazo::ColorTransferFunction>(),
		Zuazo::Utils::Any<Zuazo::ColorSubsampling>(),
		Zuazo::Utils::Any<Zuazo::ColorRange>(),
		Zuazo::Utils::Any<Zuazo::ColorFormat>()	
	);

	const Zuazo::Utils::Limit<Zuazo::DepthStencilFormat> depthStencil(
		Zuazo::Utils::MustBe<Zuazo::DepthStencilFormat>(Zuazo::DepthStencilFormat::NONE) //Not interested in the depth buffer
	);

	const auto windowSize = Zuazo::Math::Vec2i(1280, 720);
	const auto& monitor = Zuazo::Consumers::WindowRenderer::NO_MONITOR; //Not interested in the full-screen mode

	//Construct the window object
	Zuazo::Consumers::WindowRenderer window(
		instance, 						//Instance
		"Output Window",				//Layout name
		windowVideoMode,				//Video mode limits
		depthStencil,					//Depth buffer limits
		windowSize,						//Window size (in screen coordinates)
		monitor							//Monitor for setting fullscreen
	);
	window.setResizeable(false); //Disable resizeing, as extra care needs to be taken

	//Open the window (now becomes visible)
	window.asyncOpen(lock);

	//Construct the keyer
	Keyer keyer(
		instance,
		"Test keyer",
		&window,
		window.getVideoMode().getFrameDescriptor().calculateSize() //Will set it fullscreen
	);

	//Configure the keyer alike
	keyer.setScalingMode(Zuazo::ScalingMode::CROPPED);
	keyer.setScalingFilter(Zuazo::ScalingFilter::NEAREST);
	keyer.setOpacity(1.0f);
	keyer.setChromaKeyEnabled(true);

	//Open the keyer
	keyer.asyncOpen(lock);

	//Create a video source
	Zuazo::Sources::FFmpegClip videoClip(
		instance,
		"Video Source",
		Zuazo::VideoMode::ANY,
		std::string(argv[1])
	);

	videoClip.play();
	videoClip.setRepeat(Zuazo::ClipBase::Repeat::REPEAT);
	videoClip.asyncOpen(lock);

	//Create a player for playing the clip
	Zuazo::Player clipPlayer(instance, &videoClip);
	clipPlayer.enable();

	//Route the signal
	Zuazo::Signal::getInput<Zuazo::Video>(keyer, "keyIn") << videoClip;
	Zuazo::Signal::getInput<Zuazo::Video>(keyer, "fillIn") << videoClip;
	window.setLayers({keyer});

	//Wait
	lock.unlock();
	getchar();
	lock.lock();

	return 0;
}