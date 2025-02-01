#pragma once
#include <filesystem>

struct ClientParams {
    std::string server_address;
    std::string server_port;
    std::filesystem::path images_path;
    std::string overlay_text;
    std::filesystem::path responses_path;
};

ClientParams parseCommandLine(int argc, char* argv[]);