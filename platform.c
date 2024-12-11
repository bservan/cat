#include "platform.h"

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>

int cat_is_regular_file(const char *path) {
    DWORD file_attributes = GetFileAttributes(path);
    if (file_attributes == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    return !(file_attributes & FILE_ATTRIBUTE_DIRECTORY);
}

int cat_is_directory(const char *path) {
    DWORD file_attributes = GetFileAttributes(path);
    if (file_attributes == INVALID_FILE_ATTRIBUTES) {
        return 0;
    }
    return (file_attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

#else
#include <sys/stat.h>

int cat_is_regular_file(const char *path) {
    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        return 0;
    }
    return S_ISREG(file_stat.st_mode);
}

int cat_is_directory(const char *path) {
    struct stat file_stat;
    if (stat(path, &file_stat) != 0) {
        return 0;
    }
    return S_ISDIR(file_stat.st_mode);
}

#endif
