#pragma once

#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio.hpp>

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;

const size_t BUFFER_SIZE = 1024 * 1024;

static std::unordered_map<std::string, std::vector<char>> image_parts;

class Session : public std::enable_shared_from_this<Session> {
public:
	tcp::socket socket;
	beast::flat_buffer buffer;
	http::request<http::vector_body<char>> request;
	Session(tcp::socket&& socket);
	void Start();
	//void sendAllParts(std::shared_ptr<tcp::socket> socket, const std::string& image_id, const std::vector<char>& image_data);
	~Session() {
		std::cout << "Session object was deleted\n";
	}
private:
	void Read();
	void handleRead(boost::system::error_code& err, std::size_t);
	void handleWrite(boost::system::error_code& err, std::size_t);
};