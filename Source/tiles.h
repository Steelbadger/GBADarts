#ifndef TILES_H
#define TILES_H

#include <stdint.h>


extern const uint8_t blank_tile[64];
extern const uint8_t horiz_border_tile[64];
extern const uint8_t vertical_border_tile[64];
extern const uint8_t corner_border_tile[64];
extern const uint8_t window_background_tile[64];
extern const uint8_t vertical_divider_tile[64];
extern const uint8_t horizontal_divider_tile[64];
extern const uint8_t minidart_tile[64];

extern const unsigned char BGTiles[13696];
extern const unsigned char CursorTiles[4096];
extern const unsigned char DartTiles[1024];

extern const unsigned short BGPalette[256];
extern const unsigned short DartPal[3];
extern const unsigned short CursorPal[3];

#endif
