#ifndef TILE_H
#define TILE_H

#include "types.h"

typedef struct tile {
    uint8_t *ptr;
    size_t   bytes_per_row;
    void    *arg;
    rect     rect;
} tile;

#endif
