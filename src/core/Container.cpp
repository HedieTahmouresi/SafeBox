#include "core/Container.h"
#include "core/CGroupManager.h"
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

struct CloneArgs {
    Container* container;
    int pipe_fd; 
};

int Container::child_func(void* arg) {
    CloneArgs* args = static_cast<CloneArgs*>(arg);
    char ch;
    if (read(args->pipe_fd, &ch, 1) != 1) {
        std::cerr << "[Child] Failed to read from sync pipe!" << std::endl;
        return 1;
    }
    close(args->pipe_fd);
    args->container->run_child();
    return 0;
}

void Container::run_child() {
    setup_root("../rootfs");

    std::string new_hostname = "safebox-alpine";
    sethostname(new_hostname.c_str(), new_hostname.size());

    std::cout << "[Child] Scanning /proc..." << std::endl;
    DIR* dir = opendir("/proc");
    if (dir) {
        struct dirent* entry;
        std::vector<std::string> pids;
        while ((entry = readdir(dir)) != nullptr) {
            if (isdigit(entry->d_name[0])) pids.push_back(entry->d_name);
        }
        closedir(dir);
        if (pids.size() == 1 && pids[0] == "1") {
            std::cout << "[Child] SUCCESS: PID 1 verified." << std::endl;
        }
    }

    std::cout << "[Child] Exiting..." << std::endl;
}

void Container::run() {
    std::cout << "[Parent] Creating child process..." << std::endl;

    int pipe_fds[2];
    if (pipe(pipe_fds) == -1) {
        perror("pipe");
        return;
    }

    CloneArgs args = { this, pipe_fds[0] };

    pid_t child_pid = clone(
        child_func, 
        child_stack.data() + STACK_SIZE, 
        CLONE_NEWUTS | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, 
        &args
    );

    if (child_pid == -1) {
        std::cerr << "[Parent] clone() failed: " << strerror(errno) << std::endl;
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return;
    }

    close(pipe_fds[0]);

    if (CGroupManager::setup(child_pid, 10 * 1024 * 1024)) { 
        write(pipe_fds[1], "X", 1);
    } else {
        std::cerr << "[Parent] CGroup setup failed. Killing child." << std::endl;
        kill(child_pid, SIGKILL);
    }
    close(pipe_fds[1]);

    waitpid(child_pid, nullptr, 0);
    std::cout << "[Parent] Child exited." << std::endl;

    CGroupManager::cleanup();
}

} 