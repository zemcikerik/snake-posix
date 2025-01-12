#include "socket_message.h"

#include <stdlib.h>
#include <string.h>
#include <endian.h>

void socket_message_init_magic_bytes(socket_message_t* self, const char* magic_bytes) {
    self->type_ = SOCKET_MESSAGE_MAGIC_BYTES;
    self->data_.magic_bytes_ = malloc(sizeof(socket_message_magic_bytes_data_t));
    memcpy(self->data_.magic_bytes_->bytes, magic_bytes, sizeof(char[4]));
}

void socket_message_init_game_full(socket_message_t* self) {
    self->type_ = SOCKET_MESSAGE_GAME_FULL;
}

void socket_message_init_connected(socket_message_t* self) {
    self->type_ = SOCKET_MESSAGE_CONNECTED;
}

void socket_message_init_map(socket_message_t* self, const map_t* map) {
    self->type_ = SOCKET_MESSAGE_MAP;

    socket_message_map_data_t* map_data = malloc(sizeof(socket_message_map_data_t));
    self->data_.map_ = map_data;

    map_data->width_ = map_get_width(map);
    map_data->height_ = map_get_height(map);
    map_data->tiles_ = malloc(map_data->width_ * map_data->height_ * sizeof(map_tile_t));

    size_t index = 0;
    for (coordinate_t i = 0; i < map_data->height_; ++i) {
        for (coordinate_t j = 0; j < map_data->width_; ++j) {
            const coordinates_t coordinates = { i, j };
            map_data->tiles_[index++] = map_get_tile_state(map, coordinates);
        }
    }
}

void socket_message_init_tile_change(socket_message_t* self, const coordinate_t row, const coordinate_t column, const map_tile_t tile) {
    self->type_ = SOCKET_MESSAGE_TILE_CHANGE;
    self->data_.tile_change_ = malloc(sizeof(socket_message_tile_change_t));
    self->data_.tile_change_->row_ = row;
    self->data_.tile_change_->column_ = column;
    self->data_.tile_change_->tile_ = tile;
}

void socket_message_init_player_state(socket_message_t* self, const player_status_t status, const direction_t direction) {
    self->type_ = SOCKET_MESSAGE_PLAYER_STATE;
    self->data_.player_state_ = malloc(sizeof(socket_message_player_state_t));
    self->data_.player_state_->status_ = status;
    self->data_.player_state_->direction_ = direction;
}

void socket_message_init_direction(socket_message_t* self, const direction_t direction) {
    self->type_ = SOCKET_MESSAGE_DIRECTION;
    self->data_.direction_ = malloc(sizeof(socket_message_direction_t));
    self->data_.direction_->direction_ = direction;
}


void socket_message_init_disconnect(socket_message_t* self) {
    self->type_ = SOCKET_MESSAGE_DISCONNECT;
}

void socket_message_init_game_end(socket_message_t* self) {
    self->type_ = SOCKET_MESSAGE_GAME_END;
}

void socket_message_init_pause(socket_message_t* self) {
    self->type_ = SOCKET_MESSAGE_PAUSE;
}

void socket_message_init_unpause(socket_message_t* self) {
    self->type_ = SOCKET_MESSAGE_UNPAUSE;
}

void socket_message_init_respawn(socket_message_t* self) {
    self->type_ = SOCKET_MESSAGE_RESPAWN;
}

size_t socket_message_calculate_data_buffer_size(const socket_message_t* self) {
    if (self->type_ == SOCKET_MESSAGE_MAGIC_BYTES) {
        return sizeof(char[4]);
    }

    if (self->type_ == SOCKET_MESSAGE_MAP) {
        const size_t dimensions_size = 2 * sizeof(coordinate_t);
        const size_t tile_count = self->data_.map_->width_ * self->data_.map_->height_;
        const size_t size_per_tile = sizeof(size_t) + 1 + sizeof(player_id_t);

        return dimensions_size + tile_count * size_per_tile;
    }

    if (self->type_ == SOCKET_MESSAGE_TILE_CHANGE) {
        const size_t dimensions_size = 2 * sizeof(coordinate_t);
        const size_t tile_size = sizeof(size_t) + 1 + sizeof(player_id_t);
        return dimensions_size + tile_size;
    }

    if (self->type_ == SOCKET_MESSAGE_PLAYER_STATE) {
        return 2;
    }

    if (self->type_ == SOCKET_MESSAGE_DIRECTION) {
        return 1;
    }

    return 0;
}

void socket_message_serialize_magic_bytes(const socket_message_magic_bytes_data_t* data, char* buffer) {
    memcpy(buffer, data->bytes, 4);
}

void socket_message_serialize_connected(const socket_message_connected_data_t* data, char* buffer) {
    const uint16_t player_id_reordered = htole16(data->player_id_);
    memcpy(buffer, &player_id_reordered, sizeof(uint16_t));
    buffer[sizeof(uint16_t)] = (char) data->direction_;
}

