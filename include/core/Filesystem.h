#ifndef SAFEBOX_CORE_FILESYSTEM_H
#define SAFEBOX_CORE_FILESYSTEM_H

#include <string>

namespace safebox {

class Filesystem {
public:
    static void setup(const char* root_path);

private:
    static void pivot_root(const char* new_root, const char* put_old);
};

} 

#endif