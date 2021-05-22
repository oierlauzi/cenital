#include <Control/ViewBase.h>

#include <Control/Controller.h>

using namespace Zuazo;

namespace Cenital::Control {

ViewBase::ViewBase(Controller& controller)
	: m_controller(controller)
{
}

void ViewBase::setController(Controller& controller) noexcept {
	m_controller = controller;
}

Controller& ViewBase::getController() const noexcept {
	return m_controller;
}


void ViewBase::action(	const Message& request,
						Message& response ) 
{
	getController().process(request, response);
}
	
}