#include "core/CGroupManager.h"
#include "core/Logger.h"
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <cerrno>

namespace safebox {

bool CGroupManager::write_file(const std::string& path, const std::string& value) {
    std::ofstream ofs(path);
    if (!ofs.is_open()) {
        Logger::log("[Cgroup] Failed to open " + path + ": " + std::string(strerror(errno)), Logger::Level::ERROR);
        return false;
    }
    ofs << value;
    if (ofs.fail()) {
        Logger::log("[Cgroup] Failed to write to " + path + ": " + std::string(strerror(errno)), Logger::Level::ERROR);
        return false;
    }
    return true;
}

bool CGroupManager::setup(pid_t pid, size_t memory_limit_bytes, double cpu_percent) {
    Logger::log("[Cgroup] Setting up limits for PID " + std::to_string(pid), Logger::Level::INFO);

    if (mkdir(CGROUP_DIR, 0755) == -1 && errno != EEXIST) {
        Logger::log("[Cgroup] Failed to create dir: " + std::string(strerror(errno)), Logger::Level::ERROR);
        return false;
    }

    std::string mem_path = std::string(CGROUP_DIR) + "/memory.max";
    if (!write_file(mem_path, std::to_string(memory_limit_bytes))) {
        return false;
    }

    long period = 100000;
    long quota = static_cast<long>(period * (cpu_percent / 100.0));
    
    std::string cpu_val = std::to_string(quota) + " " + std::to_string(period);
    std::string cpu_path = std::string(CGROUP_DIR) + "/cpu.max";
    
    if (!write_file(cpu_path, cpu_val)) {
        Logger::log("[Cgroup] Warning: Failed to set CPU limit (check cgroup2 delegation).", Logger::Level::ERROR);
    } else {
        Logger::log("[Cgroup] CPU Limit set to " + std::to_string(cpu_percent) + "% (" + cpu_val + ")", Logger::Level::INFO);
    }

    std::string procs_path = std::string(CGROUP_DIR) + "/cgroup.procs";
    if (!write_file(procs_path, std::to_string(pid))) {
        return false;
    }

    return true;
}

void CGroupManager::cleanup() {
    Logger::log("[Cgroup] Cleaning up...", Logger::Level::INFO);
    if (rmdir(CGROUP_DIR) == -1) {
        if (errno != ENOENT) {
             Logger::log("[Cgroup] Warning: Failed to remove cgroup dir: " + std::string(strerror(errno)), Logger::Level::ERROR);
        }
    }
}

} 