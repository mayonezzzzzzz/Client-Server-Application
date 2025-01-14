#include "MessageHandler.h"
#include <iostream>
#include <vector>
#include <thread>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

/*void do_accept(std::shared_ptr<asio::io_context> ioc, tcp::acceptor& acceptor) {
    acceptor.async_accept([ioc, &acceptor](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::cout << "New connection accepted.\n";
            auto shared_socket = std::make_shared<tcp::socket>(std::move(socket));
            start_session(shared_socket); // Обрабатываем соединение
        }
        else {
            std::cerr << "Accept error: " << ec.message() << "\n";
        }
        do_accept(ioc, acceptor); // Продолжаем принимать новые соединения
        });
}*/

int main() {
    try {
        auto ioc = std::make_shared<asio::io_context>(6);

        auto message_handler = std::make_shared<MessageHandler>(ioc);
        message_handler->startHandle();

        // пул потоков
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