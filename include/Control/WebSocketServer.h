#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace Cenital::Control {

class WebSocketServer {
public:
	using SessionPtr = websocketpp::connection_hdl;
	using Socket = websocketpp::server<websocketpp::config::asio>;
	using Message = Socket::message_ptr;
	using MessageCallback = Socket::message_handler;
	using ConnectionOpenCallback = websocketpp::open_handler;
	using ConnectionCloseCallback = websocketpp::close_handler;

	WebSocketServer(boost::asio::io_service& ios,
					uint16_t port,
					ConnectionOpenCallback openCbk = {},
					ConnectionCloseCallback closeCbk = {},
					MessageCallback msgCbk = {} );
	WebSocketServer(const WebSocketServer& other) = delete;
	WebSocketServer(WebSocketServer&& other) = default;
	~WebSocketServer() = default;

	WebSocketServer&			operator=(const WebSocketServer& other) = delete;
	WebSocketServer&			operator=(WebSocketServer&& other) = default;

	void						setConnectionOpenCallback(ConnectionOpenCallback cbk);
	void						setConnectionCloseCallback(ConnectionCloseCallback cbk);
	void						setMessageCallback(MessageCallback cbk);

	void						startAccept();
	void						send(SessionPtr connection, const std::string& msg);

private:
	Socket						m_socket;

};

}