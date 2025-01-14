#include "Session.h"
#include "Compression.h"

Session::Session(tcp::socket&& socket) : socket(std::move(socket)), buffer(), request() {};

void Session::Start() {
    std::cout << "New session started\n";
    asio::post(socket.get_executor(), beast::bind_front_handler(&Session::Read, shared_from_this()));
}

/*void Session::sendAllParts(std::shared_ptr<tcp::socket> socket, const std::string& image_id, const std::vector<char>& image_data) {
    size_t total_size = image_data.size();
    size_t offset = 0;

    auto send_next_part = std::make_shared<std::function<void()>>(); // для продолжения отправки
    *send_next_part = [socket, image_id, image_data, total_size, offset, send_next_part]() mutable {
        if (offset < total_size) {
            size_t part_size = std::min(BUFFER_SIZE, total_size - offset);
            bool is_last_part = (offset + part_size == total_size);

            http::response<http::vector_body<char>> response{ http::status::ok, 11 };
            response.set(http::field::content_type, "image/jpeg");
            response.set("Image-ID", image_id);
            response.set("Last-Part", is_last_part ? "1" : "0");
            response.body().assign(image_data.begin() + offset, image_data.begin() + offset + part_size);
            response.prepare_payload();

            http::async_write(*socket, response, [socket, offset, part_size, send_next_part](boost::system::error_code ec, size_t) mutable {
                if (!ec) {
                    offset += part_size; // Продвигаемся на следующую часть
                    (*send_next_part)(); // Продолжаем отправку
                }
                else {
                    std::cerr << "Error sending part: " << ec.message() << "\n";
                }
                });
        }
        };

    (*send_next_part)(); // Запуск отправки первой части
}*/

void Session::Read() {
    std::cout << "Start reading requests\n";
    http::async_read(socket, buffer, request, beast::bind_front_handler(&Session::handleRead, shared_from_this()));
}

void Session::handleRead(boost::system::error_code& err, std::size_t) {
    if (!err) {
        try {
            std::cout << "Inside async_read\n";
            if (request.method() == http::verb::post) {
                std::string image_id(request.at("Image-ID").data(), request.at("Image-ID").size());
                bool is_last_part = (request.at("Last-Part") == "1");

                std::vector<char>& image_data = image_parts[image_id];
                image_data.insert(image_data.end(), request.body().begin(), request.body().end());

                if (is_last_part) {
                    // Декомпрессия 
                    std::vector<unsigned char> decompressed_image_data;
                    int width, height, quality = 75;

                    std::cout << "Received image size: " << image_data.size() << std::endl;
                    if (decompress(image_data, decompressed_image_data, width, height, quality)) {
                        // Сжатие 
                        std::cout << "Decompressed size:" << decompressed_image_data.size() << std::endl;
                        std::vector<char> compressed_image_data;
                        if (compress(decompressed_image_data, compressed_image_data, width, height, quality)) {
                            std::cout << "Compressedsize:" << compressed_image_data.size() << std::endl;

                            auto response = std::make_shared<http::response<http::vector_body<char>>>(http::status::ok, 11);
                            response->set(http::field::content_type, "image/jpeg");
                            response->set("Image-ID", image_id);
                            response->set("Last-Part", is_last_part ? "1" : "0");
                            response->body() = compressed_image_data;
                            response->prepare_payload();

                            http::async_write(socket, *response, beast::bind_front_handler(&Session::handleWrite, shared_from_this()));
                            return;

                            //sendAllParts(std::make_shared<tcp::socket>(std::move(socket)), image_id, compressed_image_data);

                            /*size_t total_size = image_data.size();
                            size_t offset = 0;

                            // Лямбда для отправки частей данных
                            auto send_next_part = std::make_shared<std::function<void(size_t)>>(); // Оборачиваем в shared_ptr
                            *send_next_part = [total_size, image_id, image_data, send_next_part](size_t current_offset) mutable {
                                if (current_offset < total_size) {
                                    size_t part_size = std::min(BUFFER_SIZE, total_size - current_offset);
                                    bool is_last_part = (current_offset + part_size == total_size);

                                    http::response<http::vector_body<char>> response{ http::status::ok, 11 };
                                    response.set(http::field::content_type, "image/jpeg");
                                    response.set("Image-ID", image_id);
                                    response.set("Last-Part", is_last_part ? "1" : "0");
                                    response.body().assign(image_data.begin() + current_offset, image_data.begin() + current_offset + part_size);
                                    response.prepare_payload();

                                    http::async_write(socket, response, beast::bind_front_handler(&Session::handleWrite(), shared_from_this()));
                                }
                                };

                            // запуск
                            (*send_next_part)(offset);*/

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
            }
        }
        catch (boost::system::error_code& err) {
            std::cerr << "Error in async_read function: " << err.what() << "\n";
        }
    }
    else {
        std::cout << "Outside async_read\n";
    }
}

void Session::handleWrite(boost::system::error_code& err, std::size_t) {
    if (err) {
        std::cerr << "Error in async_write function: " << err.what() << "\n";
    }
}