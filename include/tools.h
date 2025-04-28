#pragma once

#include "common.h"

/**
 * Init log file
 */
void initLogging();

/**
 * Write log message to log file
 */
void logMessage(const std::string& message);

/**
 * Start application
 */
void launchApplication(const char* appCommand);

/**
 * Handle window event
 */
void handleWindowEvent(Display* display, Window window);

/**
 * Scan existing windows
 */
void scanExistingWindows(Display* dpy, Window root);