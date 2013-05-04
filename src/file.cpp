#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "file.hpp"

time_t last_modified(std::string filename)
{
    struct stat st;

    if (stat(filename.c_str(), &st)) {
        return 0;
    } else {
        return st.st_mtime;
    }
}
