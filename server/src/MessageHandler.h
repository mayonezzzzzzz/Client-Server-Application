#pragma once

#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio.hpp>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

class MessageHandler : public std::enable_shared_from_this<MessageHandler> {
public:
	MessageHandler(std::shared_ptr<asio::io_context> ioc);
	void startHandle();
	~MessageHandler() {
		std::cout << "MessageHandler object was deleted\n";
	};
private:
	std::shared_ptr<asio::io_context> ioc;
	tcp::acceptor acceptor;
	std::shared_ptr<http::request<http::vector_body<char>>> request;
};