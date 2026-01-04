#include "core/Container.h"
#include <iostream>
int main(int argc, char** argv) {
    (void)argc; (void)argv;
    std::cout << "[SafeBox] Starting..." << std::endl;
    safebox::Container container;
    container.run();
    return 0;
}