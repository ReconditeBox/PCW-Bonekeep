#include "MAPFMT.H"


extern int open();
extern int read();
extern int write();
extern int close();
extern int creat();
extern int printf();


static unsigned char map_data[BK_MAP_H][BK_MAP_W];


static int tile_id();
static int output_level_number();
static int write_all();
static int encode_map();


/*
 * Convert a source-map character to its BONEKEEP tile ID.
 * Returns -1 for an invalid map character.
 */

static int tile_id(c)
char c;
{
    switch (c) {

    case BK_C_VOID:
        return BK_T_VOID;

    case BK_C_WALL:
        return BK_T_WALL;

    case BK_C_FLOOR:
        return BK_T_FLOOR;

    case BK_C_START:
        return BK_T_START;

    case BK_C_DOWN:
        return BK_T_DOWN;

    case BK_C_UP:
        return BK_T_UP;

    case BK_C_GOLD:
        return BK_T_GOLD;

    case BK_C_KEY:
        return BK_T_KEY;

    case BK_C_DOOR_CLOSED:
        return BK_T_DOOR_CLOSED;

    case BK_C_DOOR_LOCKED:
        return BK_T_DOOR_LOCKED;

    case BK_C_DOOR_OPEN:
        return BK_T_DOOR_OPEN;

    case BK_C_SKELETON:
        return BK_T_SKELETON;

    case BK_C_ZOMBIE:
        return BK_T_ZOMBIE;

    case BK_C_BONES:
        return BK_T_BONES;

    case BK_C_SWORD:
        return BK_T_SWORD;

    case BK_C_SHIELD:
        return BK_T_SHIELD;

    case BK_C_ARMOUR:
        return BK_T_ARMOUR;

    case BK_C_POTION:
        return BK_T_POTION;

    case BK_C_FOOD:
        return BK_T_FOOD;

    case BK_C_COFFIN:
        return BK_T_COFFIN;

    case BK_C_EXIT:
        return BK_T_EXIT;
    }


    return -1;
}


/*
 * Return the numeric .000 to .999 extension used by a
 * BONEKEEP level file.  Paths and drive prefixes are fine;
 * only the final four characters are examined.
 */

static int output_level_number(name)
char *name;
{
    int len;
    int a;
    int b;
    int c;


    len = 0;

    while (name[len])
        ++len;


    if (len < 4)
        return -1;


    if (name[len - 4] != '.')
        return -1;


    a = name[len - 3] - '0';
    b = name[len - 2] - '0';
    c = name[len - 1] - '0';


    if (a < 0 || a > 9 ||
        b < 0 || b > 9 ||
        c < 0 || c > 9)
        return -1;


    return (a * 100) + (b * 10) + c;
}


/*
 * Write all requested bytes.
 */

