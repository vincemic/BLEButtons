#include "FileSystem.h"
#include "FS.h"
#include <LittleFS.h>
#include <ArduinoLog.h>

#define FORMAT_LITTLEFS_IF_FAILED true

FileSystem::FileSystem()
{
}

void FileSystem::begin()
{

    if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
    {
        Log.errorln(F("LittleFS mount failed"));
        return;
    }
    else
    {
        Log.noticeln(F("LittleFS mounted"));
    }
}

void FileSystem::readFile(const char *path, Stream *stream)
{
    Log.traceln(F("Reading file: %s\r\n"), path);

    File file = LittleFS.open(path);
    if (!file || file.isDirectory())
    {
        Log.errorln(F("- failed to open file for reading"));
        return;
    }

    Log.traceln(F("- read from file:"));
    
    while (file.available())
    {
        stream->write(file.read());
    }
    file.close();
}

void FileSystem::deleteFile(const char *path)
{
    Log.traceln(F("Deleting file and empty folders on path: %s\r\n"), path);

    if (LittleFS.remove(path))
    {
        Log.traceln(F("- file deleted"));
    }
    else
    {
        Log.errorln(F("- delete failed"));
    }

    char *pathStr = strdup(path);
    if (pathStr)
    {
        char *ptr = strrchr(pathStr, '/');
        if (ptr)
        {
            Log.traceln(F("Removing all empty folders on path: %s\r\n"), path);
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

void FileSystem::writeFile(const char *path, const char *contents)
{
    if (!LittleFS.exists(path))
    {
        if (strchr(path, '/'))
        {
            Log.traceln(F("Create missing folders of: %s\r\n"), path);
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

    Log.traceln(F("Writing file to: %s\r\n"), path);
    File file = LittleFS.open(path, FILE_WRITE);
    if (!file)
    {
        Log.errorln(F("- failed to open file for writing"));
        return;
    }
    if (file.print(contents))
    {
        Log.traceln(F("- file written"));
    }
    else
    {
        Log.errorln(F("- write failed"));
    }
    file.close();
}

FileSystem Files = FileSystem();