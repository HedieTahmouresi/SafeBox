#include "core/Container.h"
#include "core/CGroupManager.h"
#include "core/Filesystem.h" 
#include "core/Security.h"
#include "core/Monitor.h"
#include "core/Logger.h"
#include "core/NetworkManager.h"
#include <iostream>
#include <sched.h>      
#include <sys/wait.h>   
#include <unistd.h>     
#include <cstring>      
#include <cerrno>       
#include <vector>       

namespace safebox {

Container::Container(ContainerConfig config) : config(std::move(config)) {
    child_stack.resize(STACK_SIZE);
}

struct CloneArgs {
    Container* container;
    int pipe_fd; 
};

int Container::child_func(void* arg) {
    CloneArgs* args = static_cast<CloneArgs*>(arg);
    char ch;
    if (read(args->pipe_fd, &ch, 1) != 1) {
        Logger::log("[Child] Failed to read from sync pipe!", Logger::Level::ERROR);
        return 1;
    }
    close(args->pipe_fd);
    args->container->run_child();
    return 0;
}

void Container::run_child() {
    Logger::log("[Child] Configuring environment...", Logger::Level::INFO);

    Filesystem::setup("../rootfs");

    if (sethostname(config.hostname.c_str(), config.hostname.size()) < 0) {
        std::cerr << "[Child] Failed to set hostname: " << strerror(errno) << std::endl;
    }

    Security::enable_seccomp();

    std::vector<char*> args;
    for (const auto& arg : config.command) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr); 

    execvp(args[0], args.data());

    std::cerr << "[Child] CRITICAL: execvp failed: " << strerror(errno) << std::endl;
    exit(1);
}

void Container::run() {
    Logger::log("--------------------------------------------------");
    Logger::log("[Parent] Starting new container session.", Logger::Level::INFO);

    std::string cmd_string = "";
    for(const auto& s : config.command) cmd_string += s + " ";
    Logger::log("[Parent] Requested Command: " + cmd_string, Logger::Level::INFO);

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return;
    }

    CloneArgs args = { this, pipe_fds[0] };


    pid_t child_pid = clone(
        child_func, 
        child_stack.data() + STACK_SIZE, 
        CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET | SIGCHLD, 
        &args
    );


    if (child_pid == -1) {
        Logger::log("[Parent] clone() failed: " + std::string(strerror(errno)), Logger::Level::ERROR);
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return;
    }

    close(pipe_fds[0]);

    if (CGroupManager::setup(child_pid, config.memory_limit, config.cpu_limit)) {

        Logger::log("[Parent] Resources limited successfully.", Logger::Level::INFO);
        write(pipe_fds[1], "X", 1);
    } else {
        Logger::log("[Parent] CGroup setup failed. Killing child.", Logger::Level::ERROR);
        kill(child_pid, SIGKILL);
        return;
    }


    close(pipe_fds[1]);

    Monitor monitor;
    monitor.start(); 

    Logger::log("[Parent] Waiting for child process...", Logger::Level::INFO);
    waitpid(child_pid, nullptr, 0);

    monitor.stop(); 

    Logger::log("[Parent] Child exited. Session complete.");
    Logger::log("--------------------------------------------------");

    CGroupManager::cleanup();
}
}
