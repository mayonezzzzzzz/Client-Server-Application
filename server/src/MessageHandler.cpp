#include "MessageHandler.h"
#include "Session.h"
#include <iostream>

MessageHandler::MessageHandler(std::shared_ptr<asio::io_context> ioc) :
    ioc(ioc), acceptor(*ioc, tcp::endpoint(asio::ip::make_address_v4("127.0.0.1"), 8080)),
    request(std::make_shared<http::request<http::vector_body<char>>>()) {
    acceptor.set_option(asio::socket_base::reuse_address(true));
    acceptor.listen();
    std::cout << "HTTP server started. Port - 8080\n";
}

void MessageHandler::startHandle() {
    try {
        auto self = shared_from_this();
        std::cout << "Starting handling\n";

        acceptor.async_accept(asio::make_strand(*ioc), [self](boost::system::error_code err, tcp::socket socket) {
            try {
                std::cout << "New connection accepted\n";
                auto session = std::make_shared<Session>(std::move(socket));
                session->Start();
            }
            catch (boost::system::error_code& err) {
                std::cerr << "Connection accepting error: " << err.what() << "\n";
            }
            self->startHandle();
            });
    }
    catch (boost::system::error_code& err) {
        std::cerr << "Error in async_accept function: " << err.what() << "\n";
    }
}

/*static void send_all_parts(std::shared_ptr<tcp::socket> socket, const std::string& image_id, const std::vector<char>& image_data) {
    size_t total_size = image_data.size();
    size_t offset = 0;

    auto send_next_part = std::make_shared<std::function<void()>>(); // Используется для продолжения отправки
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
}

void start_session(std::shared_ptr<tcp::socket> socket) {
    auto buffer = std::make_shared<beast::flat_buffer>();
    auto request = std::make_shared<http::request<http::vector_body<char>>>();

    http::async_read(*socket, *buffer, *request, [socket, buffer, request](boost::system::error_code ec, size_t) {
        if (ec) {
            std::cerr << "Error reading request: " << ec.message() << "\n";
            return;
        }

        if (request->method() == http::verb::post) {
            std::string image_id(request->at("Image-ID").data(), request->at("Image-ID").size());
            bool is_last_part = (request->at("Last-Part") == "1");

            std::vector<char>& image_data = image_parts[image_id];
            image_data.insert(image_data.end(), request->body().begin(), request->body().end());

            if (is_last_part) {
                // Декомпрессия
                std::vector<unsigned char> decompressed_image_data;
                int width, height, quality;

                std::cout << "Received image size: " << image_data.size() << std::endl;
                if (decompress(image_data, decompressed_image_data, width, height, quality)) {
                    // Сжатие
                    std::cout << "Decompressed size: " << decompressed_image_data.size() << std::endl;
                    std::vector<char> compressed_image_data;
                    if (compress(decompressed_image_data, compressed_image_data, width, height, quality)) {
                        std::cout << "Compressed size: " << compressed_image_data.size() << std::endl;
                        send_all_parts(socket, image_id, compressed_image_data);
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
        else {
            http::response<http::string_body> response{ http::status::method_not_allowed, request->version() };
            response.body() = "Only POST method is allowed.";
            response.prepare_payload();

            http::async_write(*socket, response, [socket](boost::system::error_code ec, size_t) {
                if (ec) {
                    std::cerr << "Error sending error response: " << ec.message() << "\n";
                }
                });
        }
        });
}*/