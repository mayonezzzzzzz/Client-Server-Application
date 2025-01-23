#include "ImageHandler.h"
#include "Logger.h"
#include <boost/asio.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

int main() {
    std::filesystem::path images_path = std::filesystem::path(__FILE__).parent_path().parent_path() / "images";
    std::filesystem::path responses_path = std::filesystem::path(__FILE__).parent_path().parent_path() / "responses";

    if (!std::filesystem::exists(images_path)) {
        std::filesystem::create_directory(images_path);
    }

    if (!std::filesystem::exists(responses_path)) {
        std::filesystem::create_directory(responses_path);
    }

    while (true) {
        try {
            asio::io_context ioc;
            tcp::resolver resolver(ioc);
            tcp::socket socket(ioc);

            std::string const address = "127.0.0.1";
            std::string const port = "8080";

            auto endpoints = resolver.resolve(address, port);
            asio::connect(socket, endpoints);

            std::cout << "Available images:\n";
            for (const auto& image : std::filesystem::directory_iterator(images_path)) {
                std::cout << image.path().filename() << "\n";
            }

            std::cout << "Enter the name of the image to send: ";
            std::string choice;
            std::getline(std::cin, choice);

            std::filesystem::path image_path = images_path / (choice + ".jpeg");
            if (!std::filesystem::exists(image_path)) {
                std::cout << "Image not found: " + image_path.string();
                logError("Image not found: " + image_path.string());
                continue;
            }

            std::ifstream image_file(image_path, std::ios::binary);
            image_file.seekg(0, std::ios::end);
            size_t image_size = image_file.tellg();
            image_file.seekg(0, std::ios::beg);

            // Отправка сообщения
            std::vector<char> buffer(std::istreambuf_iterator<char>(image_file), {});
            size_t offset = 0;
            while (offset < image_size) {
                size_t part_size = std::min(BUFFER_SIZE, image_size - offset); // Изображение отправляется по частям не более 1Мб
                sendPart(resolver, socket, address, { buffer.begin() + offset, buffer.begin() + offset + part_size }, choice, offset + part_size == image_size);
                offset += part_size;
            }

            // Получение ответа от сервера
            receiveParts(socket, choice, responses_path);
            logInfo("Successfully processed image: " + choice);
        }
        catch (boost::beast::system_error& err) {
            std::cout << "Error: " << err.what() << std::endl << std::endl;
            logError(err.what());
        }
    }
    return 0;
}