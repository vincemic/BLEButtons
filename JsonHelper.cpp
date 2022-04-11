#include "JsonHelper.h"


void jsonMerge(SpiRamJsonDocument &destJsonDocument, const SpiRamJsonDocument &srcJsonDocument, String elementName)
{
    String sourceJson;
    serializeJson(srcJsonDocument, sourceJson);

    if (sourceJson.length() > 0)
    {
        destJsonDocument[elementName.c_str()] = "$$$PLACEHOLDER";
        String destinationJson;
        serializeJson(destJsonDocument, destinationJson);

        destinationJson.replace(F("\"$$$PLACEHOLDER\""), sourceJson);

        deserializeJson(destJsonDocument, destinationJson);
    }
}
