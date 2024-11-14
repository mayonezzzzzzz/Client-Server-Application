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

// одна сессия - через сокет
void session(tcp::socket socket) {
	try {
		beast::flat_buffer buffer;

		// запрос с массивом байтов в теле
		http::request<http::vector_body<char>> request;

		// чтение запроса из сокета в буфер
		http::read(socket, buffer, request);

		// если метод - POST, создается ответ
		if (request.method() == http::verb::post) {
			std::vector<char> image_data = request.body();

			// ответ принимает в себя данные из запроса (пока что без изменений)
			http::response<http::vector_body<char>> response = { http::status::ok, request.version() };
			response.body() = std::move(image_data);
			response.set(http::field::content_type, "image/jpeg");
			response.prepare_payload();
			// отправка ответа клиенту через сокет
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