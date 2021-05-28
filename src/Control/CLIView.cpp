#include <Control/CLIView.h>

#include <Control/Message.h>

#include <zuazo/Utils/Functions.h>

namespace Cenital::Control {

using namespace Zuazo;

constexpr auto SEPARATOR = ' ';
constexpr auto ESCAPE = '\\';
static const std::string SPECIAL_CHARACTERS = { SEPARATOR, ESCAPE };

static bool isEscaped(std::string_view str, size_t pos) {
	bool result;

	if(pos > 0) {
		//Count the consecutive escape characters at the end
		size_t count = 0;
		while(count < pos && str[pos - count - 1] == ESCAPE) {
			++count;
		}

		//If there is a even amount of consecutive escape characters at
		//the end, it it not scaped, if the amount is odd, it is escaped
		result = (count % 2) != 0;
	} else {
		//It cannot be escaped if it is in the first position
		result = false;
	}

	return result;
}

static void addEscapeCharacters(std::string& str) {
	//Escape all the special characters
	for(size_t i = str.find_first_of(SPECIAL_CHARACTERS); i < str.size(); i = str.find_first_of(SPECIAL_CHARACTERS, i)) {
		str.insert(i, 1, ESCAPE);
		i += 2; //Jump the newly added and existing escaped characters
	}
}

static void removeEscapeCharacters(std::string& str) {
	for(size_t i = str.find(ESCAPE); i < str.size(); i = str.find(ESCAPE, i)) {
		str.erase(i, 1);
		++i; //Skip the escaped character
	}
}

static void tokenize(std::string_view msg, std::vector<std::string>& tokens) {
	tokens.clear();

	//Remove the carriage return at the end
	while(!msg.empty() && (msg.back() == '\n')) {
		msg.remove_suffix(1);
	}

	//Process the string
	size_t splitPos = 0;
	while(!msg.empty()) {
		//Find the first occurrence of the delimitator
		splitPos = msg.find(SEPARATOR, splitPos);

		if(splitPos == 0) {
			//Remove the spaces in the front
			msg.remove_prefix(1);
		} else if(splitPos == std::string_view::npos) {
			//No more separators, add the rest and stop
			tokens.emplace_back(msg);
			removeEscapeCharacters(tokens.back());

			msg = std::string_view();
		} else if(isEscaped(msg, splitPos)) {
			//Delimitator is scaped, start finding the next
			//one from it.
			++splitPos;
		} else {
			//We've found a separator. Extract a token that
			//ends on it (exclusively) and pop it from the
			//message (inclusively)
			tokens.emplace_back(msg.substr(0, splitPos));
			removeEscapeCharacters(tokens.back());

			msg.remove_prefix(splitPos+1); //+1 as we do not care about the separator.
			splitPos = 0; //In order to start finding at the beggining
		}
	}
}

static void serialize(const std::vector<std::string>& tokens, std::string& msg) {
	msg.clear();

	std::string token;

	for(size_t i = 0; i < tokens.size(); ++i) {
		token = tokens[i];

		//Sanitize the token
		addEscapeCharacters(token);

		//Add a separator if not the first one
		if(i != 0) {
			msg += SEPARATOR;
		} 

		//Add the token itself
		msg += token;
	}

	//Add a carriage return at the end
	msg += '\n';
}







CLIView::CLIView(Controller& controller) 
	: ViewBase(controller)
	, m_listeners()
	, m_request(Message::Type::request)
	, m_response()
	, m_update()
{
}

void CLIView::addListener(Listener ls) {
	m_listeners.push_back(std::move(ls));
}

void CLIView::parse(std::string_view msg, std::string& ret) {
	//Elaborate the request
	assert(m_request.getType() == Message::Type::request); //Should not be changed
	auto& reqTokens = m_request.getPayload();
	tokenize(msg, reqTokens);

	//Check if a acknowledgment id is provided
	std::string ack;
	if(!reqTokens.empty() && reqTokens.front().front() == '#') {
		ack = std::move(reqTokens.front());
		reqTokens.erase(reqTokens.cbegin()); //Pop front
	}

	//Clear the response
	m_response.setType(Message::Type::error);
	auto& resTokens = m_response.getPayload();
	resTokens.clear();

	//Make the request to the controller
	action(m_request, m_response);

	//Add success token at the front
	std::string success = 	m_response.getType() == Message::Type::response ?
							"OK" :
							"FAIL";
	resTokens.insert(resTokens.cbegin(), std::move(success));

	//Append the ack id at the front
	if(!ack.empty()) {
		resTokens.insert(resTokens.cbegin(), std::move(ack));
	}

	//Elaborate the response
	serialize(resTokens, ret);
}

void CLIView::update(const Message& msg) {
	assert(msg.getType() == Message::Type::broadcast);
	serialize(msg.getPayload(), m_update);

	//Transmit the update to all the listeners
	for(const auto& listener : m_listeners) {
		Utils::invokeIf(listener, m_update);
	}
	
}

}