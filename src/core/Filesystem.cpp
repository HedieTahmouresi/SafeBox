#include "core/Filesystem.h"
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
        std::cerr << "[Filesystem] pivot_root failed: " << strerror(errno) << std::endl;
        exit(1);
    }
}

void Filesystem::setup(const char* root_path) {
    std::cout << "[Filesystem] Setting up rootfs at: " << root_path << std::endl;

    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
        std::cerr << "[Filesystem] Failed to make mounts private: " << strerror(errno) << std::endl;
        exit(1);
    }

    if (mount(root_path, root_path, "bind", MS_BIND | MS_REC, NULL) == -1) {
        std::cerr << "[Filesystem] Failed to bind mount rootfs: " << strerror(errno) << std::endl;
        exit(1);
    }

    std::string old_root = std::string(root_path) + "/old_root";
    if (mkdir(old_root.c_str(), 0777) == -1 && errno != EEXIST) {
        std::cerr << "[Filesystem] Failed to create old_root: " << strerror(errno) << std::endl;
        exit(1);
    }

    pivot_root(root_path, old_root.c_str());

    if (chdir("/") == -1) {
        std::cerr << "[Filesystem] Failed to chdir to /: " << strerror(errno) << std::endl;
        exit(1);
    }

    if (umount2("/old_root", MNT_DETACH) == -1) {
        std::cerr << "[Filesystem] Failed to unmount old root: " << strerror(errno) << std::endl;
        exit(1);
    }
    rmdir("/old_root"); 

    std::cout << "[Filesystem] Mounting /proc..." << std::endl;
    if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
        std::cerr << "[Filesystem] Failed to mount /proc: " << strerror(errno) << std::endl;
        exit(1);
    }
}

} 