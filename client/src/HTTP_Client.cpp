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

// запись ответов в файлы с расширением .jpeg
void saveResponse(std::filesystem::path path, http::response<http::vector_body<char>> response) {

    time_t now = time(0);
    std::tm local_time = *std::localtime(&now);
    std::ostringstream new_file_path;
    // текущая дата и время
    new_file_path << std::put_time(&local_time, "%d.%m.%Y %H-%M-%S") << ".jpeg";

    // создение изображения с текущей датой и временем в названии
    std::ofstream image_file(path / new_file_path.str(), std::ios::binary);
    if (image_file.is_open()) {
        image_file.write(response.body().data(), response.body().size());
        image_file.close();
    }
}

int main()
{
    // выход из папки src, путь к папке responses в client
    std::filesystem::path path_to_requests = std::filesystem::path(__FILE__).parent_path().parent_path() / "responses";

    if (!std::filesystem::exists(path_to_requests)) {
        std::filesystem::create_directory(path_to_requests);
    }

    // путь к папке, на которой находятся изображения для отправки
    std::filesystem::path images_path = std::filesystem::path(__FILE__).parent_path().parent_path() / "images";

    if (!std::filesystem::exists(images_path)) {
        std::filesystem::create_directory(images_path);
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

            std::cout << "List of images" << std::endl
                << "--------------" << std::endl;
            for (const auto& image : std::filesystem::directory_iterator(images_path)) {
                std::cout << image.path().filename() << std::endl;
            }
            std::cout << "\nChoose the name of the image to send: " << std::endl;
            std::string choice;
            std::getline(std::cin, choice);

            // считывание в бинарном режиме содержимого изображения для передачи
            std::vector<char> image_data;
            for (const auto& image : std::filesystem::directory_iterator(images_path)) {
                if (image.path().filename() == choice) {
                    std::filesystem::path image_path = images_path / image.path().filename();
                    std::ifstream image_in_binary_view(image_path, std::ios::binary);

                    // получение размера изображения
                    image_in_binary_view.seekg(0, std::ios::end);
                    std::streamsize image_size = image_in_binary_view.tellg();
                    image_in_binary_view.seekg(0, std::ios::beg);

                    // считываение содержимого изображения в vector
                    image_data.resize(image_size, '\0');
                    image_in_binary_view.read(&image_data[0], image_size);
                }
            }

            // запрос с изображением, представленный потоком байтов (метод - post)
            http::request<http::vector_body<char>> request{ http::verb::post, "/", 11 };
            request.set(http::field::host, address);
            request.set(http::field::content_type, "image/jpeg");
            // используется std::move для предотвращения копирования
            request.body() = std::move(image_data);
            request.prepare_payload();

            // отправка запроса через установленное tcp соединение
            http::write(stream, request);

            // буфер для хранения
            beast::flat_buffer buffer;

            // ответ сервера
            http::response<http::vector_body<char>> response;

            // чтение ответа сервера
            http::read(stream, buffer, response);

            std::cout << response << std::endl;

            // сохранение ответа как изображения
            saveResponse(path_to_requests, response);

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