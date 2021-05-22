#include <Control/Message.h>

#include <algorithm>
#include <cassert>

namespace Cenital::Control {


Message::Message(	Type type,
					std::vector<std::string> payload )
	: m_payload(std::move(payload))
	, m_type(type)
{
}



void Message::setType(Type type) noexcept {
	m_type = type;
}

Message::Type Message::getType() const noexcept {
	return m_type;
}



std::vector<std::string>& Message::getPayload() noexcept {
	return m_payload;
}

const std::vector<std::string>& Message::getPayload() const noexcept {
	return m_payload;
}
	
}