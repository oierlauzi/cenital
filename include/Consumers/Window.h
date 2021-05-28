#pragma once

#include <zuazo/Consumers/RendererWrapper.h>
#include <zuazo/Renderers/Window.h>

namespace Cenital::Consumers {

class Window 
	: public Zuazo::Consumers::RendererWrapper<Zuazo::Renderers::Window>
{
public:
	using Monitor = Zuazo::Renderers::Window::Monitor;

	Window(	Zuazo::Instance& instance,
			std::string name,
			Zuazo::Math::Vec2i size );
	Window(const Window& other) = delete;
	Window(Window&& other) = delete;
	~Window() = default;


	static Monitor							getPrimaryMonitor();
	static Utils::BufferView<const Monitor>	getMonitors();
};


}