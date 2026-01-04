#ifndef SAFEBOX_CORE_CONTAINER_H
#define SAFEBOX_CORE_CONTAINER_H

#include <vector>
#include <string>

namespace safebox {

class Container {
private:
    static constexpr size_t STACK_SIZE = 65536; 
    std::vector<char> child_stack;

    void run_pivot_root(const char* new_root, const char* put_old);
    
    void setup_root(const char* root_path);

    static int child_func(void* arg);
    void run_child();

public:
    Container();
    void run();
};

} 
#endif