#pragma once
#include <cstddef>
class FileSystemClass
{

public:
    FileSystemClass();
    void begin();
    void writeFile(const char *path, const char *contents);
    void deleteFile(const char *path);
    size_t readFile(const char *path,char *buffer, size_t bufferSize);
};

extern FileSystemClass FileSystem;