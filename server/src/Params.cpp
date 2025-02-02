#include "Params.h"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

ServerParams parseCommandLine(int argc, char* argv[]) {
    ServerParams params;

    // Добавление параметров в описание
    po::options_description desc("Available options");
    desc.add_options()
        ("help, h", "Produce help message")
        ("port, p", po::value<std::string>(&params.port)->default_value("8080"), "Port")
        ("max-requests, max", po::value<size_t>(&params.max_requests)->default_value(3), "Maximum number of parallel requests");

    // Парсинг
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
    }

    return params;
}