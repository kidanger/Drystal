#ifndef EMSCRIPTEN

#include <sys/stat.h>

#include "file.hpp"

time_t last_modified(const char* filename)
{
    struct stat st;

    if (stat(filename, &st)) {
        return 0;
    } else {
        return st.st_mtime;
    }
}

#endif
