#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <string>
#include <filesystem>
#include <vector>

#pragma warning(disable : 4996)

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = boost::beast::http;
using tcp = asio::ip::tcp;

const size_t BUFFER_SIZE = 1024 * 1024;

void sendPart(tcp::resolver& resolver, tcp::socket& socket, const std::string& address, const std::vector<char>& part, const std::string& image_id, bool is_last_part);
void receiveParts(tcp::socket& socket, const std::string& image_id, const std::filesystem::path& save_path);