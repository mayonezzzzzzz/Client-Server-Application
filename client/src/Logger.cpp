#include "Logger.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include <iomanip>

// Функция для логирования событий в файл client.log внутри папки с проектом
void log(const std::string& level, const std::string& message) {
    std::ofstream log_file("client.log", std::ios::app);
    if (log_file.is_open()) {
        time_t now = time(0);
        std::tm local_time = *std::localtime(&now);
        log_file << "[" << std::put_time(&local_time, "%Y-%m-%d %H:%M:%S") << "] [" << level << "] " << message << "\n";
    }
}

void logError(const std::string& message) {
    log("ERROR", message);
}

void logInfo(const std::string& message) {
    log("INFO", message);
}