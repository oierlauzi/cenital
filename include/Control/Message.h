#pragma once

#include <vector>
#include <string>

namespace Cenital::Control {

class Message {
public:
	enum class Type {
		ERROR,
		REQUEST,
		RESPONSE,
		BROADCAST,
	};

	explicit Message(	Type type = Type::ERROR,
						std::vector<std::string> payload = {} );
	Message(const Message& other) = delete;
	Message(Message&& other) = default;
	~Message() = default;

	Message&						operator=(const Message& other) = delete;
	Message&						operator=(Message&& other) = default;

	void							setType(Type type) noexcept;
	Type							getType() const noexcept;

	std::vector<std::string>&		getPayload() noexcept;
	const std::vector<std::string>&	getPayload() const noexcept;

private:
	std::vector<std::string>		m_payload;
	Type							m_type;

};

}