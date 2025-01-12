#include "server_launcher.h"
#include <stdatomic.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../libsnake/constants.h"

atomic_bool launched_signal_received = false;

void server_launcher_on_launched_signal_received(const int) {
    launched_signal_received = true;
}

void server_launcher_init_sig() {
    signal(SIG_LAUNCHED, server_launcher_on_launched_signal_received);
}

bool server_launcher_launch_server_process(game_settings_t* settings) {
    launched_signal_received = false;
    settings->parent_ = getpid();
    const pid_t pid = fork();

    if (pid == 0) {
        size_t index = 0;
        char* args[13];

#define DECLARE_INT_ARG_BUFFER(name) char name##_buffer[16]
#define PUSH_INT_ARG(name, flag, arg)   \
        sprintf(name##_buffer, "%d", (int) arg);    \
        args[index++] = flag;   \
        args[index++] = name##_buffer;

        args[index++] = "./server";

        if (settings->room_name_ != NULL) {
            args[index++] = "-n";
            args[index++] = settings->room_name_;
        }

        DECLARE_INT_ARG_BUFFER(port);
        DECLARE_INT_ARG_BUFFER(width);
        DECLARE_INT_ARG_BUFFER(height);

        if (settings->port_ != SETTING_NO_PORT) {
            const int port = (int) settings->port_;
            PUSH_INT_ARG(port, "-p", port);
        }

        if (settings->map_path_) {
            args[index++] = "-m";
            args[index++] = settings->map_path_;
        } else {
            PUSH_INT_ARG(width, "-w", settings->width_);
            PUSH_INT_ARG(height, "-h", settings->height_);
        }

        DECLARE_INT_ARG_BUFFER(parent);
        PUSH_INT_ARG(parent, "-a", settings->parent_);

        args[index] = NULL;
        execv("./server", args);

        perror("Failed to launch server process");
        exit(EXIT_FAILURE);
    }

    sleep(CLIENT_WAIT_FOR_SERVER_SECONDS);

    if (launched_signal_received) {
        return true;
    }

    kill(pid, SIGKILL);
    return false;
}
