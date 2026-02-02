#ifndef SAFEBOX_CORE_LOGGER_H
#define SAFEBOX_CORE_LOGGER_H

#include <string>
#include <fstream>
#include <iostream> 
#include <chrono>
#include <iomanip>
#include <ctime>

namespace safebox {

class Logger {
public:
    enum class Level { INFO, ERROR };

    static void log(const std::string& message, Level level = Level::INFO) {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        struct tm *timeinfo = std::localtime(&now);
        
        std::string label = (level == Level::ERROR) ? "[ERROR]" : "[INFO]";
        char time_buf[64];
        std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", timeinfo);
        
        std::cerr << "[" << time_buf << "] " << label << " " << message << std::endl;

        std::ofstream log_file("safebox.log", std::ios::app);
        if (log_file.is_open()) {
            log_file << "[" << time_buf << "] " << label << " " << message << std::endl;
        }
    }
};

} 
#endif