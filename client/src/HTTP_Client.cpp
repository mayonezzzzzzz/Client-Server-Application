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

// ������ ������� � ����� � ����������� .jpeg
void saveResponse(std::filesystem::path path, http::response<http::vector_body<char>> response) {

    time_t now = time(0);
    std::tm local_time = *std::localtime(&now);
    std::ostringstream new_file_path;
    // ������� ���� � �����
    new_file_path << std::put_time(&local_time, "%d.%m.%Y %H-%M-%S") << ".jpeg";

    // �������� ����������� � ������� ����� � �������� � ��������
    std::ofstream image_file(path / new_file_path.str(), std::ios::binary);
    if (image_file.is_open()) {
        image_file.write(response.body().data(), response.body().size());
        image_file.close();
    }
}

int main()
{
    // ����� �� ����� src, ���� � ����� responses � client
    std::filesystem::path path_to_requests = std::filesystem::path(__FILE__).parent_path().parent_path() / "responses";

    if (!std::filesystem::exists(path_to_requests)) {
        std::filesystem::create_directory(path_to_requests);
    }

    // ���� � �����, �� ������� ��������� ����������� ��� ��������
    std::filesystem::path images_path = std::filesystem::path(__FILE__).parent_path().parent_path() / "images";

    if (!std::filesystem::exists(images_path)) {
        std::filesystem::create_directory(images_path);
    }

    while (true) {
        try {
            // �������� Input-Output
            asio::io_context iocont;

            tcp::resolver resolver(iocont);

            beast::tcp_stream stream(iocont);

            // ����
            std::string const port = "8080";

            // ip-����� lockalhost-�
            std::string const address = "127.0.0.1";

            // ������ �� endpoints (�������� �����)
            auto const result = resolver.resolve(address, port);

            // ��������� tcp ����������
            stream.connect(result);

            std::cout << "List of images" << std::endl
                << "--------------" << std::endl;
            for (const auto& image : std::filesystem::directory_iterator(images_path)) {
                std::cout << image.path().filename() << std::endl;
            }
            std::cout << "\nChoose the name of the image to send: " << std::endl;
            std::string choice;
            std::getline(std::cin, choice);

            // ���������� � �������� ������ ����������� ����������� ��� ��������
            std::vector<char> image_data;
            for (const auto& image : std::filesystem::directory_iterator(images_path)) {
                if (image.path().filename() == choice) {
                    std::filesystem::path image_path = images_path / image.path().filename();
                    std::ifstream image_in_binary_view(image_path, std::ios::binary);

                    // ��������� ������� �����������
                    image_in_binary_view.seekg(0, std::ios::end);
                    std::streamsize image_size = image_in_binary_view.tellg();
                    image_in_binary_view.seekg(0, std::ios::beg);

                    // ����������� ����������� ����������� � vector
                    image_data.resize(image_size, '\0');
                    image_in_binary_view.read(&image_data[0], image_size);
                }
            }

            // ������ � ������������, �������������� ������� ������ (����� - post)
            http::request<http::vector_body<char>> request{ http::verb::post, "/", 11 };
            request.set(http::field::host, address);
            request.set(http::field::content_type, "image/jpeg");
            // ������������ std::move ��� �������������� �����������
            request.body() = std::move(image_data);
            request.prepare_payload();

            // �������� ������� ����� ������������� tcp ����������
            http::write(stream, request);

            // ����� ��� ��������
            beast::flat_buffer buffer;

            // ����� �������
            http::response<http::vector_body<char>> response;

            // ������ ������ �������
            http::read(stream, buffer, response);

            std::cout << response << std::endl;

            // ���������� ������ ��� �����������
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