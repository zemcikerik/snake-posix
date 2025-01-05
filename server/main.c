#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "argument_parser.h"
#include "server.h"
#include "../libargs/args.h"
#include "../libsnake/rng.h"

#define DELAY_BETWEEN_TICKS_MS 350

atomic_bool interrupted = false;

long long get_current_time_in_ms() {
    struct timeval val;
    gettimeofday(&val, NULL);
    return val.tv_sec * 1000 + val.tv_usec / 1000;
}

void interrupt_received(const int) {
    interrupted = true;
}

int main(int argc, char* argv[]) {
    game_settings_t* settings = argument_parser_parse(argc, argv);

    if (!settings) {
        return 1;
    }

    init_rng();
    server_t server;
    server_init(&server, settings);
    signal(SIGINT, interrupt_received);

    if (settings->parent_ != SETTING_NO_PARENT) {
        kill(settings->parent_, SIG_LAUNCHED);
    }

    bool game_running = false;
    bool game_ticked_at_least_once = false;
    long long last_tick = get_current_time_in_ms();

    do {
        while (last_tick + DELAY_BETWEEN_TICKS_MS > get_current_time_in_ms() && !interrupted) {
            usleep(5000);
        }

        if (interrupted) {
            break; // todo
        }

        last_tick = get_current_time_in_ms();
        game_running = server_tick(&server);

        if (game_running) {
            game_ticked_at_least_once = true;
        }
    } while (game_running || !game_ticked_at_least_once);

    server_end_game(&server);
    sleep(2);
    server_destroy(&server);
    argument_parser_free(settings);
    return 0;
}
