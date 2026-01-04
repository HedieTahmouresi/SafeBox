#include "core/Container.h"
#include <iostream>
#include <sched.h>      
#include <sys/wait.h>   
#include <sys/utsname.h>
#include <unistd.h>     
#include <cstring>      
#include <cerrno>       
#include <sys/mount.h>  
#include <sys/syscall.h>
#include <sys/stat.h>   
#include <dirent.h>     
#include <vector>       
#include <algorithm>    
#include <cctype>       

namespace safebox {

Container::Container() {
    child_stack.resize(STACK_SIZE);
}

void Container::run_pivot_root(const char* new_root, const char* put_old) {
    if (syscall(SYS_pivot_root, new_root, put_old) == -1) {
        std::cerr << "[Child] pivot_root failed: " << strerror(errno) << std::endl;
        exit(1);
    }
}

void Container::setup_root(const char* root_path) {
    std::cout << "[Child] Setting up rootfs at: " << root_path << std::endl;

    if (mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL) == -1) {
        std::cerr << "[Child] Failed to make mounts private: " << strerror(errno) << std::endl;
        exit(1);
    }
    if (mount(root_path, root_path, "bind", MS_BIND | MS_REC, NULL) == -1) {
        std::cerr << "[Child] Failed to bind mount rootfs: " << strerror(errno) << std::endl;
        exit(1);
    }

    std::string old_root = std::string(root_path) + "/old_root";
    if (mkdir(old_root.c_str(), 0777) == -1 && errno != EEXIST) {
        std::cerr << "[Child] Failed to create old_root: " << strerror(errno) << std::endl;
        exit(1);
    }

    run_pivot_root(root_path, old_root.c_str());

    if (chdir("/") == -1) {
        std::cerr << "[Child] Failed to chdir to /: " << strerror(errno) << std::endl;
        exit(1);
    }

    if (umount2("/old_root", MNT_DETACH) == -1) {
        std::cerr << "[Child] Failed to unmount old root: " << strerror(errno) << std::endl;
        exit(1);
    }
    rmdir("/old_root"); 

    if (mount("proc", "/proc", "proc", 0, NULL) == -1) {
        std::cerr << "[Child] Failed to mount /proc: " << strerror(errno) << std::endl;
        exit(1);
    }
}

int Container::child_func(void* arg) {
    Container* container = static_cast<Container*>(arg);
    container->run_child();
    return 0;
}

void Container::run_child() {
    setup_root("../rootfs");

    std::string new_hostname = "safebox-alpine";
    if (sethostname(new_hostname.c_str(), new_hostname.size()) < 0) {
        std::cerr << "[Child] Failed to set hostname: " << strerror(errno) << std::endl;
    }

    if (getpid() != 1) {
        std::cerr << "[Child] CRITICAL: PID not 1. Isolation failed!" << std::endl;
    }

    std::cout << "[Child] Scanning /proc for running processes..." << std::endl;

    DIR* dir = opendir("/proc");
    if (!dir) {
        std::cerr << "[Child] Failed to open /proc: " << strerror(errno) << std::endl;
        return;
    }

    struct dirent* entry;
    std::vector<std::string> pids;

    while ((entry = readdir(dir)) != nullptr) {
        if (isdigit(entry->d_name[0])) {
            pids.push_back(entry->d_name);
        }
    }
    closedir(dir);

    std::cout << "[Child] Visible PIDs: ";
    for (const auto& pid : pids) {
        std::cout << pid << " ";
    }
    std::cout << std::endl;

    if (pids.size() == 1 && pids[0] == "1") {
        std::cout << "[Child] SUCCESS: Only PID 1 is visible! (Procfs isolation works)" << std::endl;
    } else {
        std::cout << "[Child] WARNING: Strange PIDs visible. Isolation might be partial." << std::endl;
    }

    std::cout << "[Child] Exiting..." << std::endl;
}

void Container::run() {
    std::cout << "[Parent] Creating child process..." << std::endl;

    pid_t child_pid = clone(
        child_func, 
        child_stack.data() + STACK_SIZE, 
        CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, 
        this
    );

    if (child_pid == -1) {
        std::cerr << "[Parent] clone() failed: " << strerror(errno) << std::endl;
        return;
    }

    waitpid(child_pid, nullptr, 0);
    std::cout << "[Parent] Child exited." << std::endl;
}

} 