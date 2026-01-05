#include "core/NetworkManager.h"
#include "core/Logger.h"
#include <cstdlib>
#include <iostream>

namespace safebox {

bool NetworkManager::run_cmd(const std::string& cmd) {
    // REMOVED: " > /dev/null 2>&1" so we can see the error output!
    std::string full_cmd = cmd; 
    
    int ret = system(full_cmd.c_str());
    if (ret != 0) {
        Logger::log("Network ERROR: Command failed: " + cmd, Logger::Level::ERROR);
        // Also print to stderr so you see it immediately
        std::cerr << "[Network ERROR] Command returned " << ret << ": " << cmd << std::endl;
        return false;
    }
    return true;
}

bool NetworkManager::setup(pid_t child_pid) {
    Logger::log("Network: Configuring virtual network for PID " + std::to_string(child_pid));

    run_cmd("ip link add name sb-br0 type bridge");
    run_cmd("ip link set sb-br0 up");
    run_cmd("ip addr add 10.0.0.1/24 dev sb-br0"); 

    std::string veth_host = "veth" + std::to_string(child_pid);
    std::string veth_guest = "vpeer" + std::to_string(child_pid);
    
    if (!run_cmd("ip link add " + veth_host + " type veth peer name " + veth_guest)) {
        return false;
    }

    run_cmd("ip link set " + veth_host + " master sb-br0");
    run_cmd("ip link set " + veth_host + " up");

    if (!run_cmd("ip link set " + veth_guest + " netns " + std::to_string(child_pid))) {
        return false;
    }

    std::string ns_exec = "nsenter -t " + std::to_string(child_pid) + " -n ";
    
    run_cmd(ns_exec + "ip link set " + veth_guest + " name eth0");
    
    run_cmd(ns_exec + "ip addr add 10.0.0.2/24 dev eth0");
    
    run_cmd(ns_exec + "ip link set eth0 up");
    
    run_cmd(ns_exec + "ip route add default via 10.0.0.1");

    Logger::log("Network: Bridge and Veth pair established. IP: 10.0.0.2");
    return true;
}

void NetworkManager::cleanup() {
    Logger::log("Network: Cleaning up bridge...");
    run_cmd("ip link delete sb-br0 type bridge");
}

}