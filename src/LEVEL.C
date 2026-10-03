#include "game.h"


/*
 * BKM1 disk format values used by the game loader.
 * Kept local to LEVEL.C so the native Hi-Tech C 3.09
 * build does not depend on a second header here.
 */
#define MHSZ    8
#define MTYPES 32

#define TVOID   0
#define TWALL   1
#define TFLOOR  2
#define TSTART  3
#define TDOWN   4
#define TUP     5
#define TGOLD   6
#define TKEY    7
#define TCLOSE  8
#define TLOCK   9
#define TOPEN  10
#define TSKEL  12
#define TZOMB  13
#define TBONES 14
#define TSWORD 15
#define TSHIELD 16
#define TARMOUR 17
#define TPOTION 18
#define TFOOD  19
#define TEXIT  21
#define TCOFFIN 22


char level[LEVEL_H][LEVEL_W];

int level_w;
int level_h;


static int read_all();
static int tile_char();

int item_char(c)
char c;
{
    if (c == '!')
        return 1;

    if (c == '*')
        return 1;

    if (c == 'A')
        return 1;

    if (c == 'p')
        return 1;

    if (c == 'f')
        return 1;

    if (c == 'K')
        return 1;

    return 0;
}



static int read_all(fd, buf, count)
int fd;
unsigned char *buf;
int count;
{
    int n;
    int done;


    done = 0;


    while (done < count) {

        n = read(
            fd,
            buf + done,
            count - done
        );


        if (n <= 0)
            return 0;


        done += n;
    }


    return 1;
}



static int tile_char(id)
int id;
{
    switch (id) {

    case TVOID:
        return ' ';

    case TWALL:
        return '#';

    case TFLOOR:
        return '.';

    case TSTART:
        return '@';

    case TDOWN:
        return '>';

    case TUP:
        return '<';

    case TGOLD:
        return '$';

    case TKEY:
        return 'k';

    case TCLOSE:
        return '+';

    case TLOCK:
        return 'L';

    case TOPEN:
        return '/';

    case TSKEL:
        return 'S';

    case TZOMB:
        return 'Z';

    case TBONES:
        return 'x';

    case TSWORD:
        return '!';

    case TSHIELD:
        return '*';

    case TARMOUR:
        return 'A';

    case TPOTION:
        return 'p';

    case TFOOD:
        return 'f';

    case TEXIT:
        return 'X';

    case TCOFFIN:
        return 'C';

    }


    return -1;
}


/*
 * Validate a destination level and matching stair.
 */

int level_entry_ok(name, level_number, entry_x, entry_y, expected)
char *name;
int level_number;
int entry_x;
int entry_y;
char expected;
{
    int fd;
    int width;
    int height;
    int x;
    int y;
    int starts;
    int exits;
    int monsters;
    int found;
    int c;

    unsigned char header[MHSZ];
    unsigned char row[LEVEL_W];
    unsigned char id;


    fd = open(name, 0);

    if (fd < 0)
        return 0;


    if (!read_all(
            fd,
            header,
            MHSZ)) {

        close(fd);
        return 0;
    }


    if (header[0] != 'B' ||
        header[1] != 'K' ||
        header[2] != 'M' ||
        header[3] != '1') {

        close(fd);
        return 0;
    }


    width = header[4];
    height = header[5];


    if (width <= 0 ||
        width > LEVEL_W ||
        height <= 0 ||
        height > LEVEL_H ||
        header[6] != MTYPES ||
        header[7] != 0) {

        close(fd);
        return 0;
    }


    if (entry_x < 0 || entry_y < 0 ||
        entry_x >= width || entry_y >= height) {

        close(fd);
        return 0;
    }


    starts = 0;
    exits = 0;
    monsters = 0;
    found = 0;


    for (y = 0; y < height; ++y) {

        if (!read_all(
                fd,
                row,
                width)) {

            close(fd);
            return 0;
        }


        for (x = 0; x < width; ++x) {

            id = row[x];


            if (id >= MTYPES) {

                close(fd);
                return 0;
            }


            c = tile_char(id);

            if (c < 0) {

                close(fd);
                return 0;
            }


            if (id == TSTART)
                ++starts;

            if (id == TEXIT)
                ++exits;

            if (id == TSKEL ||
                id == TZOMB)
                ++monsters;


            if (x == entry_x &&
                y == entry_y &&
                c == expected)
                found = 1;
        }
    }


    close(fd);


    if (monsters > MAX_MONSTERS)
        return 0;


    if (level_number == 0) {

        if (starts != 1)
            return 0;

    } else {

        if (starts != 0 || exits != 0)
            return 0;
    }


    return found;
}


