#ifndef SAFEBOX_CORE_CGROUP_MANAGER_H
#define SAFEBOX_CORE_CGROUP_MANAGER_H

#include <string>
#include <sys/types.h>

namespace safebox {

class CGroupManager {
public:
    static constexpr const char* CGROUP_DIR = "/sys/fs/cgroup/safebox";
    
    static bool setup(pid_t pid, size_t memory_limit_bytes);

    static void cleanup();

private:
    static bool write_file(const std::string& path, const std::string& value);
};

}

#endif