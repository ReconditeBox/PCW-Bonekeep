#include "MAPFMT.H"

/*
 * MEDIT - BONEKEEP raw map editor
 *
 * Hi-Tech C / CP/M
 *
 * Usage:
 *
 *     MEDIT filename
 *
 * Lowercase w/a/s/d move the cursor.
 * Map-symbol keys place tiles.
 * Space places void.
 * ENTER asks:
 *
 *     Save & Quit or Quit? (s/q)
 */

#define SCREEN_W        90
#define SCREEN_H        30

#define MAP_W           BK_MAP_W
#define MAP_H           BK_MAP_H

#define MAP_X           1
#define MAP_Y           1

#define DIVIDER_X       76
#define PANEL_X         77
#define RIGHT_X         89

#define MESSAGE_X       1
#define MESSAGE_Y       26
#define MESSAGE_W       88
#define MESSAGE_LINES   3


extern char getch();
extern void putch();

extern int open();
extern int read();
extern int write();
extern int close();
extern int creat();


static char map_data[MAP_H][MAP_W];

static int cursor_x;
static int cursor_y;


static void pcw_goto();
static void pcw_clear();
static void pcw_cursor();
static void pcw_text();

static void clear_messages();
static void show_message();

static void clear_map_data();
static int valid_tile();
static int load_map();
static int save_map();

static void draw_ui();
static void draw_symbol_list();
static void draw_map();
static void draw_help();

static void move_cursor();
static void place_tile();
static int finish_editor();


static void pcw_goto(x, y)
int x;
int y;
{
    putch(27);
    putch('Y');
    putch(y + 32);
    putch(x + 32);
}


static void pcw_clear()
{
    putch(27);
    putch('E');
}


static void pcw_cursor(on)
int on;
{
    putch(27);

    if (on)
        putch('e');
    else
        putch('f');
}


static void pcw_text(x, y, text)
int x;
int y;
char *text;
{
    pcw_goto(x, y);

    while (*text)
        putch(*text++);
}


static void clear_messages()
{
    int x;
    int y;


    for (y = MESSAGE_Y;
         y < MESSAGE_Y + MESSAGE_LINES;
         ++y) {

        pcw_goto(MESSAGE_X, y);


        for (x = MESSAGE_X;
             x < DIVIDER_X;
             ++x) {

            putch(' ');
        }
    }
}


static void show_message(text)
char *text;
{
    clear_messages();

    pcw_text(
        MESSAGE_X,
        MESSAGE_Y,
        text
    );
}


static void clear_map_data()
{
    int x;
    int y;


    for (y = 0; y < MAP_H; ++y) {

        for (x = 0; x < MAP_W; ++x)
            map_data[y][x] = ' ';
    }
}


/*
 * Raw BONEKEEP map symbols.
 *
 * Lowercase w/a/s/d are deliberately not tiles,
 * so they remain available for cursor movement.
 */

static int valid_tile(c)
char c;
{
    switch (c) {

    case BK_C_VOID:
    case BK_C_WALL:
    case BK_C_FLOOR:
    case BK_C_START:
    case BK_C_EXIT:
    case BK_C_DOWN:
    case BK_C_UP:
    case BK_C_GOLD:
    case BK_C_KEY:
    case BK_C_DOOR_CLOSED:
    case BK_C_DOOR_LOCKED:
    case BK_C_DOOR_OPEN:
    case BK_C_SKELETON:
    case BK_C_ZOMBIE:
    case BK_C_BONES:
    case BK_C_SWORD:
    case BK_C_SHIELD:
    case BK_C_ARMOUR:
    case BK_C_POTION:
    case BK_C_FOOD:
    case BK_C_COFFIN:

        return 1;
    }


    return 0;
}


/*
 * Load a raw text map.
 *
 * If the file does not exist, return -1 so the
 * caller can start a new blank map.
 *
 * Return zero for an invalid map file.
 * Return one for a successful load.
 */

