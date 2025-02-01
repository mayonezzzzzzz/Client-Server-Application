#include "MessageHandler.h"
#include "Session.h"

MessageHandler::MessageHandler(std::shared_ptr<asio::io_context> ioc, const std::string port) :
    ioc(ioc), acceptor(*ioc, tcp::endpoint(asio::ip::make_address_v4("127.0.0.1"), std::stoi(port))) {
    acceptor.set_option(asio::socket_base::reuse_address(true)); // позволяет использовать порт и адрес сокета повторно, если тот был закрыт недавно
    acceptor.listen();
    std::cout << "HTTP server started. Port - " << port << "\n";
}

// Метод для постоянного отслеживания новых соединений
void MessageHandler::startHandle() {
    auto self = shared_from_this(); // для продления времени жизни объекта (пока используется в async_accept - не будет уничтожен)
    std::cout << "Starting handling\n";

    // Асинхронное принятие входящих соединений
    acceptor.async_accept(asio::make_strand(*ioc), [self](boost::system::error_code err, tcp::socket socket) {
        if(!err) {
            std::cout << "New connection accepted\n";
            // Создается объект сессии - обработки сообщений для данного соединения
            auto session = std::make_shared<Session>(std::move(socket));
            session->Start();
        }
        else {
            std::cerr << "Error in async_accept function: " << err.what() << "\n";
        }
        // Продолжают отслеживаться новые соединения
        self->startHandle();
        });
}