#pragma once

#include "common.h"

class XServerConnection
{
private:
    Display* display;
public:
    XServerConnection();
    ~XServerConnection();
    Display* get() const;
};