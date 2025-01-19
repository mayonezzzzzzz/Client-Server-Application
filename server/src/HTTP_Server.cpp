#include "MessageHandler.h"
#include <iostream>
#include <vector>
#include <thread>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

int main() {
    try {
        auto ioc = std::make_shared<asio::io_context>(6);

        auto message_handler = std::make_shared<MessageHandler>(ioc);
        message_handler->startHandle();

        // Пул потоков
        std::vector<std::thread> threads;
        threads.reserve(6);
        for (std::size_t i = 0; i < 6; ++i) {
            threads.emplace_back([ioc]() {
                ioc->run(); // Каждый поток запускает обработку задач в io_context
                });
        }

        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }
    catch (boost::system::system_error& err) {
        std::cerr << "Server start error: " << err.what() << "\n";
    }

    return 0;
}