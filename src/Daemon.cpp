#include "../include/common.h"
#include "../include/Daemon.h"

Daemon::Daemon() {
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