#include <Control/TCPServer.h>

#include <zuazo/Utils/Functions.h>

#include <memory>
#include <queue>

namespace Cenital::Control {

class TCPServer::Session 
	: public std::enable_shared_from_this<TCPServer::Session>
{
public:
	Session(boost::asio::ip::tcp::socket socket,
			ConnectionCloseCallback closeCbk,
			MessageCallback msgCbk )
		: m_socket(std::move(socket))
		, m_closeCallback(std::move(closeCbk))
		, m_messageCallback(std::move(msgCbk))
	{
	}

	void startListening() {
		asyncRead();
	}

	void send(std::string message) {
		const bool idle = m_outgoing.empty();
		m_outgoing.push(std::move(message));

		if(idle) {
			asyncWrite();
		}
	}

private:
	void asyncRead() {
		boost::asio::async_read_until(
			m_socket, m_streambuf, '\n',
			std::bind(&Session::onRead, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
		);
	}

	void onRead(boost::system::error_code error, size_t byteCnt) {
		if(!error) {
			if(m_messageCallback) {
				//Convert the data to string
				const auto buffer = m_streambuf.data();
				std::string str(
					boost::asio::buffers_begin(buffer),
					boost::asio::buffers_begin(buffer) + byteCnt
				);

				//Invoke the corresponding callback
				m_messageCallback(weak_from_this(), std::move(str));
			}
			
			//Pop the consumed bytes
			m_streambuf.consume(byteCnt);

			//Read the next message
			asyncRead();
		} else {
			//Error happened while receiving
			if(m_closeCallback) {
				m_closeCallback(weak_from_this());
			}

			m_socket.close(error);
		}
	}

	void asyncWrite() {
		boost::asio::async_write(
			m_socket, 
			boost::asio::buffer(m_outgoing.front()), 
			std::bind(&Session::onWrite, shared_from_this(), std::placeholders::_1, std::placeholders::_2)
		);
	}

	void onWrite(boost::system::error_code error, size_t) {
		//Byte count not used
		if(!error) {
			//The first message on the queue was successfully sent
			m_outgoing.pop();

			//Send the next message
			if(!m_outgoing.empty()) {
				asyncWrite();
			}
		} else {
			//Error happened while transmitting
			if(m_closeCallback) {
				m_closeCallback(weak_from_this());
			}

			m_socket.close(error);
		}
	}

	boost::asio::ip::tcp::socket 		m_socket;
	boost::asio::streambuf 				m_streambuf;
	std::queue<std::string> 			m_outgoing;

	ConnectionCloseCallback 			m_closeCallback;
	MessageCallback 					m_messageCallback;
};


TCPServer::TCPServer(	boost::asio::io_service& ios,
						uint16_t port,
						ConnectionOpenCallback openCbk,
						ConnectionCloseCallback closeCbk,
						MessageCallback msgCbk )
	: m_ios(ios)
	, m_acceptor(m_ios, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
	, m_socket(m_ios)
	, m_openCallback(std::move(openCbk))
	, m_closeCallback(std::move(closeCbk))
	, m_messageCallback(std::move(msgCbk))
{
}



void TCPServer::setConnectionOpenCallback(ConnectionOpenCallback cbk) {
	m_openCallback = std::move(cbk);
}

void TCPServer::setConnectionCloseCallback(ConnectionCloseCallback cbk) {
	m_closeCallback = std::move(cbk);
}

void TCPServer::setMessageCallback(MessageCallback cbk) {
	m_messageCallback = std::move(cbk);
}


void TCPServer::startAccept() {
	asyncAccept();
}


void TCPServer::send(SessionPtr session, Message msg) {
	//Send only if the handler exists
	const auto s = session.lock();
	if(s) {
		s->send(std::move(msg));
	}
}



void TCPServer::asyncAccept() {
	m_acceptor.async_accept(
		m_socket, 
		std::bind(&TCPServer::onAccept, std::ref(*this), std::placeholders::_1)
	);
}

void TCPServer::onAccept(boost::system::error_code) {
	//Currently not using the error code
	//Create a new client from the accept
	auto client = Zuazo::Utils::makeShared<Session>(
		std::move(m_socket),
		m_closeCallback,
		m_messageCallback
	);

	//Call the corresponding callback
	if(m_openCallback) {
		m_openCallback(client);
	}

	//Start listening
	client->startListening();

	//Accept the next client
	m_socket = Socket(m_ios); //Rember that the socket was moved to the client
	asyncAccept();
}

}