#ifndef SAFEBOX_CORE_CONTAINER_H
#define SAFEBOX_CORE_CONTAINER_H

#include <vector>
#include <string>

namespace safebox {

class Container {
private:
    static constexpr size_t STACK_SIZE = 65536; 
    std::vector<char> child_stack;

    static int child_func(void* arg);

    void run_child();

public:
    Container();
    void run();
};

} 

#endif 