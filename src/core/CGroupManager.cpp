#include "core/CGroupManager.h"
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
        std::cerr << "[Cgroup] Failed to open " << path << ": " << strerror(errno) << std::endl;
        return false;
    }
    ofs << value;
    if (ofs.fail()) {
        std::cerr << "[Cgroup] Failed to write to " << path << ": " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

bool CGroupManager::setup(pid_t pid, size_t memory_limit_bytes, double cpu_percent) {
    std::cout << "[Cgroup] Setting up limits for PID " << pid << "..." << std::endl;

    if (mkdir(CGROUP_DIR, 0755) == -1 && errno != EEXIST) {
        std::cerr << "[Cgroup] Failed to create dir: " << strerror(errno) << std::endl;
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
        std::cerr << "[Cgroup] Warning: Failed to set CPU limit (check cgroup2 delegation)." << std::endl;
    } else {
        std::cout << "[Cgroup] CPU Limit set to " << cpu_percent << "% (" << cpu_val << ")" << std::endl;
    }

    std::string procs_path = std::string(CGROUP_DIR) + "/cgroup.procs";
    if (!write_file(procs_path, std::to_string(pid))) {
        return false;
    }

    return true;
}

void CGroupManager::cleanup() {
    std::cout << "[Cgroup] Cleaning up..." << std::endl;
    if (rmdir(CGROUP_DIR) == -1) {
        if (errno != ENOENT) {
             std::cerr << "[Cgroup] Warning: Failed to remove cgroup dir: " << strerror(errno) << std::endl;
        }
    }
}

} 