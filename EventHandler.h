#pragma once

class EventHandlerClass
{
public:
    EventHandlerClass();
    void process(const char *label);

};

extern EventHandlerClass EventHandler;