#pragma once

#include <zuazo/Utils/BufferView.h>

#include <functional>
#include <string>

namespace Cenital::Control {

class Controller;
class Message;

class ViewBase {
public:
	explicit ViewBase(Controller& controller);
	ViewBase(const ViewBase& other) = default;
	ViewBase(ViewBase&& other) = default;
	virtual ~ViewBase() = default;

	ViewBase&							operator=(const ViewBase& other) = default;
	ViewBase&							operator=(ViewBase&& other) = default;

	void								setController(Controller& controller) noexcept;
	Controller& 						getController() const noexcept;

	void								action(	const Message& request,
												Message& response );
	virtual void						update(const Message& msg) = 0;

private:
	std::reference_wrapper<Controller>	m_controller;

};

}