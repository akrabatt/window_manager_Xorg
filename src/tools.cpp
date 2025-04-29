#include "../include/common.h"
#include "../include/tools.h"

std::ofstream LogFile;

void initLogging()
{
    LogFile.open("/tmp/my_window_manager.log", std::ios::out | std::ios::app);
    if (!LogFile.is_open()) {
        throw std::runtime_error("Error open log file");
    }
}

void logMessage(const std::string& message)
{
    if(LogFile.is_open())
    {
        LogFile << message << std::endl;
    }
}

void launchApplication(const char* appCommand)
{
    logMessage("Launching application: " + std::string(appCommand));

    pid_t pid = fork();
    if(pid < 0)
    {
        logMessage("Error start application: " + std::string(strerror(errno)));
        return;
    }

    if(pid == 0)
    {
        setenv("PATH", "/usr/bin:/bin:/usr/sbin:/sbin", 1);
        execl("/bin/sh", "sh", "-c", appCommand, (char*)nullptr);
        logMessage("Error to execute execl: " + std::string(strerror(errno)));
        exit(EXIT_FAILURE);
    } 
    // else {
    //     int status;
    //     waitpid(pid, &status, 0);
    //     if(WIFEXITED(status))
    //     {
    //         logMessage("Application complited with code: " + std::to_string(WEXITSTATUS(status)));
    //     }
    // }
}

void handleWindowEvent(Display* display, Window window)
{
    XWindowAttributes attrs;
    XGetWindowAttributes(display, window, &attrs);
    if (attrs.override_redirect)
    {
        return;
    }

    // create red color
    Colormap colormap = DefaultColormap(display, DefaultScreen(display));
    XColor redColor, exactColor;
    if (!XAllocNamedColor(display, colormap, "red", &redColor, &exactColor))
    {
        logMessage("Failed to allocate red color, fallback to black");
        redColor.pixel = BlackPixel(display, DefaultScreen(display));
    }

    int border_width = 4;
    int frame_x = 100;
    int frame_y = 100;
    unsigned int client_width = attrs.width;
    unsigned int client_height = attrs.height;

    if (client_width < 100) client_width = 600;
    if (client_height < 100) client_height = 400;

    int frame_width = client_width + border_width * 2;
    int frame_height = client_height + border_width * 2;

    // create window-frame
    Window frame = XCreateSimpleWindow(
        display,
        DefaultRootWindow(display),
        frame_x, frame_y,
        frame_width, frame_height,
        border_width,
        redColor.pixel,
        WhitePixel(display, DefaultScreen(display))
    );

    XSelectInput(display, frame,
        SubstructureRedirectMask |
        SubstructureNotifyMask |
        ButtonPressMask |
        ButtonReleaseMask |
        PointerMotionMask
    );
    

    // reparent the client window to the frame
    XReparentWindow(display, window, frame, border_width, border_width);

    XMapWindow(display, frame);
    XMapWindow(display, window);

    XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime);

    logMessage("Framed and mapped window");
}


void scanExistingWindows(Display* dpy, Window root)
{
    Window parent, *kids;
    unsigned int nkids;

    if(!XQueryTree(dpy, root, &root, &parent, &kids, &nkids)) { return; } 

    for(unsigned i = 0; i < nkids; i++)
    {
        XWindowAttributes wa;
        if(XGetWindowAttributes(dpy, kids[i], &wa) && wa.map_state == IsViewable && !wa.override_redirect)
        {
            handleWindowEvent(dpy, kids[i]);
        }
    }
    XFree(kids);
}