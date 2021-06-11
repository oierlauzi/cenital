#include <Control/Node.h>

#include <Control/Controller.h>
#include <Control/Message.h>

namespace Cenital::Control {

using namespace Zuazo;

Node::Node(std::initializer_list<PathMap::value_type> ilist)
	: m_paths(ilist)
{
}

bool Node::addPath(std::string token, Callback path) {
	bool result;
	std::tie(std::ignore, result) = m_paths.emplace(std::move(token), std::move(path));
	return result;
}

bool Node::removePath(const std::string& token) {
	return m_paths.erase(token);
}

Node::Callback* Node::getPath(const std::string& token) {
	const auto ite = m_paths.find(token);
	return (ite != m_paths.cend()) ? &(ite->second) : nullptr;
}

const Node::Callback* Node::getPath(const std::string& token) const {
	const auto ite = m_paths.find(token);
	return (ite != m_paths.cend()) ? &(ite->second) : nullptr;
}

void Node::operator()(	Controller& controller,
						ZuazoBase& base, 
						const Message& request,
						size_t level,
						Message& response  ) const 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() > level) {
		//There are pending tokens
		const auto& token = tokens[level];

		//Call the corresponding function poping the 
		//first element as it is the one we've used.
		const Callback* path;
		if(token == "help") {
			help(controller,base, request, level + 1, response);
		} else if(token == "name") {
			name(controller,base, request, level + 1, response);
		} else if(token == "type") {
			type(controller,base, request, level + 1, response);
		} else if(token == "ping") {
			ping(controller,base, request, level + 1, response);
		} else if((path = getPath(token))) {
			Utils::invokeIf(*path, controller, base, request, level + 1, response);
		}
	}
}


void Node::help(Controller&,
				ZuazoBase&, 
				const Message& request,
				size_t level,
				Message& response ) const
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level) {
		//Elaborate the response
		response.setType(Message::Type::response);
		auto& payload = response.getPayload();
		payload = { "help", "name", "type", "ping" }; //Ensured paths
		payload.reserve(m_paths.size() + 2);
		std::transform(
			m_paths.cbegin(), m_paths.cend(),
			std::back_inserter(payload),
			[] (const std::pair<const std::string, Callback>& entry) -> const std::string& {
				return entry.first;
			}
		);
	}
}

void Node::name(Controller&,
				ZuazoBase& base, 
				const Message& request,
				size_t level,
				Message& response )
{

	const auto& tokens = request.getPayload();
	if(tokens.size() == level) {
		response.setType(Message::Type::response);
		response.getPayload() = { base.getName() };
	}
}


void Node::type(Controller& controller,
				ZuazoBase& base, 
				const Message& request,
				size_t level,
				Message& response )
{

	const auto& tokens = request.getPayload();
	if(tokens.size() == level) {
		//Determine the identity of the base class
		const auto& classIndex = controller.getClassIndex();
		const auto* classEntry = classIndex.find(typeid(base));

		if(classEntry) {
			//Elaborate the response
			response.setType(Message::Type::response);
			response.getPayload() = { classEntry->getName() };
		}
	}
}

void Node::ping(Controller&,
				ZuazoBase&, 
				const Message& request,
				size_t level,
				Message& response   ) 
{
	const auto& tokens = request.getPayload();
	if(tokens.size() == level) {
		response.setType(Message::Type::response);
		response.getPayload() = { "pong" };
	}
}

}