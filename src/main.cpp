#include "core/Container.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

void print_usage(const char* prog_name) {
    std::cerr << "Usage: " << prog_name << " --cmd <command> [args...]" << std::endl;
    std::cerr << "Example: " << prog_name << " --cmd /bin/sh" << std::endl;
    std::cerr << "Example: " << prog_name << " --cmd /bin/echo Hello World" << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    safebox::ContainerConfig config;
    bool cmd_found = false;

    // Basic Argument Parser
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--cmd") == 0) {
            cmd_found = true;
            // Everything after --cmd is part of the command
            for (int j = i + 1; j < argc; ++j) {
                config.command.push_back(argv[j]);
            }
            break; 
        }
    }

    if (!cmd_found || config.command.empty()) {
        std::cerr << "Error: No command specified." << std::endl;
        print_usage(argv[0]);
        return 1;
    }

    std::cout << "[SafeBox] Command detected: ";
    for (const auto& arg : config.command) std::cout << arg << " ";
    std::cout << std::endl;

    safebox::Container container(config);
    container.run();

    return 0;
}