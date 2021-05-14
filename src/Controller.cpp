#include <Controller.h>

#include <mutex>

namespace Cenital {

using namespace Zuazo;

/*
 * Controller::Result
 */
Controller::Result::Result(	Type type,
							std::vector<std::string> payload )
	: m_type(type)
	, m_payload(payload)
{
}

void Controller::Result::setType(Type type) noexcept {
	m_type = type;
}

Controller::Result::Type Controller::Result::getType() const noexcept {
	return m_type;
}


void Controller::Result::setPayload(std::vector<std::string> payload) {
	m_payload = std::move(payload);
}

const std::vector<std::string>& Controller::Result::getPayload() const noexcept {
	return m_payload;
}




/*
 * Controller::Node
 */

const std::string Controller::Node::PING_PATH = "ping";

Controller::Node::Node(std::initializer_list<std::pair<const std::string, Callback>> ilist)
	: m_paths(ilist)
{
	//By default, always add the ping
	addPath(PING_PATH, ping);
}

void Controller::Node::addPath(std::string token, Callback path) {
	m_paths.emplace(std::move(token), std::move(path));
}

void Controller::Node::removePath(const std::string& token) {
	m_paths.erase(token);
}

const Controller::Node::Callback* Controller::Node::getPath(const std::string& token) const {
	const auto ite = m_paths.find(token);
	return (ite != m_paths.cend()) ? &(ite->second) : nullptr;
}

Controller::Result Controller::Node::operator()(ZuazoBase& base, 
												TokenArray tokens ) const 
{
	Result result;

	if(!tokens.empty()) {
		//There are pending tokens
		const auto* path = getPath(tokens.front());
		if(path) {
			//We're able to process this token 
			//Call the corresponding callback poping the 
			//first element as it is the one we've used.
			if(*path) {
				result = (*path)(
					base,
					TokenArray(std::next(tokens.begin()), tokens.cend())
				);
			}
		}
	}

	return result;
}



Controller::Result Controller::Node::ping(	ZuazoBase& base, 
											TokenArray tokens ) 
{
	Result result;

	if(tokens.empty()) {
		result.setType(Result::Type::RESPONSE);
		result.setPayload({
			"pong",
			base.getName()
		});
	}

	return result;
}

/*
 * Controller
 */

Controller::Controller(Zuazo::ZuazoBase& base)
	: m_root()
	, m_views()
	, m_baseObject(base)
{
}

Controller::Node& Controller::getRootNode() noexcept {
	return m_root;
}

const Controller::Node& Controller::getRootNode() const noexcept {
	return m_root;
}


Controller::Result Controller::process(TokenArray tokens) {
	std::lock_guard<Instance> lock(m_baseObject.get().getInstance());
	const auto result = m_root(m_baseObject, tokens);

	if(result.getType() == Result::Type::SUCCESS) {
		//TODO Update all views
	}

	return result;
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

}