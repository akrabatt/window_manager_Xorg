#include "include/common.h"
#include "include/Daemon.h"
#include "include/XServerConnection.h"
#include "include/tools.h"


Window moving_window = 0;
int start_root_x = 0, start_root_y = 0;
int win_start_x = 0, win_start_y = 0;

int main() {
    try {
        const char* displayEnv = getenv("DISPLAY");
        if (!displayEnv || std::string(displayEnv).empty())
            throw std::runtime_error("variable DISPLAY is not set");
        // Daemon daemon;

        initLogging();
        logMessage(std::string("DISPLAY: ") + displayEnv);
        logMessage("Starting window manager...");

        XServerConnection xconn;
        Display* display = xconn.get();
        Window root = DefaultRootWindow(display);

        // subscribe to events on root window
        XSelectInput(display, root,
            SubstructureRedirectMask |
            SubstructureNotifyMask   |
            KeyPressMask            |
            ButtonPressMask);
        XGrabKeyboard(display, root, True, GrabModeAsync, GrabModeAsync, CurrentTime);

        logMessage("Subscribed to root events");

        // capture existing windows
        scanExistingWindows(display, root);

        // autostart applications
        const char* autostart[] = {
            // "xclock", 
            "xterm", 
            nullptr
        };
        for (const char** cmd = autostart; *cmd; ++cmd) {
            launchApplication(*cmd);
        }

        logMessage("Autostart applications launched");

        
        while (true) {
            XEvent ev;
            XNextEvent(display, &ev);
            switch (ev.type) {
            case MapRequest: {
                Window w = ev.xmaprequest.window;
                handleWindowEvent(display, w);
                break;
            }
            case KeyPress: {
                KeySym key = XLookupKeysym(&ev.xkey, 0);
                logMessage(std::string("Key pressed: ") + XKeysymToString(key));
                if (key == XK_F1) {
                    launchApplication("xterm");
                } else if (key == XK_F2) {
                    launchApplication("gedit");
                }
                break;
            }
            case ButtonPress: {
                XButtonEvent* be = &ev.xbutton;
                XWindowAttributes attr;
                XGetWindowAttributes(display, be->window, &attr);
            
                moving_window = be->window;
            
                start_root_x = be->x_root;
                start_root_y = be->y_root;
            
                win_start_x = attr.x;
                win_start_y = attr.y;
            
                logMessage("Started moving window");
                break;
            }
            
            case MotionNotify: {
                if (moving_window != 0) {
                    XMotionEvent* me = &ev.xmotion;
            
                    int dx = me->x_root - start_root_x;
                    int dy = me->y_root - start_root_y;
            
                    XMoveWindow(display, moving_window,
                        win_start_x + dx,
                        win_start_y + dy);
            
                    logMessage("Moving window");
                }
                break;
            }
            
            case ButtonRelease: {
                moving_window = 0;
                logMessage("Stopped moving window");
                break;
            }
                       
            case ConfigureRequest: {
                XConfigureRequestEvent* cre = &ev.xconfigurerequest;

                XWindowChanges changes;
                changes.x = cre->x;
                changes.y = cre->y;
                changes.width = cre->width;
                changes.height = cre->height;
                changes.border_width = cre->border_width;
                changes.sibling = cre->above;
                changes.stack_mode = cre->detail;

                XConfigureWindow(display, cre->window, cre->value_mask, &changes);
                logMessage("Handled ConfigureRequest event");
                break;
            }
            default:
                break;
            }
        }
    } catch (const std::exception& e) {
        logMessage(std::string("Error: ") + e.what());
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}