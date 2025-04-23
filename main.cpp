#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <signal.h>
#include <stdexcept>
#include <memory>
#include <sys/wait.h>
#include <cstring>

std::ofstream logFile;

void initLogging() {
    logFile.open("/tmp/my_window_manager.log", std::ios::out | std::ios::app);
    if (!logFile.is_open()) {
        throw std::runtime_error("Error open log file");
    }
}

void logMessage(const std::string& message) {
    if (logFile.is_open()) {
        logFile << message << std::endl;
    }
}

class Daemon {
public:
    Daemon() {
        pid_t pid = fork();
        if (pid < 0) {
            throw std::runtime_error("Error start daemon");
        }
        if (pid > 0) {
            exit(EXIT_SUCCESS);
        }

        if (setsid() < 0) {
            throw std::runtime_error("Error create new session");
        }

        if (chdir("/") != 0) {
            throw std::runtime_error("Error change work directory");
        }

        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
    }
};

class XServerConnection {
private:
    Display* display;

public:
    XServerConnection() : display(XOpenDisplay(nullptr)) {
        if (!display) {
            throw std::runtime_error("Error connecting to X server");
        }
    }

    ~XServerConnection() {
        if (display) {
            XCloseDisplay(display);
        }
    }

    Display* get() const {
        return display;
    }
};

void launchApplication(const char* appCommand) {
    logMessage("Launching application: " + std::string(appCommand));
    pid_t pid = fork();
    if (pid < 0) {
        logMessage("Error start application: " + std::string(strerror(errno)));
        return;
    }

    if (pid == 0) {
        setenv("PATH", "/usr/bin:/bin:/usr/sbin:/sbin", 1);
        execl("/bin/sh", "sh", "-c", appCommand, (char*)nullptr);
        logMessage("Error to execute execl: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            logMessage("Application complited with code: " + std::to_string(WEXITSTATUS(status)));
        }
    }
}

void handleWindowEvent(Display* display, Window window) {
    XWindowAttributes attrs;
    XGetWindowAttributes(display, window, &attrs);

    if (attrs.map_state != IsViewable) {
        logMessage("Window is not viewable, ingnoring");
        return;
    }

    XSetWindowAttributes newAttrs;
    newAttrs.border_pixel = WhitePixel(display, DefaultScreen(display));
    newAttrs.background_pixel = BlackPixel(display, DefaultScreen(display));

    XChangeWindowAttributes(display, window, CWBorderPixel | CWBackPixel, &newAttrs);
    XSetWindowBorderWidth(display, window, 5);
    XMapWindow(display, window);
    XSetInputFocus(display, window, RevertToParent, CurrentTime);

    logMessage("Parsed window create ivent");
}

int main() {
    try {
        const char* displayEnv = getenv("DISPLAY");
        if (!displayEnv || std::string(displayEnv).empty()) {
            throw std::runtime_error("variable DISPLAY is not set");
        }
        logMessage("DISPLAY: " + std::string(displayEnv));

        Daemon daemon;

        initLogging();
        logMessage("Starting window manager...");

        setenv("DISPLAY", displayEnv, 1);

        XServerConnection xConnection;
        Display* display = xConnection.get();

        Window root = DefaultRootWindow(display);
        XSelectInput(display, root, SubstructureNotifyMask | KeyPressMask);
        XGrabKeyboard(display, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);

        logMessage("Subscription to key and window events successfully completed");

        while (true) {
            XEvent event;
            XNextEvent(display, &event);

            if (event.type == MapNotify) {
                handleWindowEvent(display, event.xmap.window);
            } else if (event.type == KeyPress) {
                KeySym key = XLookupKeysym(&event.xkey, 0);
                logMessage("Key pressed: " + std::to_string(key) + " (" + XKeysymToString(key) + ")");
                if (key == XK_F1) {
                    launchApplication("xterm");
                } else if (key == XK_F2) {
                    launchApplication("gedit");
                } else {
                    logMessage("Key isn't supported: " + std::string(XKeysymToString(key)));
                }
            } else {
                logMessage("Error: " + std::to_string(event.type));
            }
        }
    } catch (const std::exception& e) {
        logMessage("Error: " + std::string(e.what()));
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