static int load_map(name)
char *name;
{
    int fd;
    int n;
    int i;

    int x;
    int y;

    char buf[128];
    char c;


    clear_map_data();


    fd = open(name, 0);

    if (fd < 0)
        return -1;


    x = 0;
    y = 0;


    for (;;) {

        n = read(fd, buf, 128);

        if (n < 0) {

            close(fd);

            return 0;
        }


        if (n == 0)
            break;


        for (i = 0; i < n; ++i) {

            c = buf[i];


            if ((unsigned char)c == 26) {

                close(fd);

                return 1;
            }


            if (c == '\r')
                continue;


            if (c == '\n') {

                x = 0;

                if (y < MAP_H)
                    ++y;

                continue;
            }


            if (y >= MAP_H ||
                x >= MAP_W) {

                close(fd);

                return 0;
            }


            if (!valid_tile(c)) {

                close(fd);

                return 0;
            }


            map_data[y][x] = c;

            ++x;
        }
    }


    close(fd);

    return 1;
}


/*
 * Save only the used rectangular part of the map.
 *
 * Trailing spaces on each line are omitted.
 * Intermediate blank lines are preserved.
 * A CP/M ^Z EOF is appended.
 */

static int save_map(name)
char *name;
{
    int fd;

    int x;
    int y;

    int last_x;
    int last_y;

    char crlf[2];
    char eof;


    last_y = -1;


    for (y = 0; y < MAP_H; ++y) {

        for (x = MAP_W - 1; x >= 0; --x) {

            if (map_data[y][x] != ' ') {

                last_y = y;

                break;
            }
        }
    }


    fd = creat(name, 0);

    if (fd < 0)
        return 0;


    crlf[0] = '\r';
    crlf[1] = '\n';


    if (last_y >= 0) {

        for (y = 0; y <= last_y; ++y) {

            last_x = -1;


            for (x = MAP_W - 1; x >= 0; --x) {

                if (map_data[y][x] != ' ') {

                    last_x = x;

                    break;
                }
            }


            if (last_x >= 0) {

                if (write(
                        fd,
                        map_data[y],
                        last_x + 1) !=
                    last_x + 1) {

                    close(fd);

                    return 0;
                }
            }


            if (write(fd, crlf, 2) != 2) {

                close(fd);

                return 0;
            }
        }
    }


    eof = 26;


    if (write(fd, &eof, 1) != 1) {

        close(fd);

        return 0;
    }


    close(fd);

    return 1;
}


static void draw_ui()
{
    int x;
    int y;


    /*
     * Top border.
     */

    pcw_goto(0, 0);

    putch('+');


    for (x = 1; x < DIVIDER_X; ++x)
        putch('-');


    putch('+');


    for (x = PANEL_X; x < RIGHT_X; ++x)
        putch('-');


    putch('+');


    /*
     * Map and sidebar borders.
     */

    for (y = 1; y <= MAP_H; ++y) {

        pcw_goto(0, y);
        putch('|');


        pcw_goto(DIVIDER_X, y);
        putch('|');


        pcw_goto(RIGHT_X, y);
        putch('|');
    }


    /*
     * Bottom of map / top of messages on the left.
     * The sidebar continues through to the bottom.
     */

    pcw_goto(0, 25);

    putch('+');


    for (x = 1; x < DIVIDER_X; ++x)
        putch('-');


    putch('+');


    /*
     * Message rows on the left and sidebar on the right.
     */

    for (y = MESSAGE_Y;
         y < MESSAGE_Y + MESSAGE_LINES;
         ++y) {

        pcw_goto(0, y);
        putch('|');


        pcw_goto(DIVIDER_X, y);
        putch('|');


        pcw_goto(RIGHT_X, y);
        putch('|');
    }


    /*
     * Bottom border.
     */

    pcw_goto(0, 29);

    putch('+');


    for (x = 1; x < DIVIDER_X; ++x)
        putch('-');


    putch('+');


    for (x = PANEL_X; x < RIGHT_X; ++x)
        putch('-');


    putch('+');
}


static void draw_symbol_list()
{
    pcw_text(PANEL_X, 1,  "# wall      ");
    pcw_text(PANEL_X, 2,  ". floor     ");
    pcw_text(PANEL_X, 3,  "@ start     ");
    pcw_text(PANEL_X, 4,  "X exit      ");
    pcw_text(PANEL_X, 5,  "> down      ");
    pcw_text(PANEL_X, 6,  "< up        ");
    pcw_text(PANEL_X, 7,  "$ gold      ");
    pcw_text(PANEL_X, 8,  "k key       ");
    pcw_text(PANEL_X, 9,  "+ closed    ");
    pcw_text(PANEL_X, 10, "L locked    ");
    pcw_text(PANEL_X, 11, "/ open      ");
    pcw_text(PANEL_X, 13, "S skeleton  ");
    pcw_text(PANEL_X, 14, "Z zombie    ");
    pcw_text(PANEL_X, 15, "x bones     ");
    pcw_text(PANEL_X, 16, "! sword     ");
    pcw_text(PANEL_X, 17, "* shield    ");
    pcw_text(PANEL_X, 18, "A armour    ");
    pcw_text(PANEL_X, 19, "p potion    ");
    pcw_text(PANEL_X, 20, "f food      ");
    pcw_text(PANEL_X, 21, "C coffin    ");
}


