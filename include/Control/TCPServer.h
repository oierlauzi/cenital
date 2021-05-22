#pragma once

#include <memory>
#include <functional>
#include <unordered_set>

#include <boost/asio.hpp>

namespace Cenital::Control {

class TCPServer {
public:
	class Session;
	using SessionPtr = std::weak_ptr<Session>;
	using Acceptor = boost::asio::ip::tcp::acceptor;
	using Socket = boost::asio::ip::tcp::socket;
	using Message = std::string;
	using MessageCallback = std::function<void(SessionPtr, Message)>;
	using ConnectionOpenCallback = std::function<void(SessionPtr)>;
	using ConnectionCloseCallback = std::function<void(SessionPtr)>;

	TCPServer(	boost::asio::io_service& ios,
				uint16_t port,
				ConnectionOpenCallback openCbk = {},
				ConnectionCloseCallback closeCbk = {},
				MessageCallback msgCbk = {} );
	TCPServer(const TCPServer& other) = delete;
	TCPServer(TCPServer&& other) = default;
	~TCPServer() = default;

	void						setConnectionOpenCallback(ConnectionOpenCallback cbk);
	void						setConnectionCloseCallback(ConnectionCloseCallback cbk);
	void						setMessageCallback(MessageCallback cbk);

	void						startAccept();
	void						send(SessionPtr session, Message msg);

private:
    boost::asio::io_service&	m_ios;
    Acceptor					m_acceptor;
	Socket						m_socket;

	ConnectionOpenCallback		m_openCallback;
	ConnectionCloseCallback		m_closeCallback;
	MessageCallback				m_messageCallback;

	void 						asyncAccept();
	void						onAccept(boost::system::error_code error);

};

}