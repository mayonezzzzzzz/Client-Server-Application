#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <ctime>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/beast/websocket.hpp>

#pragma warning(disable : 4996)

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = boost::beast::http;
using tcp = asio::ip::tcp;

// запись ответов в файл base.txt и в отдельные файлы
void push_response_into_files(std::filesystem::path path, http::response<http::string_body> response) {

    // в base.txt записывается тело каждого ответа
    std::ofstream all_requests(path / "base.txt", std::ios_base::app);
    if (all_requests.is_open()) {
        all_requests << response.body() << std::endl;
        all_requests.close();
    }

    time_t now = time(0);
    std::tm local_time = *std::localtime(&now);
    std::ostringstream new_file_path;
    // текущая дата и время
    new_file_path << std::put_time(&local_time, "%d.%m.%Y %H-%M-%S") << ".txt";

    // запись ответа в файл с текущей датой и временем в названии
    std::ofstream current_request(path / new_file_path.str(), std::ios_base::out);
    if (current_request.is_open()) {
        current_request << response << std::endl;
        current_request.close();
    }
}

int main()
{
    // выход из папки src, путь к папке responses в client
    std::filesystem::path path_to_requests = std::filesystem::path(__FILE__).parent_path().parent_path() / "responses";

    if (!std::filesystem::exists(path_to_requests)) {
        std::filesystem::create_directory(path_to_requests);
    }

    while (true) {
        try {
            // контекст Input-Output
            asio::io_context iocont;

            tcp::resolver resolver(iocont);

            beast::tcp_stream stream(iocont);

            // порт
            std::string const port = "8080";

            // ip-адрес lockalhost-а
            std::string const address = "127.0.0.1";

            // список из endpoints (конечных точек)
            auto const result = resolver.resolve(address, port);

            // установка tcp соединения
            stream.connect(result);

            std::cout << "\nEnter a string to send: ";
            std::string data = "";
            std::getline(std::cin, data);

            // запрос с текстовыми данными (метод - post)
            http::request<http::string_body> request{ http::verb::post, "/", 11 };
            request.set(http::field::host, address);
            request.set(http::field::content_type, "text/plain");
            request.body() = data;
            request.prepare_payload();

            // отправка запроса через установленное tcp соединение
            http::write(stream, request);

            // буфер для хранения
            beast::flat_buffer buffer;

            // ответ сервера
            http::response<http::string_body> response;

            // чтение ответа сервера
            http::read(stream, buffer, response);

            std::cout << response << std::endl;

            // запись ответа в файлы
            push_response_into_files(path_to_requests, response);

            beast::error_code errc;
            stream.socket().shutdown(tcp::socket::shutdown_both, errc);
            if (errc && errc != beast::errc::not_connected)
                throw beast::system_error{ errc };
        }
        catch (boost::beast::system_error& err) {
            std::cout << "Error: " << err.what() << std::endl;
        }
    }

    return 0;
}