static void draw_map()
{
    int x;
    int y;


    for (y = 0; y < MAP_H; ++y) {

        pcw_goto(
            MAP_X,
            MAP_Y + y
        );


        for (x = 0; x < MAP_W; ++x)
            putch(map_data[y][x]);
    }
}


static void draw_help(name)
char *name;
{
    clear_messages();


    pcw_text(
        MESSAGE_X,
        MESSAGE_Y,
        "w/a/s/d move. Symbol places tile. SPACE=void. ENTER=finish."
    );


    pcw_text(
        MESSAGE_X,
        MESSAGE_Y + 1,
        "File: "
    );


    pcw_text(
        MESSAGE_X + 6,
        MESSAGE_Y + 1,
        name
    );
}


static void move_cursor(dx, dy)
int dx;
int dy;
{
    int nx;
    int ny;


    nx = cursor_x + dx;
    ny = cursor_y + dy;


    if (nx < 0 ||
        nx >= MAP_W ||
        ny < 0 ||
        ny >= MAP_H)
        return;


    cursor_x = nx;
    cursor_y = ny;


    pcw_goto(
        MAP_X + cursor_x,
        MAP_Y + cursor_y
    );
}


static void place_tile(c)
char c;
{
    map_data[cursor_y][cursor_x] = c;


    pcw_goto(
        MAP_X + cursor_x,
        MAP_Y + cursor_y
    );


    putch(c);


    pcw_goto(
        MAP_X + cursor_x,
        MAP_Y + cursor_y
    );
}


/*
 * Return one to exit the editor.
 * Return zero to continue editing.
 */

static int finish_editor(name)
char *name;
{
    char key;


    show_message(
        "Save & Quit or Quit? (s/q)"
    );


    for (;;) {

        key = getch();


        if (key == 'q' ||
            key == 'Q') {

            return 1;
        }


        if (key == 's' ||
            key == 'S') {

            if (save_map(name))
                return 1;


            show_message(
                "Cannot save file. Press any key."
            );


            getch();


            draw_help(name);


            pcw_goto(
                MAP_X + cursor_x,
                MAP_Y + cursor_y
            );


            return 0;
        }
    }
}


int main(argc, argv)
int argc;
char *argv[];
{
    char key;

    int load_result;
    int done;


    if (argc != 2) {

        pcw_clear();

        pcw_text(
            1,
            1,
            "Usage: MEDIT filename"
        );


        return 1;
    }


    load_result =
        load_map(argv[1]);


    if (load_result == 0) {

        pcw_clear();

        pcw_text(
            1,
            1,
            "Invalid or oversized map file."
        );


        return 1;
    }


    cursor_x = 0;
    cursor_y = 0;

    done = 0;


    pcw_clear();
    pcw_cursor(0);


    draw_ui();
    draw_symbol_list();
    draw_map();
    draw_help(argv[1]);


    if (load_result < 0) {

        pcw_text(
            MESSAGE_X,
            MESSAGE_Y + 2,
            "New blank map."
        );
    }


    pcw_cursor(1);


    pcw_goto(
        MAP_X + cursor_x,
        MAP_Y + cursor_y
    );


    while (!done) {

        key = getch();


        switch (key) {

        case 'w':

            move_cursor(0, -1);

            break;


        case 's':

            move_cursor(0, 1);

            break;


        case 'a':

            move_cursor(-1, 0);

            break;


        case 'd':

            move_cursor(1, 0);

            break;


        case '\r':
        case '\n':

            done =
                finish_editor(
                    argv[1]
                );


            if (!done) {

                pcw_goto(
                    MAP_X + cursor_x,
                    MAP_Y + cursor_y
                );
            }


            break;


        default:

            if (valid_tile(key))
                place_tile(key);

            break;
        }
    }


    pcw_cursor(1);
    pcw_clear();

    pcw_text(
        1,
        1,
        "MEDIT finished."
    );


    pcw_goto(0, 29);


    return 0;
}
