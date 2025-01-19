#include "ImageHandler.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>

// Функция для отправки одной части изображения
void sendPart(tcp::resolver& resolver, tcp::socket& socket, const std::string& address, const std::vector<char>& part, const std::string& image_id, bool is_last_part) {
    http::request<http::vector_body<char>> request{ http::verb::post, "/", 11 };
    request.set(http::field::host, address);
    request.set(http::field::content_type, "image/jpeg");
    request.set("Image-ID", image_id);
    request.set("Last-Part", is_last_part ? "1" : "0");
    request.body() = part;
    request.prepare_payload();

    http::write(socket, request);
}

// Функция для получения ответа по частям и записи его в jpeg файл
void receiveParts(tcp::socket& socket, const std::string& image_id, const std::filesystem::path& save_path) {
    beast::flat_buffer buffer;
    std::vector<char> image_data;

    while (true) {
        http::response<http::vector_body<char>> response;
        http::read(socket, buffer, response);

        std::string received_id(response["Image-ID"].data(), response["Image-ID"].length());
        if (received_id != image_id) {
            throw std::runtime_error("Unknown image ID in response");
        }

        image_data.insert(image_data.end(), response.body().begin(), response.body().end());

        if (response["Last-Part"] == "1") {
            break;
        }
    }

    time_t now = time(0);
    std::tm local_time = *std::localtime(&now);
    std::ostringstream new_file_path;
    // Имя файла - текущие дата и время + ID полученного изображения
    new_file_path << std::put_time(&local_time, "%d.%m.%Y %H-%M-%S ID-") << image_id << ".jpeg";

    std::ofstream image_file(save_path / new_file_path.str(), std::ios::binary);
    if (image_file.is_open()) {
        image_file.write(image_data.data(), image_data.size());
    }
}