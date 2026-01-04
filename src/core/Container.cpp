#include "core/Container.h"
#include <iostream>
#include <sched.h>      
#include <sys/wait.h>   
#include <sys/utsname.h>
#include <unistd.h>     
#include <cstring>      
#include <cerrno>       

namespace safebox {

Container::Container() {
    child_stack.resize(STACK_SIZE);
}

int Container::child_func(void* arg) {
    Container* container = static_cast<Container*>(arg);
    container->run_child();
    return 0;
}

void Container::run_child() {
    pid_t pid = getpid();
    std::cout << "[Child] Inside container! PID: " << pid << std::endl;

    if (pid != 1) {
        std::cerr << "[Child] CRITICAL ERROR: PID is not 1. Isolation failed!" << std::endl;
        return;
    }

    std::string new_hostname = "safebox-container";
    if (sethostname(new_hostname.c_str(), new_hostname.size()) < 0) {
        std::cerr << "[Child] Failed to set hostname: " << strerror(errno) << std::endl;
        return;
    }

    char buf[128];
    if (gethostname(buf, sizeof(buf)) == 0) {
        std::cout << "[Child] New hostname: " << buf << std::endl;
    }

    std::cout << "[Child] Exiting..." << std::endl;
}

void Container::run() {
    std::cout << "[Parent] Creating child process..." << std::endl;

    pid_t child_pid = clone(
        child_func, 
        child_stack.data() + STACK_SIZE, 
        CLONE_NEWUTS | CLONE_NEWPID | SIGCHLD, 
        this
    );

    if (child_pid == -1) {
        std::cerr << "[Parent] clone() failed: " << strerror(errno) << std::endl;
        return;
    }

    std::cout << "[Parent] Child PID: " << child_pid << std::endl;

    waitpid(child_pid, nullptr, 0);
    std::cout << "[Parent] Child exited. Verifying parent hostname..." << std::endl;

    char buf[128];
    if (gethostname(buf, sizeof(buf)) == 0) {
        std::cout << "[Parent] Hostname is still: " << buf << std::endl;
    }
}

} 