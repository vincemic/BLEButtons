#pragma once
#include <ArduinoJson.h>
#include <ArduinoLog.h>



struct SpiRamAllocator
{
    void *allocate(size_t size)
    {
        void *heap = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
        
        if (heap == NULL)
            Log.errorln(F("JsonHelper] Failed to allocate SPIRAM"));

        return heap;
    }

    void deallocate(void *pointer)
    {
        heap_caps_free(pointer);
    }

    void *reallocate(void *ptr, size_t new_size)
    {
        return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
    }
};

using SpiRamJsonDocument = BasicJsonDocument<SpiRamAllocator>;

void jsonMerge(SpiRamJsonDocument &destJsonDocument, const SpiRamJsonDocument &srcJsonDocument, String elementName);