#include "MixTransition.h" //TODO remove from here

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

	const auto windowSize = Zuazo::Math::Vec2i(1920, 1080);
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

	//Open the window (now becomes visible)
	window.asyncOpen(lock);

	Zuazo::Sources::FFmpegClip clip1(
		instance, 
		"Clip 1",
		Zuazo::VideoMode::ANY,
		argv[1]
	);
	clip1.setRepeat(Zuazo::ClipBase::Repeat::REPEAT);
	clip1.play();
	clip1.asyncOpen(lock);

	Zuazo::Sources::FFmpegClip clip2(
		instance, 
		"Clip 2",
		Zuazo::VideoMode::ANY,
		argv[2]
	);
	clip2.setRepeat(Zuazo::ClipBase::Repeat::REPEAT);
	clip2.play();
	clip2.asyncOpen(lock);

	MixTransition transition(instance, "DVE Transition");
	transition.setRepeat(Zuazo::ClipBase::Repeat::PING_PONG);
	transition.play();
	transition.setEffect(MixTransition::Effect::BLACK);
	transition.setRenderer(&window);
	transition.setSize(window.getSize());
	transition.asyncOpen(lock);

	transition.getPrevIn() << clip1;
	transition.getPostIn() << clip2;

	Zuazo::Player player1(instance, &clip1);
	Zuazo::Player player2(instance, &clip2);
	Zuazo::Player player3(instance, &transition);
	player1.enable();
	player2.enable();
	player3.enable();

	window.setViewportSizeCallback(
		std::bind(&TransitionBase::setSize, &transition, std::placeholders::_2)
	);
	window.setLayers(transition.getLayers());


	//Wait
	lock.unlock();
	getchar();
	lock.lock();

	return 0;
}