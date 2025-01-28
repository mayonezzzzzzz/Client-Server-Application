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

const size_t MAX_INPUT_LENGTH = 50;
const size_t BUFFER_SIZE = 1024 * 1024;

void detectDirectory(std::string& path_str) {
    std::ifstream path_file("path.txt", std::ios::in);
    if (path_file.is_open()) {
        std::getline(path_file, path_str);
        path_file.close();
    }
    else {
        std::ofstream new_path_file("path.txt", std::ios::trunc);
        if (new_path_file.is_open()) {
            new_path_file.close();
        }
    }

    if (!std::filesystem::exists(path_str) && !std::filesystem::is_directory(path_str)) {
        path_str = ".";
    }
}

int main() {
    std::string path_str;
    detectDirectory(path_str);
    std::filesystem::path path = path_str;

    std::filesystem::path images_path = path;
    std::filesystem::path responses_path = path / "responses";

    if (!std::filesystem::exists(images_path)) {
        std::filesystem::create_directory(images_path);
    }

    if (!std::filesystem::exists(responses_path)) {
        std::filesystem::create_directory(responses_path);
    }

    std::cout << "If you want to specify a directory to take images from, write the full path to it into the file \"path.txt\" in the root of the project\n\n";

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

            std::string overlay_text;
            std::cout << "Enter the text to overlay on the image: ";
            std::getline(std::cin, overlay_text);

            if (overlay_text.empty()) {
                overlay_text = "default text";
            }
            else if (overlay_text.length() > MAX_INPUT_LENGTH) {
                overlay_text = overlay_text.substr(0, MAX_INPUT_LENGTH);
            }

            std::ifstream image_file(image_path, std::ios::binary);
            image_file.seekg(0, std::ios::end);
            size_t image_size = image_file.tellg();
            image_file.seekg(0, std::ios::beg);

            // Отправка сообщения
            std::vector<char> buffer(std::istreambuf_iterator<char>(image_file), {});
            size_t offset = 0;
            bool is_first_part = true;
            while (offset < image_size) {\
                /*  Изображение отправляется по частям не более 1Мб
                    Учитывается место для текста при отправке первой части  */
                size_t part_size = std::min(BUFFER_SIZE - overlay_text.size(), image_size - offset); 
                std::vector<char> part(buffer.begin() + offset, buffer.begin() + offset + part_size);

                // В первой части - текст + часть изображения, затем - без текста
                if (is_first_part) {
                    std::string separator = "\n\n"; // Разделитель между текстом и изображением
                    std::vector<char> text_data_and_part(overlay_text.begin(), overlay_text.end());
                    text_data_and_part.insert(text_data_and_part.end(), separator.begin(), separator.end());
                    text_data_and_part.insert(text_data_and_part.end(), part.begin(), part.end());
                    sendPart(resolver, socket, address, text_data_and_part, choice, offset + part_size == image_size);
                    is_first_part = false;
                    overlay_text.clear();
                }
                else {
                    sendPart(resolver, socket, address, part, choice, offset + part_size == image_size);
                }

                offset += part_size;
            }

            // Получение ответа от сервера
            receiveParts(socket, choice, responses_path);
            logInfo("Successfully processed image: " + choice);
            std::cout << "Successfully processed image: " << choice;
        }
        catch (boost::beast::system_error& err) {
            std::cout << "Error: " << err.what() << std::endl << std::endl;
            logError(err.what());
        }
    }
    return 0;
}