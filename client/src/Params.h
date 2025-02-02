#pragma once
#include <string>

struct ClientParams {
    std::string address;
    std::string port;
    std::string images_path;
    std::string overlay_text;
    std::string responses_path;
};

ClientParams parseCommandLine(int argc, char* argv[]);