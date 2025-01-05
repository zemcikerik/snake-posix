#include "shm_game_state.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "constants.h"

char* allocate_and_format_name(const char* room_name) {
    char* buffer = malloc(SHM_NAME_BUFFER_SIZE);
    sprintf(buffer, SHM_NAME_FORMAT, room_name);
    return buffer;
}

void shm_close_mapped_memory(shm_game_state_t* self) {
    if (munmap(self->game_state_, sizeof(game_state_t)) == -1) {
        perror("Failed to unmap shared memory");
        exit(EXIT_FAILURE);
    }

    if (close(self->fd_) == -1) {
        perror("Failed to close shared memory");
        exit(EXIT_FAILURE);
    }
}

// todo maybe merge init logic
bool shm_game_state_init(shm_game_state_t* self, const char* room_name) {
    self->name_ = allocate_and_format_name(room_name);
    self->fd_ = shm_open(self->name_, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

    if (self->fd_ == -1) {
        free(self->name_);
        return false;
    }

    if (ftruncate(self->fd_, sizeof(game_state_t))) {
        perror("Failed to truncate shared memory");
        exit(EXIT_FAILURE);
    }

    self->game_state_ = mmap(NULL, sizeof(game_state_t), PROT_READ | PROT_WRITE, MAP_SHARED, self->fd_, 0);

    if (self->game_state_ == MAP_FAILED) {
        perror("Failed to map shared memory");
        exit(EXIT_FAILURE);
    }

    return true;
}

void shm_game_state_destroy(shm_game_state_t* self) {
    shm_close_mapped_memory(self);

    if (shm_unlink(self->name_) == -1) {
        perror("Failed to unlink shared memory");
        exit(EXIT_FAILURE);
    }

    free(self->name_);
}

bool shm_game_state_open(shm_game_state_t* self, const char* room_name) {
    self->name_ = allocate_and_format_name(room_name);
    self->fd_ = shm_open(self->name_, O_RDWR, 0);

    if (self->fd_ == -1) {
        free(self->name_);
        return false;
    }

    self->game_state_ = mmap(NULL, sizeof(game_state_t), PROT_READ | PROT_WRITE,MAP_SHARED, self->fd_, 0);

    if (self->game_state_ == MAP_FAILED) {
        perror("Failed to map shared memory");
        exit(EXIT_FAILURE);
    }

    return true;
}

void shm_game_state_close(shm_game_state_t* self) {
    shm_close_mapped_memory(self);
    free(self->name_);
}

game_state_t* shm_game_state_get(shm_game_state_t* self) {
    return self->game_state_;
}
