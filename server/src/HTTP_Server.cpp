#include "MessageHandler.h"
#include <iostream>
#include <vector>
#include <thread>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

int main() {
    try {
        const auto& numOfProcessors = std::max(1, std::thread::hardware_concurrency());
        auto ioc = std::make_shared<asio::io_context>(numOfProcessors);

        auto message_handler = std::make_shared<MessageHandler>(ioc);
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