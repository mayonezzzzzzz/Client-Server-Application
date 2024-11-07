#include <iostream>
#include <string>
#include <thread>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = boost::beast::http;
using tcp = asio::ip::tcp;

void session(tcp::socket socket) {
	try {
		beast::flat_buffer buffer;

		http::request<http::string_body> request;

		http::read(socket, buffer, request);

		if (request.method() == http::verb::post) {
			std::string input = request.body();
			std::string output = input.append(input);

			http::response<http::string_body> response = { http::status::ok, request.version() };
			response.body() = output;

			response.prepare_payload();
			http::write(socket, response);
		}
		else {
			http::response<http::string_body> response = { http::status::method_not_allowed, request.version() };
			response.body() = "Expexted POST method.";

			response.prepare_payload();
			http::write(socket, response);
		}
		socket.shutdown(tcp::socket::shutdown_send);
	}
	catch (boost::beast::system_error& err) {
		std::cout << "Error: " << err.what() << std::endl;
	}
}

int main() {
	// контекст Input-Output
	asio::io_context iocont;

	// порт
	unsigned short const port = 8080;

	// ip-адрес lockalhost-а
	auto const address = asio::ip::make_address_v4("127.0.0.1");

	// acceptor для принятия tcp соединений
	tcp::acceptor acceptor(iocont, tcp::endpoint(address, port));
	std::cout << "HTTP server started. Port - " << port << std::endl;

	while (true)
	{
		tcp::socket socket(iocont);
		acceptor.accept(socket);
		std::thread(session, std::move(socket)).detach();
	}

	return 0;
}