#define COPY_AND_MOVE_PTR(name)   \
    memcpy(buffer, &name, sizeof(name));   \
    buffer += sizeof(name);

char* socket_message_serialize_map_tile(const map_tile_t* tile, char* buffer) {
    const uint64_t order_reordered = htole64(tile->order_);
    COPY_AND_MOVE_PTR(order_reordered);

    *buffer = (char) tile->type_;
    buffer += 1;

    const uint16_t player_reordered = htole16(tile->player_);
    COPY_AND_MOVE_PTR(player_reordered);

    return buffer;
}

void socket_message_serialize_map(const socket_message_map_data_t* data, char* buffer) {
    const uint32_t width_reordered = htole32(data->width_);
    COPY_AND_MOVE_PTR(width_reordered);

    const uint32_t height_reordered = htole32(data->height_);
    COPY_AND_MOVE_PTR(height_reordered);

    const size_t length = data->width_ * data->height_;

    for (size_t i = 0; i < length; ++i) {
        buffer = socket_message_serialize_map_tile(&data->tiles_[i], buffer);
    }
}

void socket_message_serialize_tile_change(const socket_message_tile_change_t* data, char* buffer) {
    const uint32_t row_reordered = htole32(data->row_);
    COPY_AND_MOVE_PTR(row_reordered);

    const uint32_t column_reordered = htole32(data->column_);
    COPY_AND_MOVE_PTR(column_reordered);

    socket_message_serialize_map_tile(&data->tile_, buffer);
}

void socket_message_serialize_player_state(const socket_message_player_state_t* data, char* buffer) {
    buffer[0] = (char) data->status_;
    buffer[1] = (char) data->direction_;
}

void socket_message_serialize_direction(const socket_message_direction_t* data, char* buffer) {
    buffer[0] = (char) data->direction_;
}

char* socket_message_serialize(const socket_message_t* self, size_t* out_length) {
    *out_length = 1 + socket_message_calculate_data_buffer_size(self);
    char* buffer = malloc(*out_length);
    memset(buffer, 0x69, *out_length);
    buffer[0] = (char) self->type_;

    if (self->type_ == SOCKET_MESSAGE_MAGIC_BYTES) {
        socket_message_serialize_magic_bytes(self->data_.magic_bytes_, buffer + 1);
    } else if (self->type_ == SOCKET_MESSAGE_MAP) {
        socket_message_serialize_map(self->data_.map_, buffer + 1);
    } else if (self->type_ == SOCKET_MESSAGE_TILE_CHANGE) {
        socket_message_serialize_tile_change(self->data_.tile_change_, buffer + 1);
    } else if (self->type_ == SOCKET_MESSAGE_PLAYER_STATE) {
        socket_message_serialize_player_state(self->data_.player_state_, buffer + 1);
    } else if (self->type_ == SOCKET_MESSAGE_DIRECTION) {
        socket_message_serialize_direction(self->data_.direction_, buffer + 1);
    }

    return buffer;
}

bool socket_message_serialize_to_socket(const socket_message_t* self, socket_t* socket) {
    size_t buffer_length;
    char* buffer = socket_message_serialize(self, &buffer_length);

    const bool result = socket_write(socket, buffer, buffer_length);

    socket_message_destroy_buffer(buffer);
    return result;
}

bool socket_message_deserialize_magic_bytes_from_socket(socket_t* socket, socket_message_data_t* out_data) {
    socket_message_magic_bytes_data_t* data = malloc(sizeof(socket_message_magic_bytes_data_t));

    if (!socket_read(socket, data->bytes, 4)) {
        free(data);
        return false;
    }

    out_data->magic_bytes_ = data;
    return true;
}

char* socket_message_deserialize_map_tile(char* buffer, map_tile_t* out_tile) {
    out_tile->order_ = le64toh(*(uint64_t*) buffer);
    buffer += sizeof(uint64_t);

    out_tile->type_ = (map_tile_type_t) *buffer;
    buffer += 1;

    out_tile->player_ = le16toh(*(uint16_t*) buffer);
    buffer += sizeof(uint16_t);

    return buffer;
}

bool socket_message_deserialize_map_from_socket(socket_t* socket, socket_message_data_t* out_data) {
    char dimensions_buffer[2 * sizeof(uint32_t)];

    if (!socket_read(socket, dimensions_buffer, 2 * sizeof(uint32_t))) {
        return false;
    }

    const coordinate_t width = le32toh(*(uint32_t*) dimensions_buffer);
    const coordinate_t height = le32toh(*((uint32_t*) dimensions_buffer + 1));
    const size_t length = width * height;
    char* tiles_buffer = malloc(length * (sizeof(uint64_t) + 1 + sizeof(uint16_t)));

    if (!socket_read(socket, tiles_buffer, length * (sizeof(uint64_t) + 1 + sizeof(uint16_t)))) {
        free(tiles_buffer);
        return false;
    }

    char* current_tile_buffer = tiles_buffer;
    map_tile_t* tiles = malloc(sizeof(map_tile_t) * length);

    for (size_t i = 0; i < length; ++i) {
        current_tile_buffer = socket_message_deserialize_map_tile(current_tile_buffer, &tiles[i]);
    }

    free(tiles_buffer);

    socket_message_map_data_t* data = malloc(sizeof(socket_message_map_data_t));
    data->width_ = width;
    data->height_ = height;
    data->tiles_ = tiles;

    out_data->map_ = data;
    return true;
}

