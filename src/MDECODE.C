#include "MAPFMT.H"


extern int open();
extern int read();
extern int write();
extern int close();
extern int creat();
extern int printf();


static int read_all();
static int write_all();
static int tile_char();
static int decode_map();


/*
 * Read exactly count bytes.
 */

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


/*
 * Convert a BONEKEEP tile ID back to its source-map
 * character.  Returns -1 for an undefined tile ID.
 */

static int tile_char(id)
int id;
{
    switch (id) {

    case BK_T_VOID:
        return BK_C_VOID;

    case BK_T_WALL:
        return BK_C_WALL;

    case BK_T_FLOOR:
        return BK_C_FLOOR;

    case BK_T_START:
        return BK_C_START;

    case BK_T_DOWN:
        return BK_C_DOWN;

    case BK_T_UP:
        return BK_C_UP;

    case BK_T_GOLD:
        return BK_C_GOLD;

    case BK_T_KEY:
        return BK_C_KEY;

    case BK_T_DOOR_CLOSED:
        return BK_C_DOOR_CLOSED;

    case BK_T_DOOR_LOCKED:
        return BK_C_DOOR_LOCKED;

    case BK_T_DOOR_OPEN:
        return BK_C_DOOR_OPEN;

    case BK_T_SKELETON:
        return BK_C_SKELETON;

    case BK_T_ZOMBIE:
        return BK_C_ZOMBIE;

    case BK_T_BONES:
        return BK_C_BONES;

    case BK_T_SWORD:
        return BK_C_SWORD;

    case BK_T_SHIELD:
        return BK_C_SHIELD;

    case BK_T_ARMOUR:
        return BK_C_ARMOUR;

    case BK_T_POTION:
        return BK_C_POTION;

    case BK_T_FOOD:
        return BK_C_FOOD;

    case BK_T_COFFIN:
        return BK_C_COFFIN;

    case BK_T_EXIT:
        return BK_C_EXIT;
    }


    return -1;
}


static int decode_map(input_name, output_name)
char *input_name;
char *output_name;
{
    int in_fd;
    int out_fd;

    int width;
    int height;

    int x;
    int y;

    int c;

    unsigned char header[BK_MAP_HEADER_SIZE];
    unsigned char row[BK_MAP_W];
    unsigned char line[BK_MAP_W + 2];


    in_fd = open(input_name, 0);

    if (in_fd < 0) {

        printf(
            "Cannot open %s\n",
            input_name
        );

        return 0;
    }


    if (!read_all(
            in_fd,
            header,
            BK_MAP_HEADER_SIZE)) {

        printf(
            "%s is too short or cannot be read.\n",
            input_name
        );

        close(in_fd);

        return 0;
    }


    if (header[0] != BK_MAP_MAGIC0 ||
        header[1] != BK_MAP_MAGIC1 ||
        header[2] != BK_MAP_MAGIC2 ||
        header[3] != BK_MAP_MAGIC3) {

        printf(
            "%s is not a BONEKEEP BKM1 level.\n",
            input_name
        );

        close(in_fd);

        return 0;
    }


    width = header[4];
    height = header[5];


    if (width <= 0 ||
        width > BK_MAP_W ||
        height <= 0 ||
        height > BK_MAP_H) {

        printf(
            "Invalid map size %d x %d.\n",
            width,
            height
        );

        close(in_fd);

        return 0;
    }


    if (header[6] != BK_TILE_TYPES) {

        printf(
            "Unsupported tile table: %d types.\n",
            header[6]
        );

        close(in_fd);

        return 0;
    }


    if (header[7] != 0) {

        printf(
            "Unsupported map flags: %d.\n",
            header[7]
        );

        close(in_fd);

        return 0;
    }


    out_fd = creat(output_name, 0);

    if (out_fd < 0) {

        printf(
            "Cannot create %s\n",
            output_name
        );

        close(in_fd);

        return 0;
    }


    for (y = 0; y < height; ++y) {

        if (!read_all(
                in_fd,
                row,
                width)) {

            printf(
                "%s ends before all map data is present.\n",
                input_name
            );

            close(in_fd);
            close(out_fd);

            return 0;
        }


        for (x = 0; x < width; ++x) {

            if (row[x] >= BK_TILE_TYPES) {

                printf(
                    "Invalid tile ID %d at row %d column %d.\n",
                    row[x],
                    y + 1,
                    x + 1
                );

                close(in_fd);
                close(out_fd);

                return 0;
            }


            c = tile_char(row[x]);


            if (c < 0) {

                printf(
                    "Undefined tile ID %d at row %d column %d.\n",
                    row[x],
                    y + 1,
                    x + 1
                );

                close(in_fd);
                close(out_fd);

                return 0;
            }


            line[x] = (unsigned char)c;
        }


        line[width] = '\r';
        line[width + 1] = '\n';


        if (!write_all(
                out_fd,
                line,
                width + 2)) {

            printf(
                "Error writing %s\n",
                output_name
            );

            close(in_fd);
            close(out_fd);

            return 0;
        }
    }


    close(in_fd);
    close(out_fd);


    printf(
        "%s: %d x %d decoded to %s.\n",
        input_name,
        width,
        height,
        output_name
    );


    return 1;
}


int main(argc, argv)
int argc;
char **argv;
{
    if (argc != 3) {

        printf(
            "BONEKEEP MDECODE\n"
            "Usage: MDECODE LEVEL.000 output.txt\n"
        );

        return 1;
    }


    if (!decode_map(
            argv[1],
            argv[2]))
        return 1;


    return 0;
}
