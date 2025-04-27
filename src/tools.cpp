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
    } else {
        int status;
        waitpid(pid, &status, 0);
        if(WIFEXITED(status))
        {
            logMessage("Application complited with code: " + std::to_string(WEXITSTATUS(status)));
        }
    }
}

void handleWindowEvent(Display* display, Window window)
{
    XWindowAttributes attrs;
    XGetWindowAttributes(display, window, &attrs);
    if(attrs.map_state != IsViewable)
    {
        logMessage("Window isn't viewable, ignoring");
        return;
    }

    XSetWindowAttributes newAttrs;
    newAttrs.border_pixel = WhitePixel(display, DefaultScreen(display));
    newAttrs.background_pixel = BlackPixel(display, DefaultScreen(display));

    XChangeWindowAttributes(display, window, CWBorderPixel | CWBackPixel, &newAttrs);
    XSetWindowBorder(display, window, 5);\
    XMapWindow(display, window);
    XSetInputFocus(display, window, RevertToPointerRoot, CurrentTime);

    logMessage("Window event handled");
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