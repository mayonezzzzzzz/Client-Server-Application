#pragma once

#include <boost/beast.hpp>
#include <boost/asio.hpp>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

// Сессия - для обработки сообщения текущего соединения
class Session : public std::enable_shared_from_this<Session> {
public:
	Session(tcp::socket&& socket);
	void Start();
	~Session() {
		std::cout << "Session object was deleted\n";
	}
private:
	tcp::socket socket;
	beast::flat_buffer buffer;
	http::request<http::vector_body<char>> request;
private:
	void Read();
	void sendNextPart(size_t offset, size_t total_size, std::string image_id, std::vector<char>& image_data);
	void handleRead(boost::system::error_code& err, std::size_t);
	void handleWrite(boost::system::error_code& err, std::size_t);
};