/*
 * Load one BKM1 encoded level into the runtime map.
 *
 * The disk file stores one tile ID per map square.  The
 * engine keeps its existing character map internally, so
 * ordinary tiles are converted as they are read.  Player
 * and monster starts are handled separately.
 */

int load_level(name, level_number)
char *name;
int level_number;
{
    int fd;

    int width;
    int height;

    int x;
    int y;

    int starts;
    int exits;
    int c;

    unsigned char header[MHSZ];
    unsigned char row[LEVEL_W];
    unsigned char id;


    /*
     * Blank map memory first so unused rows and columns
     * remain void.
     */

    for (y = 0; y < LEVEL_H; ++y) {

        for (x = 0; x < LEVEL_W; ++x)
            level[y][x] = ' ';
    }


    monster_reset();


    level_w = 0;
    level_h = 0;
    starts = 0;
    exits = 0;


    fd = open(name, 0);

    if (fd < 0)
        return 0;


    if (!read_all(
            fd,
            header,
            MHSZ)) {

        close(fd);
        return 0;
    }


    if (header[0] != 'B' ||
        header[1] != 'K' ||
        header[2] != 'M' ||
        header[3] != '1') {

        close(fd);
        return 0;
    }


    width = header[4];
    height = header[5];


    if (width <= 0 ||
        width > LEVEL_W ||
        height <= 0 ||
        height > LEVEL_H) {

        close(fd);
        return 0;
    }


    if (header[6] != MTYPES ||
        header[7] != 0) {

        close(fd);
        return 0;
    }


    for (y = 0; y < height; ++y) {

        if (!read_all(
                fd,
                row,
                width)) {

            close(fd);
            return 0;
        }


        for (x = 0; x < width; ++x) {

            id = row[x];


            if (id >= MTYPES) {

                close(fd);
                return 0;
            }


            c = tile_char(id);

            if (c < 0) {

                close(fd);
                return 0;
            }


            if (id == TEXIT)
                ++exits;


            if (id == TSTART) {

                player_x = x;
                player_y = y;

                level[y][x] = '.';

                ++starts;
            }


            else if (id == TSKEL ||
                     id == TZOMB) {

                if (!monster_add(
                        (char)c,
                        x,
                        y,
                        level_number)) {

                    close(fd);
                    return 0;
                }


                level[y][x] = '.';
            }


            else {

                level[y][x] = (char)c;
            }
        }
    }


    close(fd);


    level_w = width;
    level_h = height;


    if (level_number == 0) {

        if (starts != 1)
            return 0;

    } else {

        if (starts != 0 || exits != 0)
            return 0;
    }


    return 1;
}


int map_passable(x, y)
int x;
int y;
{
    if (x < 0 || y < 0)
        return 0;


    if (x >= level_w || y >= level_h)
        return 0;


    if (level[y][x] == '#')
        return 0;


    if (level[y][x] == ' ')
        return 0;


    if (level[y][x] == '+')
        return 0;


    if (level[y][x] == 'L')
        return 0;


    if (level[y][x] == 'C')
        return 0;


    return 1;
}


int map_exit(x, y)
int x;
int y;
{
    if (x < 0 || y < 0)
        return 0;


    if (x >= level_w || y >= level_h)
        return 0;


    return level[y][x] == 'X';
}


int map_closed_door(x, y)
int x;
int y;
{
    if (x < 0 || y < 0)
        return 0;


    if (x >= level_w || y >= level_h)
        return 0;


    return level[y][x] == '+';
}


int map_locked_door(x, y)
int x;
int y;
{
    if (x < 0 || y < 0)
        return 0;


    if (x >= level_w || y >= level_h)
        return 0;


    return level[y][x] == 'L';
}


void map_open_door(x, y)
int x;
int y;
{
    if (map_closed_door(x, y))
        level[y][x] = '/';
}


void map_unlock_door(x, y)
int x;
int y;
{
    if (map_locked_door(x, y))
        level[y][x] = '/';
}

