#include "core/Monitor.h"
#include "core/CGroupManager.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

namespace safebox {

Monitor::Monitor() : running(false) {}

Monitor::~Monitor() {
    stop();
}

void Monitor::start() {
    running = true;
    monitor_thread = std::thread(&Monitor::display_stats, this);
}

void Monitor::stop() {
    running = false;
    if (monitor_thread.joinable()) {
        monitor_thread.join();
    }
}

std::string Monitor::read_value(const std::string& path) {
    std::ifstream ifs(path);
    std::string value;
    if (ifs >> value) return value;
    return "0";
}

void Monitor::display_stats() {
    std::string mem_path = std::string(CGroupManager::CGROUP_DIR) + "/memory.current";
    
    while (running) {
        std::string current_mem = read_value(mem_path);
        double mem_mb = std::stod(current_mem) / 1024.0 / 1024.0;

        // ANSI ESCAPE SEQUENCE BREAKDOWN:
        // \033[s  -> Save current cursor position
        // \033[H  -> Move cursor to Home (0,0)
        // \033[K  -> Clear the entire line
        // \033[u  -> Restore saved cursor position
        std::cout << "\033[s"            
                  << "\033[H"            
                  << "\033[1;33m"        // Set color to Yellow
                  << "[SafeBox Monitor] Memory Usage: " 
                  << std::fixed << std::setprecision(2) << mem_mb << " MB"
                  << "\033[0m"           // Reset color
                  << "\033[K"            
                  << "\033[u"            
                  << std::flush;

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

}