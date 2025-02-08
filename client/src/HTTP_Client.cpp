#include "ImageHandler.h"
#include "Logger.h"
#include "Params.h"
#include <boost/asio.hpp>
#include <iostream>
#include <fstream>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

const size_t MAX_INPUT_LENGTH = 100;
const size_t BUFFER_SIZE = 1024 * 1024;

bool checkDirectory(std::string& path) {
    std::error_code ec;
    if (ec || !std::filesystem::is_directory(path, ec)) {
        std::cerr << "Invalid directory - " << path << "\n";
        std::cerr << "Error: " << ec.message() << "\n";
        path = ".";
        return false;
    }
    return true;
}

void detectDirectories(ClientParams& params) {
    checkDirectory(params.images_path);
    checkDirectory(params.responses_path);
}

int main(int argc, char* argv[]) {
    // Парсинг аргументов командной строки
    ClientParams params = parseCommandLine(argc, argv);

    detectDirectories(params);

    std::filesystem::path images_path = params.images_path;
    std::filesystem::path responses_path = std::filesystem::path(params.responses_path) / "responses";

    std::filesystem::create_directory(images_path);
    std::filesystem::create_directory(responses_path);

    std::cout << "If you want to specify a directory to take images from, write the full path to it into the params\n\n";

    while (true) {
        try {
            asio::io_context ioc;
            tcp::resolver resolver(ioc);
            tcp::socket socket(ioc);
            std::string address = params.address;

            auto endpoints = resolver.resolve(address, params.port);
            asio::connect(socket, endpoints);

            std::cout << "Available images:\n";
            for (const auto& image : std::filesystem::directory_iterator(images_path)) {
                if (image.path().extension() == ".jpeg") {
                    std::cout << image.path().filename() << "\n";
                }
            }

            std::cout << "Enter the name of the image to send: ";
            std::string choice;
            std::getline(std::cin, choice);

            std::filesystem::path image_path = images_path / (choice + ".jpeg");
            if (!std::filesystem::exists(image_path)) {
                std::cout << "Image not found: " + image_path.string() << ".jpeg\n\n";
                logError("Image not found: " + image_path.string());
                continue;
            }

            std::string overlay_text = params.overlay_text;

            if (overlay_text.empty()) {
                overlay_text = "default text";
            }
            else if (overlay_text.length() > MAX_INPUT_LENGTH) {
                overlay_text = overlay_text.substr(0, MAX_INPUT_LENGTH);
            }

            // Отправка текста для наложения
            sendTextJson(socket, address, choice, overlay_text);

            std::ifstream image_file(image_path, std::ios::binary);
            image_file.seekg(0, std::ios::end);
            size_t image_size = image_file.tellg();
            image_file.seekg(0, std::ios::beg);

            // Отправка изображения
            std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(image_file), {});
            const auto& buffer_begin = buffer.begin();
            size_t offset = 0;

            while (offset < image_size) {
                /*  Изображение отправляется по частям не более 1Мб  */
                size_t part_size = std::min(BUFFER_SIZE, image_size - offset);
                std::vector<unsigned char> part(buffer_begin + offset, buffer_begin + offset + part_size);
                sendPart(socket, address, part, choice, offset + part_size == image_size);
                offset += part_size;
            }

            // Получение ответа от сервера
            receiveParts(socket, choice, responses_path);
            logInfo("Successfully processed image: " + choice);
            std::cout << "Successfully processed image: " << choice << ".jpeg\n\n";
        }
        catch (boost::beast::system_error& err) {
            std::cout << "Error: " << err.what() << std::endl << std::endl;
            logError(err.what());
        }
    }
    return 0;
}