#include <Consumers/Window.h>

using namespace Zuazo;

namespace Cenital::Consumers {

Window::Window(	Instance& instance,
				std::string name,
				Math::Vec2i size )
	: RendererWrapper<Renderers::Window>(instance, std::move(name), size)
{
}

void Window::setTitle(std::string name) {
	getRenderer().setTitle(std::move(name));
}

const std::string& Window::getTitle() const {
	return getRenderer().getTitle();
}


void Window::setSize(Math::Vec2i size) {
	getRenderer().setSize(size);
}

Math::Vec2i Window::getSize() const {
	return getRenderer().getSize();
}


void Window::setPosition(Math::Vec2i pos) {
	getRenderer().setPosition(pos);
}

Math::Vec2i Window::getPosition() const {
	return getRenderer().getPosition();
}


void Window::setOpacity(float opa) {
	getRenderer().setOpacity(opa);
}

float Window::getOpacity() const {
	return getRenderer().getOpacity();
}


void Window::setResizeable(bool resizeable) {
	getRenderer().setResizeable(resizeable);
}

bool Window::getResizeable() const {
	return getRenderer().getResizeable();
}


void Window::setDecorated(bool deco) {
	getRenderer().setDecorated(deco);
}

bool Window::getDecorated() const {
	return getRenderer().getDecorated();
}


void Window::setMonitor(Monitor mon, const Monitor::Mode* mode) {
	getRenderer().setMonitor(mon, mode);
}

Window::Monitor Window::getMonitor() const {
	return getRenderer().getMonitor();
}



Window::Monitor Window::getPrimaryMonitor() {
	return Renderers::Window::getPrimaryMonitor();
}

Utils::BufferView<const Window::Monitor> Window::getMonitors() {
	return Renderers::Window::getMonitors();
}


}