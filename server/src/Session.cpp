#include "Session.h"
#include "Compression.h"
#include <iostream>

// Максимальный размер тела сообщения - 1Мб
const size_t BUFFER_SIZE = 1024 * 1024;

// Для хранения изображений, которые обрабатываются сервером (Пара: ID изображения - изображение)
static std::unordered_map<std::string, std::vector<char>> image_parts;

Session::Session(tcp::socket&& socket) : socket(std::move(socket)), buffer(), request() {};

void Session::Start() {
    std::cout << "New session started\n";
    asio::post(socket.get_executor(), beast::bind_front_handler(&Session::Read, shared_from_this())); // автоматический захват shared_from_this() -> продление времени жизни объекта
}

void Session::Read() {
    std::cout << "Start reading requests\n";
    http::async_read(socket, buffer, request, beast::bind_front_handler(&Session::handleRead, shared_from_this()));
}

// Функция для отправки следующей части изображения клиенту
void Session::sendNextPart(size_t offset, size_t total_size, std::string image_id, std::vector<char>& image_data) {
    if (offset < total_size) {
        size_t part_size = std::min(BUFFER_SIZE, total_size - offset);
        bool is_last_part = (offset + part_size == total_size);

        auto response = std::make_shared<http::response<http::vector_body<char>>>(http::status::ok, 11);
        response->set(http::field::content_type, "image/jpeg");
        response->set("Image-ID", image_id);
        response->set("Last-Part", is_last_part ? "1" : "0");
        response->body().assign(image_data.begin() + offset, image_data.begin() + offset + part_size);
        response->prepare_payload();

        offset += part_size;

        http::async_write(
            socket, *response,
            beast::bind_front_handler(&Session::handleWrite, shared_from_this())
        );

        // Отправка следующей части
        sendNextPart(offset, total_size, image_id, image_data);
    }
}

void Session::handleRead(boost::system::error_code& err, std::size_t) {
    if (!err) {
        std::cout << "Inside async_read\n";
        if (request.method() == http::verb::post) {
            std::string image_id(request.at("Image-ID").data(), request.at("Image-ID").size());
            bool is_last_part = (request.at("Last-Part") == "1");

            std::vector<char>& image_data = image_parts[image_id];
            // Добравление изображения в map для его хранения
            image_data.insert(image_data.end(), request.body().begin(), request.body().end());
            std::cout << "Received part size: " << request.body().size() << std::endl;
            request.body().clear();

            // Если в запросе помечено, что эта часть изображения - последняя
            if (is_last_part) {
                // Декомпрессия 
                std::vector<unsigned char> decompressed_image_data;
                int width, height, quality = 75; // параметры для функций libjpeg

                std::cout << "Received image size: " << image_data.size() << std::endl;
                if (decompress(image_data, decompressed_image_data, width, height, quality)) {
                    // Сжатие 
                    std::cout << "Decompressed size:" << decompressed_image_data.size() << std::endl;
                    std::vector<char> compressed_image_data;
                    if (compress(decompressed_image_data, compressed_image_data, width, height, quality)) {
                        std::cout << "Compressed size:" << compressed_image_data.size() << std::endl;

                        size_t total_size = compressed_image_data.size();
                        size_t offset = 0;

                        // Начало отправки ответа клиенту
                        sendNextPart(offset, total_size, image_id, compressed_image_data);
                    }
                    else {
                        std::cerr << "Error compressing image\n";
                    }
                }
                else {
                    std::cerr << "Error decompressing image\n";
                }
                image_parts.erase(image_id);
            }
            else {
                // Чтение следующего запроса
                Read();
            }
        }
    }
    else {
        std::cerr << "Error in async_read function: " << err.what() << "\n";
    }
}

void Session::handleWrite(boost::system::error_code& err, std::size_t) {
    if (err) {
        std::cerr << "Error in async_write function: " << err.what() << "\n";
    }
}