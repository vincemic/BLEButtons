#pragma once
#include <ArduinoJson.h>
#include <ArduinoLog.h>

void jsonMerge(JsonDocument &destJsonDocument, const JsonDocument &srcJsonDocument, String elementName);

struct SpiRamAllocator
{
    void *allocate(size_t size)
    {
        void *heap = malloc(size);
        
        if (heap == NULL)
            Log.errorln(F("JsonHelper] Failed to allocate SPIRAM"));

        return heap;
    }

    void deallocate(void *pointer)
    {
        free(pointer);
    }

    void *reallocate(void *ptr, size_t new_size)
    {
        return realloc(ptr, new_size);
    }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;