#pragma once

#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio.hpp>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

// Класс для обработки соединений
class MessageHandler : public std::enable_shared_from_this<MessageHandler> {
public:
	MessageHandler(const std::shared_ptr<asio::io_context>& ioc, const std::string& port);
	static std::shared_ptr<MessageHandler> createMessageHandler(const std::shared_ptr<asio::io_context>& ioc, const std::string& port);
	void startHandle();
	~MessageHandler() {
		std::cout << "MessageHandler object was deleted\n";
	};
private:
	std::shared_ptr<asio::io_context> ioc;
	tcp::acceptor acceptor;
};