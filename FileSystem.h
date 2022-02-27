#pragma once

class FileSystem
{

public:
    FileSystem();
    void begin();
    void FileSystem::writeFile(const char *path, const char *contents);
    void FileSystem::deleteFile(const char *path);
    void FileSystem::readFile(const char *path, Stream *stream);
};

extern FileSystem Files;