#ifndef INCLUDE_LIBSNAKE_CONSTANTS_H
#define INCLUDE_LIBSNAKE_CONSTANTS_H

#include <signal.h>

#define MAX_PLAYERS 64

#define MIN_MAP_WIDTH 10
#define MIN_MAP_HEIGHT 10
#define MAX_MAP_WIDTH 100
#define MAX_MAP_HEIGHT 50

#define MAX_CHANGELOG_SIZE (MAX_MAP_WIDTH * MAX_MAP_HEIGHT + MAX_PLAYERS)

#define SHM_NAME_BUFFER_SIZE 256
#define SHM_NAME_FORMAT "/shm-zemcik-snake-%s"

#define SIG_LAUNCHED SIGUSR1
#define SIG_DESTROYED SIGUSR2

#define GAME_MAGIC_BYTES_INITIALIZER { 42, 37, 13, 31 };
#define GAME_MAGIC_BYTES_CORRECT(bytes) (bytes[0] == 42 && bytes[1] == 37 && bytes[2] == 13 && bytes[3] == 31)

#define CLIENT_WAIT_FOR_SERVER_SECONDS 3

#endif
