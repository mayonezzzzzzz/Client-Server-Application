#include "ImageHandler.h"
#include <boost/json.hpp>
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>

namespace json = boost::json;

void sendTextJson(tcp::socket& socket, const std::string& address, const std::string& image_id, const std::string& overlay_text) {
    http::request<http::string_body> request{ http::verb::post, "/json_metadata", 11 };
    request.set(http::field::host, address);
    request.set(http::field::content_type, "application/json");
    request.set("Image-ID", image_id);

    json::value data = {
        {"overlay_text", overlay_text}
    };

    request.body() = json::serialize(data);
    request.prepare_payload();

    http::write(socket, request);
}

// Функция для отправки одной части изображения
void sendPart(tcp::socket& socket, const std::string& address, const std::vector<unsigned char>& part, const std::string& image_id, bool is_last_part) {
    http::request<http::vector_body<unsigned char>> request{ http::verb::post, "/", 11 };
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
    std::vector<unsigned char> image_data;

    while (true) {
        http::response<http::vector_body<unsigned char>> response;
        http::read(socket, buffer, response);

        const auto& response_image_id = response["Image-ID"];
        std::string received_id(response_image_id.data(), response_image_id.length());
        if (received_id != image_id) {
            throw std::runtime_error("Unknown image ID in response");
        }

        const auto& response_body = response.body();
        image_data.insert(image_data.end(), response_body.begin(), response_body.end());

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
        image_file.write(reinterpret_cast<const char*>(image_data.data()), image_data.size());
    }
}