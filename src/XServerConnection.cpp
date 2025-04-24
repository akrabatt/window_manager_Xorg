#include "../include/common.h"
#include "../include/XServerConnection.h"

XServerConnection::XServerConnection() : display(XOpenDisplay(nullptr))
{
    if(!display)
    {
        throw std::runtime_error("Error connecting to X server");
    }
}

XServerConnection::~XServerConnection()
{
    if(display)
    {
        XCloseDisplay(display);
    }
}

Display* XServerConnection::get() const
{
    return display;
}