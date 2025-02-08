#include "MessageHandler.h"
#include "Session.h"
#include "Params.h"
#include <iostream>
#include <vector>
#include <thread>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

int main(int argc, char* argv[]) {
    try {
        // Парсинг аргументов командной строки
        ServerParams params = parseCommandLine(argc, argv);

        MAX_REQUESTS = params.max_requests;

        const auto& numOfProcessors = std::max(1u, std::thread::hardware_concurrency() - 1);
        auto ioc = std::make_shared<asio::io_context>(numOfProcessors);

        // Используется порт, указанный в параметрах
        auto message_handler = MessageHandler::createMessageHandler(ioc, params.port);
        message_handler->startHandle();

        // Пул потоков
        std::vector<std::thread> threads;
        threads.reserve(numOfProcessors);
        for (std::size_t i = 0; i < numOfProcessors; ++i) {
            threads.emplace_back([ioc]() {
                ioc->run(); // Каждый поток запускает обработку задач в io_context
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }
    }
    catch (boost::system::system_error& err) {
        std::cerr << "Server start error: " << err.what() << "\n";
    }

    return 0;
}