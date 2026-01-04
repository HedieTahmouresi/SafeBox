#ifndef SAFEBOX_CORE_LOGGER_H
#define SAFEBOX_CORE_LOGGER_H

#include <string>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <ctime>

namespace safebox {

class Logger {
public:
    enum class Level { INFO, ERROR };

    static void log(const std::string& message, Level level = Level::INFO) {
        std::ofstream log_file("safebox.log", std::ios::app);
        if (!log_file.is_open()) return;

        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        struct tm *timeinfo = std::localtime(&now);
        
        std::string label = (level == Level::ERROR) ? "[ERROR]" : "[INFO]";
        
        log_file << "[" << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S") << "] " 
                 << label << " " << message << std::endl;
    }
};

} 
#endif