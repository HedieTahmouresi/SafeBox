#include "core/Container.h"
#include "core/Logger.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstring>

void print_usage(const char* prog_name) {
    safebox::Logger::log("Usage: " + std::string(prog_name) + " --cmd <command> [args...]", safebox::Logger::Level::ERROR);
    safebox::Logger::log("Example: " + std::string(prog_name) + " --cmd /bin/sh", safebox::Logger::Level::ERROR);
    safebox::Logger::log("Example: " + std::string(prog_name) + " --cmd /bin/echo Hello World", safebox::Logger::Level::ERROR);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    safebox::ContainerConfig config;
    bool cmd_found = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--cmd") == 0) {
            cmd_found = true;
            for (int j = i + 1; j < argc; ++j) {
                config.command.push_back(argv[j]);
            }
            break; 
        } else if (strcmp(argv[i], "--cpu") == 0 && i + 1 < argc) {
            config.cpu_limit = std::stod(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--mem") == 0 && i + 1 < argc) {
            config.memory_limit = static_cast<size_t>(std::stoul(argv[i + 1]) * 1024 * 1024);
            i++; 
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