static int write_all(fd, buf, count)
int fd;
unsigned char *buf;
int count;
{
    int n;
    int done;


    done = 0;


    while (done < count) {

        n = write(
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


static int encode_map(input_name, output_name)
char *input_name;
char *output_name;
{
    int in_fd;
    int out_fd;

    int n;
    int i;

    int x;
    int y;

    int width;
    int height;

    int id;
    int starts;
    int exits;
    int level_number;

    int done;

    unsigned char input[128];
    unsigned char header[BK_MAP_HEADER_SIZE];

    char c;


    /*
     * Start with a completely void map.  Short source-map
     * lines are therefore padded with void tiles.
     */

    for (y = 0; y < BK_MAP_H; ++y) {

        for (x = 0; x < BK_MAP_W; ++x)
            map_data[y][x] = BK_T_VOID;
    }


    in_fd = open(input_name, 0);

    if (in_fd < 0) {

        printf(
            "Cannot open %s\n",
            input_name
        );

        return 0;
    }


    x = 0;
    y = 0;

    width = 0;
    height = 0;

    starts = 0;
    exits = 0;
    done = 0;
    n = 0;


    while (!done) {

        n = read(
            in_fd,
            input,
            sizeof(input)
        );


        if (n <= 0)
            break;


        for (i = 0; i < n; ++i) {

            c = input[i];


            /* CP/M text EOF. */

            if ((unsigned char)c == 26) {

                done = 1;

                break;
            }


            if (c == '\r')
                continue;


            if (c == '\n') {

                if (y >= BK_MAP_H) {

                    printf(
                        "Map is more than %d rows high.\n",
                        BK_MAP_H
                    );

                    close(in_fd);

                    return 0;
                }


                if (x > width)
                    width = x;


                ++y;
                x = 0;

                continue;
            }


            if (y >= BK_MAP_H) {

                printf(
                    "Map is more than %d rows high.\n",
                    BK_MAP_H
                );

                close(in_fd);

                return 0;
            }


            if (x >= BK_MAP_W) {

                printf(
                    "Row %d is more than %d columns wide.\n",
                    y + 1,
                    BK_MAP_W
                );

                close(in_fd);

                return 0;
            }


            id = tile_id(c);


            if (id < 0) {

                printf(
                    "Invalid map character at row %d column %d.\n",
                    y + 1,
                    x + 1
                );

                close(in_fd);

                return 0;
            }


            map_data[y][x] =
                (unsigned char)id;


            if (c == '@')
                ++starts;

            if (c == 'X')
                ++exits;


            ++x;
        }
    }


    close(in_fd);


    if (n < 0) {

        printf(
            "Error reading %s\n",
            input_name
        );

        return 0;
    }


    /*
     * Count a final line which has no CR/LF.
     */

    if (x > 0) {

        if (x > width)
            width = x;


        ++y;
    }


    height = y;


    if (width <= 0 || height <= 0) {

        printf("Map is empty.\n");

        return 0;
    }


    /*
     * The output extension is the level number.
     * LEVEL.000 is the only level with the initial @ start
     * and the only level allowed to contain dungeon exits X.
     */

    level_number =
        output_level_number(output_name);


    if (level_number < 0) {

        printf(
            "Output file must use a .000 to .999 extension.\n"
        );

        return 0;
    }


    if (level_number == 0) {

        if (starts != 1) {

            printf(
                "LEVEL.000 must contain exactly one @ start.\n"
            );

            return 0;
        }

    } else {

        if (starts != 0) {

            printf(
                "Only LEVEL.000 may contain @.\n"
            );

            return 0;
        }


        if (exits != 0) {

            printf(
                "Only LEVEL.000 may contain X exits.\n"
            );

            return 0;
        }
    }


    header[0] = BK_MAP_MAGIC0;
    header[1] = BK_MAP_MAGIC1;
    header[2] = BK_MAP_MAGIC2;
    header[3] = BK_MAP_MAGIC3;

    header[4] = (unsigned char)width;
    header[5] = (unsigned char)height;
    header[6] = BK_TILE_TYPES;
    header[7] = 0;


    out_fd = creat(output_name, 0);

    if (out_fd < 0) {

        printf(
            "Cannot create %s\n",
            output_name
        );

        return 0;
    }


    if (!write_all(
            out_fd,
            header,
            BK_MAP_HEADER_SIZE)) {

        printf(
            "Error writing %s\n",
            output_name
        );

        close(out_fd);

        return 0;
    }


    for (y = 0; y < height; ++y) {

        if (!write_all(
                out_fd,
                map_data[y],
                width)) {

            printf(
                "Error writing %s\n",
                output_name
            );

            close(out_fd);

            return 0;
        }
    }


    close(out_fd);


    printf(
        "%s: %d x %d, %d map bytes.\n",
        output_name,
        width,
        height,
        width * height
    );


    return 1;
}


int main(argc, argv)
int argc;
char **argv;
{
    if (argc != 3) {

        printf(
            "BONEKEEP MENCODE\n"
            "Usage: MENCODE source.txt LEVEL.000\n"
        );

        return 1;
    }


    if (!encode_map(
            argv[1],
            argv[2]))
        return 1;


    return 0;
}
