#include <Control/WebSocketServer.h>

namespace Cenital::Control {

WebSocketServer::WebSocketServer(	boost::asio::io_service& ios,
									uint16_t port,
									ConnectionOpenCallback openCbk,
									ConnectionCloseCallback closeCbk,
									MessageCallback msgCbk )
	: m_socket()
{	
	//Initialize Asio
	m_socket.init_asio(&ios);

	//Configure the callbacks
	m_socket.set_open_handler(std::move(openCbk));
	m_socket.set_close_handler(std::move(closeCbk));
	m_socket.set_message_handler(std::move(msgCbk));

	//Configure the port
	m_socket.listen(port);			
}



void WebSocketServer::setConnectionOpenCallback(ConnectionOpenCallback cbk) {
	m_socket.set_open_handler(std::move(cbk));
}

void WebSocketServer::setConnectionCloseCallback(ConnectionCloseCallback cbk) {
	m_socket.set_close_handler(std::move(cbk));
}

void WebSocketServer::setMessageCallback(MessageCallback cbk) {
	m_socket.set_message_handler(std::move(cbk));
}


void WebSocketServer::startAccept() {
	m_socket.start_accept();
}

void WebSocketServer::send(SessionPtr connection, const std::string& msg) {
	m_socket.send(
		std::move(connection), 
		msg, 
		websocketpp::frame::opcode::TEXT
	);
}
	
}