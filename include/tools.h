#pragma once

#include "common.h"

void initLogging();

void logMessage(const std::string& message);

void launchApplication(const char* appCommand);

void handleWindowEvent(Display* display, Window window);

void scanExistingWindows(Display* dpy, Window root);