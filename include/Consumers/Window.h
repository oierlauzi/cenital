#pragma once

#include <Control/Node.h>

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




	void							setTitle(std::string name);
	const std::string&				getTitle() const;

	void							setSize(Zuazo::Math::Vec2i size);
	Zuazo::Math::Vec2i				getSize() const;

	void							setPosition(Zuazo::Math::Vec2i pos);
	Zuazo::Math::Vec2i				getPosition() const;

	void							setOpacity(float opa);
	float							getOpacity() const;

	void							setResizeable(bool resizeable);
	bool							getResizeable() const;

	void							setDecorated(bool deco);
	bool							getDecorated() const;

	void							setMonitor(Monitor mon, const Monitor::Mode* mode);
	Monitor							getMonitor() const;

	static Monitor					getPrimaryMonitor();
	static Zuazo::Utils::BufferView<const Monitor>	getMonitors();

	static void						registerCommands(Control::Node& node);
};


}