#ifndef SAFEBOX_CORE_MONITOR_H
#define SAFEBOX_CORE_MONITOR_H

#include <string>
#include <atomic>
#include <thread>

namespace safebox {

class Monitor {
public:
    Monitor();
    ~Monitor();

    void start();
    
    void stop();

private:
    std::atomic<bool> running;
    std::thread monitor_thread;

    void display_stats();
    std::string read_value(const std::string& path);
};

} 
#endif