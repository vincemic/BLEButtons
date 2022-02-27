#pragma once

class WifiHandlerClass
{

public:
    WifiHandlerClass();
    bool connect();
    void disconnect();
    void tick();
};

extern WifiHandlerClass WifiHandler;