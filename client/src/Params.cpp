#include "Params.h"
#include <iostream>
#include <boost/program_options.hpp>

namespace po = boost::program_options;

ClientParams parseCommandLine(int argc, char* argv[]) {
    ClientParams params;

    // Добавление всех параметров в описание (все необязательные)
    po::options_description desc("Available options");
    desc.add_options()
        ("help, h", "Help message")
        ("address, a", po::value<std::string>(&params.address)->default_value("127.0.0.1"), "Server address")
        ("port, p", po::value<std::string>(&params.port)->default_value("8080"), "Server port")
        ("images-path, i", po::value<std::string>(&params.images_path)->default_value("C:\\Users\\Public\\Pictures"), "Path to the image file")
        ("text, t", po::value<std::string>(&params.overlay_text)->default_value("default text"), "Text to overlay on the image")
        ("output-path, out", po::value<std::string>(&params.responses_path)->default_value("C:\\Users\\Public\\Pictures"), "Path to save the result");

    // Парсинг
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm); // Присвоение значений из value

    // Если в списке есть --help, выводится описание
    if (vm.count("help")) {
        std::cout << desc << "\n";
    }

    return params;
}