bool socket_message_deserialize_tile_change_from_socket(socket_t* socket, socket_message_data_t* out_data) {
#define TILE_CHANGE_BUFFER_LENGTH (2 * sizeof(uint32_t) + sizeof(uint64_t) + 1 + sizeof(uint16_t))
    char buffer[TILE_CHANGE_BUFFER_LENGTH];

    if (!socket_read(socket, buffer, TILE_CHANGE_BUFFER_LENGTH)) {
        return false;
    }

    socket_message_tile_change_t* data = malloc(sizeof(socket_message_tile_change_t));
    data->row_ = le32toh(*(uint32_t*) buffer);
    data->column_ = le32toh(*((uint32_t*) buffer + 1));
    socket_message_deserialize_map_tile(buffer + 2 * sizeof(uint32_t), &data->tile_);

    out_data->tile_change_ = data;
    return true;
}

bool socket_message_deserialize_player_state_from_socket(socket_t* socket, socket_message_data_t* out_data) {
    char buffer[2];
    if (!socket_read(socket, buffer, 2)) {
        return false;
    }

    socket_message_player_state_t* data = malloc(sizeof(socket_message_player_state_t));
    data->status_ = (player_status_t) buffer[0];
    data->direction_ = (direction_t) buffer[1];

    out_data->player_state_ = data;
    return true;
}

bool socket_message_deserialize_direction_from_socket(socket_t* socket, socket_message_data_t* out_data) {
    char raw_direction;
    if (!socket_read(socket, &raw_direction, 1)) {
        return false;
    }

    socket_message_direction_t* data = malloc(sizeof(socket_message_direction_t));
    data->direction_ = (direction_t) raw_direction;

    out_data->direction_ = data;
    return true;
}

bool socket_message_deserialize_from_socket(socket_t* socket, socket_message_t* out_message) {
    char raw_type;
    if (!socket_read(socket, &raw_type, 1)) {
        return false;
    }

    out_message->type_ = (socket_message_type_t) raw_type;

    if (out_message->type_ > SOCKET_MESSAGE_RESPAWN) {
        return false;
    }

    if (out_message->type_ == SOCKET_MESSAGE_MAGIC_BYTES) {
        return socket_message_deserialize_magic_bytes_from_socket(socket, &out_message->data_);
    }
    if (out_message->type_ == SOCKET_MESSAGE_MAP) {
        return socket_message_deserialize_map_from_socket(socket, &out_message->data_);
    }
    if (out_message->type_ == SOCKET_MESSAGE_TILE_CHANGE) {
        return socket_message_deserialize_tile_change_from_socket(socket, &out_message->data_);
    }
    if (out_message->type_ == SOCKET_MESSAGE_PLAYER_STATE) {
        return socket_message_deserialize_player_state_from_socket(socket, &out_message->data_);
    }
    if (out_message->type_ == SOCKET_MESSAGE_DIRECTION) {
        return socket_message_deserialize_direction_from_socket(socket, &out_message->data_);
    }

    return true;
}

void socket_message_deserialize_to_map(const socket_message_t* self, map_t* map) {
    if (self->type_ != SOCKET_MESSAGE_MAP) {
        return;
    }

    socket_message_map_data_t* data = self->data_.map_;
    map_init(map, data->width_, data->height_);

    size_t index = 0;
    for (size_t i = 0; i < data->height_; ++i) {
        for (size_t j = 0; j < data->width_; ++j) {
            coordinates_t coordinates = { i, j };
            map_set_tile_state(map, coordinates, data->tiles_[index++]);
        }
    }
}

void socket_message_destroy(socket_message_t* self) {
    if (self->type_ == SOCKET_MESSAGE_MAGIC_BYTES) {
        free(self->data_.magic_bytes_);
    } else if (self->type_ == SOCKET_MESSAGE_MAP) {
        free(self->data_.map_->tiles_);
        free(self->data_.map_);
    } else if (self->type_ == SOCKET_MESSAGE_TILE_CHANGE) {
        free(self->data_.tile_change_);
    } else if (self->type_ == SOCKET_MESSAGE_PLAYER_STATE) {
        free(self->data_.player_state_);
    } else if (self->type_ == SOCKET_MESSAGE_DIRECTION) {
        free(self->data_.direction_);
    }
}

void socket_message_destroy_buffer(char* buffer) {
    free(buffer);
}
