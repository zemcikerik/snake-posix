#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include "server.h"
#include "../libargs/args.h"
#include "../libsnake/rng.h"

#define DELAY_BETWEEN_TICKS_MS 500

long long get_current_time_in_ms() {
    struct timeval val;
    gettimeofday(&val, NULL);
    return val.tv_sec * 1000 + val.tv_usec / 1000;
}

int main(int argc, char* argv[]) {
    ArgParser* parser = ap_new_parser();

    ap_set_helptext(parser, "Hosts a snake game.");
    ap_add_int_opt(parser, "parent a", 0);
    ap_add_str_opt(parser, "name n", "");
    ap_add_str_opt(parser, "map m", "");
    ap_add_int_opt(parser, "width w", 0);
    ap_add_int_opt(parser, "height h", 0);

    if (!ap_parse(parser, argc, argv)) {
        ap_free(parser);
        return 1;
    }

    if (!ap_found(parser, "name")) {
        fprintf(stderr, "IPC interface missing\n");
        ap_free(parser);
        return 2;
    }

    map_settings_t map_settings;
    const char* name = ap_found(parser, "name") ? ap_get_str_value(parser, "name") : NULL;

    if (ap_found(parser, "map")) {
        map_settings.path_ = ap_get_str_value(parser, "map");
        map_settings.has_path_ = true;
    } else if (ap_found(parser, "width") && ap_found(parser, "height")) {
        map_settings.width_ = ap_get_int_value(parser, "width");
        map_settings.height_ = ap_get_int_value(parser, "height");
        map_settings.has_path_ = false;
    } else {
        fprintf(stderr, "Map settings are missing\n");
        ap_free(parser);
        return 3;
    }

    init_rng();
    server_t server;
    server_init(&server, map_settings, name);

    if (ap_found(parser, "parent")) {
        kill(ap_get_int_value(parser, "parent"), SIG_LAUNCHED);
    }

    long long last_tick = get_current_time_in_ms();
    bool game_running = false;
    bool game_ticked_at_least_once = false;

    do {
        while (last_tick + DELAY_BETWEEN_TICKS_MS > get_current_time_in_ms()) {
            usleep(5000);
        }

        last_tick = get_current_time_in_ms();
        game_running = server_tick(&server);

        if (game_running) {
            game_ticked_at_least_once = true;
        }
    } while (game_running || !game_ticked_at_least_once);

    server_destroy(&server);
    ap_free(parser);
    return 0;
}
