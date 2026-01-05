#ifndef SAFEBOX_CORE_NETWORKMANAGER_H
#define SAFEBOX_CORE_NETWORKMANAGER_H

#include <string>
#include <sys/types.h>

namespace safebox {

class NetworkManager {
public:
    static bool setup(pid_t child_pid);

    static void cleanup();

private:
    static bool run_cmd(const std::string& cmd);
};

} 
#endif