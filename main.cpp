#include "include/common.h"
#include "include/Daemon.h"
#include "include/XServerConnection.h"
#include "include/tools.h"


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
        XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask | KeyPressMask);
        XGrabKeyboard(display, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);

        logMessage("Subscription to key and window events successfully completed");

        while (true) {
            XEvent event;
            XNextEvent(display, &event);

            if (event.type == MapRequest) {
                Window w = event.xmaprequest.window;
                handleWindowEvent(display, w);
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
