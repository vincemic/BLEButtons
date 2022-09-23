#include "FileSystem.h"
#include <LittleFS.h>
#include <ArduinoLog.h>

#define FORMAT_LITTLEFS_IF_FAILED true

FileSystemClass::FileSystemClass()
{
}

void FileSystemClass::begin()
{

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
    {
        Log.errorln(F("[FileSystem] Mount failed"));
        return;
    }
    else
    {
        Log.noticeln(F("[FileSystem] Mounted"));
    }
}

size_t FileSystemClass::readFile(const char *path,char *buffer, size_t bufferSize)
{

    Log.traceln(F("[FileSystem] Reading file: %s"), path);

    size_t count = 0;
    File file = LittleFS.open(path);

    if (!file || file.isDirectory())
    {
        Log.errorln(F("[FileSystem] Failed to open file for reading"));
        return -1;
    }

    uint8_t small_buffer[1];
    while (file.available() && count < bufferSize)
    {
        file.read(small_buffer,1);
        buffer[count] = small_buffer[0];
        count++;
    }
    file.close();

    return count;
}

void FileSystemClass::deleteFile(const char *path)
{
    Log.traceln(F("[FileSystem] Deleting file and empty folders on path: %s"), path);

    if (LittleFS.remove(path))
    {
        Log.traceln(F("[FileSystem] File deleted"));
    }
    else
    {
        Log.errorln(F("[FileSystem] Delete failed"));
    }

    char *pathStr = strdup(path);
    if (pathStr)
    {
        char *ptr = strrchr(pathStr, '/');
        if (ptr)
        {
            Log.traceln(F("[FileSystem] Removing all empty folders on path: %s"), path);
        }
        while (ptr)
        {
            *ptr = 0;
            LittleFS.rmdir(pathStr);
            ptr = strrchr(pathStr, '/');
        }
        free(pathStr);
    }
}

void FileSystemClass::writeFile(const char *path, const char *contents)
{
    if (!LittleFS.exists(path))
    {
        if (strchr(path, '/'))
        {
            Log.traceln(F("[FileSystem] Create missing folders of: %s"), path);
            char *pathStr = strdup(path);
            if (pathStr)
            {
                char *ptr = strchr(pathStr, '/');
                while (ptr)
                {
                    *ptr = 0;
                    LittleFS.mkdir(pathStr);
                    *ptr = '/';
                    ptr = strchr(ptr + 1, '/');
                }
            }
            free(pathStr);
        }
    }

    Log.traceln(F("[FileSystem] Writing file to: %s"), path);
    File file = LittleFS.open(path, FILE_WRITE);
    if (!file)
    {
        Log.errorln(F("[FileSystem] Failed to open file for writing"));
        return;
    }
    if (file.print(contents))
    {
        Log.traceln(F("[FileSystem] File written"));
    }
    else
    {
        Log.errorln(F("[FileSystem] Write failed"));
    }
    file.close();
}

FileSystemClass FileSystem;
