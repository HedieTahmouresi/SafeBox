#include "core/Filesystem.h"
#include "core/Logger.h"
#include <iostream>
#include <sys/mount.h>  
#include <sys/syscall.h>
#include <sys/stat.h>   
#include <unistd.h>     
#include <cstring>      
#include <cerrno>       

namespace safebox {

void Filesystem::pivot_root(const char* new_root, const char* put_old) {
    if (syscall(SYS_pivot_root, new_root, put_old) == -1) {
        Logger::log("[Filesystem] pivot_root failed: " + std::string(strerror(errno)), Logger::Level::ERROR);
        exit(1);
    }
}

void Filesystem::setup(const char* root_path) {
    Logger::log("[Filesystem] Setting up rootfs at: " + std::string(root_path), Logger::Level::INFO);

    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
        Logger::log("[Filesystem] Failed to make mounts private: " + std::string(strerror(errno)), Logger::Level::ERROR);
        exit(1);
    }

    if (mount(root_path, root_path, "bind", MS_BIND | MS_REC, NULL) == -1) {
        Logger::log("[Filesystem] Failed to bind mount rootfs: " + std::string(strerror(errno)), Logger::Level::ERROR);
        exit(1);
    }

    std::string old_root = std::string(root_path) + "/old_root";
    if (mkdir(old_root.c_str(), 0777) == -1 && errno != EEXIST) {
        Logger::log("[Filesystem] Failed to create old_root: " + std::string(strerror(errno)), Logger::Level::ERROR);
        exit(1);
    }

    pivot_root(root_path, old_root.c_str());

    if (chdir("/") == -1) {
        Logger::log("[Filesystem] Failed to chdir to /: " + std::string(strerror(errno)), Logger::Level::ERROR);
        exit(1);
    }

    if (umount2("/old_root", MNT_DETACH) == -1) {
        Logger::log("[Filesystem] Failed to unmount old root: " + std::string(strerror(errno)), Logger::Level::ERROR);
        exit(1);
    }
    rmdir("/old_root"); 

    Logger::log("[Filesystem] Mounting /proc...", Logger::Level::INFO);
    if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
        Logger::log("[Filesystem] Failed to mount /proc: " + std::string(strerror(errno)), Logger::Level::ERROR);
        exit(1);
    }
}

} 