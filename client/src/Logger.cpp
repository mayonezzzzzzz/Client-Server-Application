#include "Logger.h"
#include <fstream>
#include <ctime>
#include <iomanip>
#include <thread>
#include <boost/format.hpp>

// Функция для логирования событий в файл client.log внутри папки с проектом
void log(const std::string& level, const std::string& message) {
    std::ofstream log_file("client.log", std::ios::app);
    if (log_file.is_open()) {
        time_t now = time(0);
        std::tm local_time = *std::localtime(&now);
        auto thread_id = std::this_thread::get_id();

        std::string log = boost::str(
            boost::format("[%1%]"     " [Thread: %2%]"     " [%3%]"     " %4%")
            % std::put_time(&local_time, "%Y-%m-%d %H:%M:%S")
            % thread_id % level % message
        );

        log_file << log << std::endl;
        //log_file << "[" << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S") << "] [" << level << "] " << message << "\n";
    }
}

void logError(const std::string& message) {
    log("ERROR", message);
}

void logInfo(const std::string& message) {
    log("INFO", message);
}