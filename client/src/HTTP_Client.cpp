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

// ������ ������� � ���� base.txt � � ��������� �����
void push_response_into_files(std::filesystem::path path, http::response<http::string_body> response) {

    // � base.txt ������������ ���� ������� ������
    std::ofstream all_requests(path / "base.txt", std::ios_base::app);
    if (all_requests.is_open()) {
        all_requests << response.body() << std::endl;
        all_requests.close();
    }

    time_t now = time(0);
    std::tm local_time = *std::localtime(&now);
    std::ostringstream new_file_path;
    // ������� ���� � �����
    new_file_path << std::put_time(&local_time, "%d.%m.%Y %H-%M-%S") << ".txt";

    // ������ ������ � ���� � ������� ����� � �������� � ��������
    std::ofstream current_request(path / new_file_path.str(), std::ios_base::out);
    if (current_request.is_open()) {
        current_request << response << std::endl;
        current_request.close();
    }
}

int main()
{
    // ����� �� ����� src, ���� � ����� responses � client
    std::filesystem::path path_to_requests = std::filesystem::path(__FILE__).parent_path().parent_path() / "responses";

    if (!std::filesystem::exists(path_to_requests)) {
        std::filesystem::create_directory(path_to_requests);
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

            std::cout << "\nEnter a string to send: ";
            std::string data = "";
            std::getline(std::cin, data);

            // ������ � ���������� ������� (����� - post)
            http::request<http::string_body> request{ http::verb::post, "/", 11 };
            request.set(http::field::host, address);
            request.set(http::field::content_type, "text/plain");
            request.body() = data;
            request.prepare_payload();

            // �������� ������� ����� ������������� tcp ����������
            http::write(stream, request);

            // ����� ��� ��������
            beast::flat_buffer buffer;

            // ����� �������
            http::response<http::string_body> response;

            // ������ ������ �������
            http::read(stream, buffer, response);

            std::cout << response << std::endl;

            // ������ ������ � �����
            push_response_into_files(path_to_requests, response);

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