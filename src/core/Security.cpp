#include "core/Security.h"
#include <seccomp.h> 
#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdlib>

namespace safebox {

void Security::enable_seccomp() {
    std::cout << "[Security] Enabling Seccomp filters..." << std::endl;

    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (!ctx) {
        std::cerr << "[Security] Failed to init seccomp context" << std::endl;
        exit(1);
    }

    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(reboot), 0);
    
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(swapon), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(swapoff), 0);

    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(init_module), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(finit_module), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(delete_module), 0);

    if (seccomp_load(ctx) < 0) {
        std::cerr << "[Security] Failed to load seccomp rules: " << strerror(errno) << std::endl;
        seccomp_release(ctx);
        exit(1);
    }

    seccomp_release(ctx);
    std::cout << "[Security] Seccomp filters active. Dangerous syscalls blocked." << std::endl;
}

}