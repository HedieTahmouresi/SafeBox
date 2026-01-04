#include "core/Security.h"
#include "core/Logger.h"
#include <seccomp.h> 
#include <iostream>
#include <cerrno>
#include <cstring>
#include <cstdlib>

namespace safebox {

void Security::enable_seccomp() {
    Logger::log("[Security] Enabling Seccomp filters...", Logger::Level::INFO);

    scmp_filter_ctx ctx = seccomp_init(SCMP_ACT_ALLOW);
    if (!ctx) {
        Logger::log("[Security] Failed to init seccomp context", Logger::Level::ERROR);
        exit(1);
    }

    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(reboot), 0);
    
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(swapon), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(swapoff), 0);

    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(init_module), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(finit_module), 0);
    seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(delete_module), 0);

    if (seccomp_load(ctx) < 0) {
        Logger::log("[Security] Failed to load seccomp rules: " + std::string(strerror(errno)), Logger::Level::ERROR);
        seccomp_release(ctx);
        exit(1);
    }

    seccomp_release(ctx);
    Logger::log("[Security] Seccomp filters active. Dangerous syscalls blocked.", Logger::Level::INFO);
}

}