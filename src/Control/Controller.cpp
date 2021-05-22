#include <Control/Controller.h>

#include <Control/ViewBase.h>
#include <Control/Message.h>

#include <mutex>

namespace Cenital::Control {

using namespace Zuazo;

Controller::Controller(Zuazo::ZuazoBase& base)
	: m_root()
	, m_views()
	, m_baseObject(base)
{
}

Node& Controller::getRootNode() noexcept {
	return m_root;
}

const Node& Controller::getRootNode() const noexcept {
	return m_root;
}


void Controller::process(	const Message& request,
							Message& response ) 
{
	std::lock_guard<Instance> lock(m_baseObject.get().getInstance());
	m_root(m_baseObject, request, 0, response);

	if(response.getType() == Message::Type::BROADCAST) {
		//Send it to all the listeners
		broadcast(response);

		//Make it as a plain success response for the sender
		response.setType(Message::Type::RESPONSE);
		response.getPayload().clear();
	}
}

void Controller::addView(ViewBase& view) {
	m_views.emplace_back(view);
}

void Controller::removeView(const ViewBase& view) {
	const auto ite = std::find_if(
		m_views.cbegin(), m_views.cend(),
		[&view] (const ViewBase& v) -> bool {
			return &view == &v;
		}
	);

	if(ite != m_views.cend()) {
		m_views.erase(ite);
	}
}



void Controller::broadcast(const Message& msg) {
	assert(msg.getType() == Message::Type::BROADCAST);

	std::for_each(
		m_views.cbegin(), m_views.cend(),
		[&msg] (ViewBase& view) -> void {
			view.update(msg);
		}
	);
}

}