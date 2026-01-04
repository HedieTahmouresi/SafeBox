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
    mkdir(old_root.c_str(), 0777);

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
}

int Container::child_func(void* arg) {
    Container* container = static_cast<Container*>(arg);
    container->run_child();
    return 0;
}

void Container::run_child() {
    setup_root("../rootfs");

    std::cout << "[Child] Filesystem isolated. Root is now Alpine." << std::endl;
    
    struct stat st;
    if (stat("/bin/sh", &st) == 0) {
        std::cout << "[Child] Verified: /bin/sh exists (Welcome to Alpine!)" << std::endl;
    } else {
        std::cerr << "[Child] Error: /bin/sh not found. Isolation might have failed." << std::endl;
    }

    if (getpid() != 1) {
        std::cerr << "[Child] CRITICAL: PID not 1." << std::endl;
    }

    std::string new_hostname = "safebox-alpine";
    sethostname(new_hostname.c_str(), new_hostname.size());

    sleep(1);
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