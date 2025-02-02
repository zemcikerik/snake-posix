#ifndef INCLUDE_LIBSNAKE_SOCKET_MESSAGE_H
#define INCLUDE_LIBSNAKE_SOCKET_MESSAGE_H

#include <stdbool.h>
#include "../map/map.h"
#include "socket.h"

typedef enum socket_message_type_t {
  SOCKET_MESSAGE_MAGIC_BYTES,
  SOCKET_MESSAGE_GAME_FULL,
  SOCKET_MESSAGE_CONNECTED,
  SOCKET_MESSAGE_MAP,
  SOCKET_MESSAGE_TILE_CHANGE,
  SOCKET_MESSAGE_PLAYER_STATE,
  SOCKET_MESSAGE_DIRECTION,
  SOCKET_MESSAGE_DISCONNECT,
  SOCKET_MESSAGE_GAME_END,
  SOCKET_MESSAGE_PAUSE,
  SOCKET_MESSAGE_UNPAUSE,
  SOCKET_MESSAGE_RESPAWN,
} socket_message_type_t;

typedef struct socket_message_magic_bytes_data_t {
  char bytes_[4];
} socket_message_magic_bytes_data_t;

typedef struct socket_message_map_data_t {
  coordinate_t width_;
  coordinate_t height_;
  map_tile_t* tiles_;
} socket_message_map_data_t;

typedef struct socket_message_tile_change_t {
  coordinate_t row_;
  coordinate_t column_;
  map_tile_t tile_;
} socket_message_tile_change_t;

typedef struct socket_message_player_state_t {
  player_status_t status_;
  direction_t direction_;
} socket_message_player_state_t;

typedef struct socket_message_direction_t {
  direction_t direction_;
} socket_message_direction_t;

typedef union socket_message_data_t {
  socket_message_magic_bytes_data_t* magic_bytes_;
  socket_message_map_data_t* map_;
  socket_message_tile_change_t* tile_change_;
  socket_message_player_state_t* player_state_;
  socket_message_direction_t* direction_;
} socket_message_data_t;

typedef struct socket_message_t {
  socket_message_type_t type_;
  socket_message_data_t data_;
} socket_message_t;

void socket_message_init_magic_bytes(socket_message_t* self, const char* magic_bytes);
void socket_message_init_game_full(socket_message_t* self);
void socket_message_init_connected(socket_message_t* self);
void socket_message_init_map(socket_message_t* self, const map_t* map);
void socket_message_init_tile_change(socket_message_t* self, coordinate_t row, coordinate_t column, map_tile_t tile);
void socket_message_init_player_state(socket_message_t* self, player_status_t status, direction_t direction);
void socket_message_init_direction(socket_message_t* self, direction_t direction);
void socket_message_init_disconnect(socket_message_t* self);
void socket_message_init_game_end(socket_message_t* self);
void socket_message_init_pause(socket_message_t* self);
void socket_message_init_unpause(socket_message_t* self);
void socket_message_init_respawn(socket_message_t* self);

char* socket_message_serialize(const socket_message_t* self, size_t* out_length);
bool socket_message_serialize_to_socket(const socket_message_t* self, socket_t* socket);
bool socket_message_deserialize_from_socket(socket_t* socket, socket_message_t* out_message);

void socket_message_deserialize_to_map(const socket_message_t* self, map_t* map);

void socket_message_destroy(socket_message_t* self);
void socket_message_destroy_buffer(char* buffer);

#endif
