#include "core/Monitor.h"
#include "core/CGroupManager.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sys/ioctl.h> 
#include <unistd.h>   

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
    
    std::ofstream log_file("safebox_usage.log", std::ios::app);

    while (running) {
        std::string current_mem = read_value(mem_path);
        try {
            double mem_mb = std::stod(current_mem) / 1024.0 / 1024.0;
            
            log_file << "[MONITOR] Memory Usage: " << mem_mb << " MB" << std::endl;
        } catch (...) {}

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

}