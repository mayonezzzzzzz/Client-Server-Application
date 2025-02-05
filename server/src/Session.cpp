#include <iostream>
#include "Session.h"
#include "Compression.h"
#include "ImageProcessor.h"
#include <atomic>

// Максимальный размер тела сообщения - 1Мб
const size_t BUFFER_SIZE = 1024 * 1024;

// Текущее количество обрабатываемых запросов
std::atomic<size_t> active_requests;
std::atomic<size_t> MAX_REQUESTS{ 0 };

// Для хранения изображений, которые обрабатываются сервером (Пара: ID изображения - изображение)
static std::unordered_map<std::string, std::vector<unsigned char>> image_parts;
// Для хранения текста для наложения на изображение
static std::unordered_map<std::string, std::string> text_parts;

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
void Session::sendNextPart(size_t offset, size_t total_size, const std::string& image_id, const std::vector<unsigned char>& image_data) {
    if (offset < total_size) {
        size_t part_size = std::min(BUFFER_SIZE, total_size - offset);
        bool is_last_part = (offset + part_size == total_size);

        auto response = std::make_shared<http::response<http::vector_body<unsigned char>>>(http::status::ok, 11);
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

        // Если количество обрабатываемых запросов достигло максимального значения
        if (active_requests.load() >= MAX_REQUESTS.load()) {
            std::cerr << "Server is busy. Rejecting request.\n";

            auto response = std::make_shared<http::response<http::string_body>>(http::status::service_unavailable, 11);
            response->set(http::field::content_type, "text/plain");
            response->body() = "Server is busy. Try again later.";
            response->prepare_payload();

            http::async_write(socket, *response,
                beast::bind_front_handler(&Session::handleWrite, shared_from_this())
            );
            return;
        }

        ++active_requests;
        std::cout << "Amount of requests: " << active_requests.load() << std::endl;

        if (request.method() == http::verb::post) {

            const auto& request_image_id = request.at("Image-ID");
            std::string image_id(request_image_id.data(), request_image_id.size());
            bool is_last_part = (request.at("Last-Part") == "1");

            auto& request_body = request.body();
            std::vector<char> body_data(request_body.begin(), request_body.end());
            std::cout << "Received part size: " << request_body.size() << std::endl;
            request_body.clear();

            if (image_parts.find(image_id) == image_parts.end()) { // если это первая часть изображения

                std::string body_str(body_data.begin(), body_data.end());
                size_t separator_pos = body_str.find("\n\n");

                if (separator_pos != std::string::npos) {

                    std::string text = body_str.substr(0, separator_pos); // текст для наложения
                    std::vector<unsigned char> image_data(body_str.begin() + separator_pos + 2, body_str.end());

                    std::cout << "Extracted text: " << text << "\n";

                    text_parts[image_id] = std::move(text);
                    image_parts[image_id] = std::move(image_data);
                }
                else {
                    std::cerr << "Error: No separator found in the first part\n";
                    return;
                }
            }
            else { // иначе добавляются только данные изображения
                std::vector<unsigned char>& image_data = image_parts[image_id];
                image_data.insert(image_data.end(), body_data.begin(), body_data.end());
            }

            // Если в запросе помечено, что эта часть изображения - последняя
            if (is_last_part) {
                processImage(image_id);
                --active_requests;
            }
            else {
                --active_requests;
                // Чтение следующего запроса
                Read();
            }
        }
    }
    else {
        std::cerr << "Error in async_read function: " << err.what() << "\n";
    }
}

void Session::processImage(const std::string& image_id) {
    // Добравление изображения и текста в map для его хранения
    std::vector<unsigned char>& image_data = image_parts[image_id];
    const std::string& text = text_parts[image_id];

    // Декомпрессия 
    std::vector<unsigned char> decompressed_image_data;
    int width, height; // параметры для функций libjpeg

    std::cout << "Received image size: " << image_data.size() << std::endl;
    if (decompress(image_data, decompressed_image_data, width, height)) {
        // Сжатие 
        std::cout << "Decompressed size:" << decompressed_image_data.size() << std::endl;
        std::cout << "Width: " << width << ", Height: " << height << std::endl;

        addTextToImage(decompressed_image_data, width, height, text);

        std::vector<unsigned char> compressed_image_data;
        if (compress(decompressed_image_data, compressed_image_data, width, height)) {

            const size_t total_size = compressed_image_data.size();
            std::cout << "Compressed size: " << total_size << std::endl;
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
    text_parts.erase(image_id);
}

void Session::handleWrite(boost::system::error_code& err, std::size_t) {
    if (err) {
        std::cerr << "Error in async_write function: " << err.what() << "\n";
    }
}