cmake_minimum_required(VERSION 3.10)
project(SafeBox VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -Werror)

include_directories(include)

add_executable(safebox 
    src/main.cpp 
    src/core/Container.cpp
    src/core/CGroupManager.cpp
    src/core/Filesystem.cpp  
)