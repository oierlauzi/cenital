#include <Control/Node.h>

#include <Control/Message.h>

using namespace Zuazo;

namespace Cenital::Control {

Node::Node(std::initializer_list<std::pair<const std::string, Callback>> ilist)
	: m_paths(ilist)
{
}

void Node::addPath(std::string token, Callback path) {
	m_paths.emplace(std::move(token), std::move(path));
}

void Node::removePath(const std::string& token) {
	m_paths.erase(token);
}

const Node::Callback* Node::getPath(const std::string& token) const {
	const auto ite = m_paths.find(token);
	return (ite != m_paths.cend()) ? &(ite->second) : nullptr;
}

void Node::operator()(	Zuazo::ZuazoBase& base, 
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
			help(base, request, level + 1, response);
		} else if(token == "ping") {
			ping(base, request, level + 1, response);
		} else if((path = getPath(token))) {
			if(*path) {
				(*path)(base, request, level + 1, response);
			}
		}
	}
}


void Node::help(Zuazo::ZuazoBase&, 
				const Message& request,
				size_t level,
				Message& response ) const
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == level) {
		response.setType(Message::Type::RESPONSE);

		//Elaborate the response
		auto& payload = response.getPayload();
		payload = { "help", "ping" }; //Ensured paths
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

void Node::ping(Zuazo::ZuazoBase& base, 
				const Message& request,
				size_t level,
				Message& response   ) 
{
	const auto& tokens = request.getPayload();

	if(tokens.size() == level) {
		response.setType(Message::Type::RESPONSE);
		response.getPayload() = {
			"pong",
			base.getName()
		};
	}
}

}