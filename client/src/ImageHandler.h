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

void sendTextJson(tcp::socket& socket, const std::string& address, const std::string& image_id, const std::string& overlay_text);
void sendPart(tcp::socket& socket, const std::string& address, const std::vector<unsigned char>& part, const std::string& image_id, bool is_last_part);
void receiveParts(tcp::socket& socket, const std::string& image_id, const std::filesystem::path& save_path);