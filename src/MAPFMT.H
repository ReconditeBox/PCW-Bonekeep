#ifndef MAPFMT_H
#define MAPFMT_H

/*
 * BONEKEEP map-format contract.
 *
 * Shared by MEDIT, MENCODE and MDECODE.
 *
 * Encoded level header, 8 bytes:
 *   0-3  "BKM1"
 *   4    map width
 *   5    map height
 *   6    number of tile IDs (32)
 *   7    flags, currently zero
 *
 * Header is followed by width * height tile bytes,
 * stored row by row. Tile values are 0..31.
 */

#define BK_MAP_MAGIC0       'B'
#define BK_MAP_MAGIC1       'K'
#define BK_MAP_MAGIC2       'M'
#define BK_MAP_MAGIC3       '1'

#define BK_MAP_HEADER_SIZE  8
#define BK_TILE_TYPES       32

#define BK_MAP_W            75
#define BK_MAP_H            24


/*
 * Raw text-map characters.
 *
 * These are the authoritative source-map symbols used
 * by MEDIT, MENCODE and MDECODE.
 */

#define BK_C_VOID           ' '
#define BK_C_WALL           '#'
#define BK_C_FLOOR          '.'
#define BK_C_START          '@'
#define BK_C_DOWN           '>'
#define BK_C_UP             '<'
#define BK_C_GOLD           '$'
#define BK_C_KEY            'k'
#define BK_C_DOOR_CLOSED    '+'
#define BK_C_DOOR_LOCKED    'L'
#define BK_C_DOOR_OPEN      '/'
#define BK_C_SKELETON       'S'
#define BK_C_ZOMBIE         'Z'
#define BK_C_BONES          'x'
#define BK_C_SWORD          '!'
#define BK_C_SHIELD         '*'
#define BK_C_ARMOUR         'A'
#define BK_C_POTION         'p'
#define BK_C_FOOD           'f'
#define BK_C_COFFIN         'C'
#define BK_C_EXIT           'X'


/*
 * Encoded tile IDs.
 *
 * @ and X are only valid on LEVEL.000.
 * > goes down: 000 -> 001 -> ... -> 999 -> 000.
 * < goes up:   000 -> 999 -> ... -> 001 -> 000.
 */

#define BK_T_VOID            0
#define BK_T_WALL            1
#define BK_T_FLOOR           2
#define BK_T_START           3
#define BK_T_DOWN            4
#define BK_T_UP              5
#define BK_T_GOLD            6
#define BK_T_KEY             7
#define BK_T_DOOR_CLOSED     8
#define BK_T_DOOR_LOCKED     9
#define BK_T_DOOR_OPEN      10
#define BK_T_SKELETON       12
#define BK_T_ZOMBIE         13
#define BK_T_BONES          14
#define BK_T_SWORD          15
#define BK_T_SHIELD         16
#define BK_T_ARMOUR         17
#define BK_T_POTION         18
#define BK_T_FOOD           19
#define BK_T_EXIT           21
#define BK_T_COFFIN         22

/* Tile IDs 11, 20, 23 and 24..31 are reserved. */

#endif
