#ifndef SAFEBOX_CORE_CONTAINER_H
#define SAFEBOX_CORE_CONTAINER_H

#include <vector>
#include <string>

namespace safebox {

struct ContainerConfig {
    std::string hostname = "safebox-container";
    std::vector<std::string> command; 
};

class Container {
private:
    static constexpr size_t STACK_SIZE = 65536; 
    std::vector<char> child_stack;
    ContainerConfig config; 

    static int child_func(void* arg);
    void run_child();

public:
    explicit Container(ContainerConfig config);
    void run();
};

} 
#endif