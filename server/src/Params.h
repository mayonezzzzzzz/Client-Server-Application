#pragma once
#include <string>

struct ServerParams {
    std::string port;
    size_t max_requests;
};

ServerParams parseCommandLine(int argc, char* argv[]);