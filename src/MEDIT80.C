#include "MAPFMT.H"

/*
 * MEDIT80 - BONEKEEP raw map editor
 *
 * Generic CP/M 2.2 / VT100, 80 x 24.
 *
 * The logical map remains 75 x 24.  A scrolling
 * 64 x 18 viewport is used for editing.
 *
 * Usage:
 *
 *     MEDIT80 filename
 */

#define SCREEN_W        80
#define SCREEN_H        24

#define MAP_W           BK_MAP_W
#define MAP_H           BK_MAP_H

#define VIEW_W          64
#define VIEW_H          18

#define MAP_X           1
#define MAP_Y           1

#define DIVIDER_X       65
#define PANEL_X         66
#define RIGHT_X         79

#define MESSAGE_X       1
#define MESSAGE_Y       20
#define MESSAGE_W       78
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

static int view_x;
static int view_y;


static void vt_number();
static void vt_goto();
static void vt_clear();
static void vt_cursor();
static void vt_text();
static void put_number2();

static void clear_messages();
static void show_message();
static void show_position();

static void clear_map_data();
static int valid_tile();
static int load_map();
static int save_map();

static void draw_ui();
static void draw_symbol_list();
static void draw_map();
static void draw_help();

static int update_view();
static void position_cursor();
static void move_cursor();
static void place_tile();
static int finish_editor();


static void vt_number(n)
int n;
{
    char buf[6];
    int i;


    if (n <= 0) {
        putch('0');
        return;
    }


    i = 0;

    while (n > 0 && i < 5) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }


    while (i > 0)
        putch(buf[--i]);
}


static void vt_goto(x, y)
int x;
int y;
{
    if (x < 0)
        x = 0;
    if (x >= SCREEN_W)
        x = SCREEN_W - 1;

    if (y < 0)
        y = 0;
    if (y >= SCREEN_H)
        y = SCREEN_H - 1;


    putch(27);
    putch('[');
    vt_number(y + 1);
    putch(';');
    vt_number(x + 1);
    putch('H');
}


static void vt_clear()
{
    putch(27);
    putch('[');
    putch('2');
    putch('J');

    putch(27);
    putch('[');
    putch('H');
}


static void vt_cursor(on)
int on;
{
    putch(27);
    putch('[');
    putch('?');
    putch('2');
    putch('5');

    if (on)
        putch('h');
    else
        putch('l');
}


static void vt_text(x, y, text)
int x;
int y;
char *text;
{
    vt_goto(x, y);

    while (*text)
        putch(*text++);
}


static void put_number2(n)
int n;
{
    if (n < 0)
        n = 0;
    if (n > 99)
        n = 99;

    putch('0' + (n / 10));
    putch('0' + (n % 10));
}


static void clear_messages()
{
    int x;
    int y;


    for (y = MESSAGE_Y;
         y < MESSAGE_Y + MESSAGE_LINES;
         ++y) {

        vt_goto(MESSAGE_X, y);

        for (x = 0; x < MESSAGE_W; ++x)
            putch(' ');
    }
}


static void show_message(text)
char *text;
{
    clear_messages();

    vt_text(
        MESSAGE_X,
        MESSAGE_Y,
        text
    );
}


static void show_position()
{
    int x;


    vt_goto(MESSAGE_X, MESSAGE_Y + 2);

    for (x = 0; x < MESSAGE_W; ++x)
        putch(' ');


    vt_goto(MESSAGE_X, MESSAGE_Y + 2);

    vt_text(
        MESSAGE_X,
        MESSAGE_Y + 2,
        "Cursor "
    );

    put_number2(cursor_x);
    putch(',');
    put_number2(cursor_y);

    vt_text(
        MESSAGE_X + 13,
        MESSAGE_Y + 2,
        "View "
    );

    put_number2(view_x);
    putch(',');
    put_number2(view_y);
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


    vt_goto(0, 0);
    putch('+');

    for (x = 1; x < DIVIDER_X; ++x)
        putch('-');

    putch('+');

    for (x = PANEL_X; x < RIGHT_X; ++x)
        putch('-');

    putch('+');


    for (y = 1; y <= VIEW_H; ++y) {

        vt_goto(0, y);
        putch('|');

        vt_goto(DIVIDER_X, y);
        putch('|');

        vt_goto(RIGHT_X, y);
        putch('|');
    }


    vt_goto(0, 19);
    putch('+');

    for (x = 1; x < DIVIDER_X; ++x)
        putch('-');

    putch('+');

    for (x = PANEL_X; x < RIGHT_X; ++x)
        putch('-');

    putch('+');


    for (y = MESSAGE_Y;
         y < MESSAGE_Y + MESSAGE_LINES;
         ++y) {

        vt_goto(0, y);
        putch('|');

        vt_goto(RIGHT_X, y);
        putch('|');
    }


    vt_goto(0, SCREEN_H - 1);
    putch('+');

    for (x = 1; x < RIGHT_X; ++x)
        putch('-');

    putch('+');
}


static void draw_symbol_list()
{
    vt_text(PANEL_X, 1,  "# wall");
    vt_text(PANEL_X, 2,  ". floor");
    vt_text(PANEL_X, 3,  "@ start");
    vt_text(PANEL_X, 4,  "X exit");
    vt_text(PANEL_X, 5,  ">down <up");
    vt_text(PANEL_X, 6,  "$gold kkey");
    vt_text(PANEL_X, 7,  "+ closed");
    vt_text(PANEL_X, 8,  "L locked");
    vt_text(PANEL_X, 9,  "/ open");
    vt_text(PANEL_X, 10, "S skel Z zomb");
    vt_text(PANEL_X, 11, "x bones");
    vt_text(PANEL_X, 12, "! sword");
    vt_text(PANEL_X, 13, "* shield");
    vt_text(PANEL_X, 14, "A armour");
    vt_text(PANEL_X, 15, "p pot f food");
    vt_text(PANEL_X, 16, "C coffin");
    vt_text(PANEL_X, 17, "SPACE void");
    vt_text(PANEL_X, 18, "ENTER finish");
}


static void draw_map()
{
    int x;
    int y;


    for (y = 0; y < VIEW_H; ++y) {

        vt_goto(
            MAP_X,
            MAP_Y + y
        );


        for (x = 0; x < VIEW_W; ++x) {

            putch(
                map_data[view_y + y]
                        [view_x + x]
            );
        }
    }
}


static void draw_help(name)
char *name;
{
    clear_messages();


    vt_text(
        MESSAGE_X,
        MESSAGE_Y,
        "w/a/s/d move. Symbol places tile. SPACE=void. ENTER=finish."
    );


    vt_text(
        MESSAGE_X,
        MESSAGE_Y + 1,
        "File: "
    );

    vt_text(
        MESSAGE_X + 6,
        MESSAGE_Y + 1,
        name
    );


    show_position();
}


static int update_view()
{
    int nx;
    int ny;
    int max_x;
    int max_y;


    max_x = MAP_W - VIEW_W;
    max_y = MAP_H - VIEW_H;


    nx = view_x;
    ny = view_y;


    if (cursor_x < nx + 5)
        nx = cursor_x - 5;

    if (cursor_x >= nx + VIEW_W - 5)
        nx = cursor_x - VIEW_W + 6;

    if (cursor_y < ny + 3)
        ny = cursor_y - 3;

    if (cursor_y >= ny + VIEW_H - 3)
        ny = cursor_y - VIEW_H + 4;


    if (nx < 0)
        nx = 0;
    if (ny < 0)
        ny = 0;

    if (nx > max_x)
        nx = max_x;
    if (ny > max_y)
        ny = max_y;


    if (nx == view_x &&
        ny == view_y)
        return 0;


    view_x = nx;
    view_y = ny;

    return 1;
}


static void position_cursor()
{
    vt_goto(
        MAP_X + cursor_x - view_x,
        MAP_Y + cursor_y - view_y
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


    if (update_view())
        draw_map();


    show_position();
    position_cursor();
}


static void place_tile(c)
char c;
{
    map_data[cursor_y][cursor_x] = c;


    position_cursor();
    putch(c);
    position_cursor();
}


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
            position_cursor();


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

        vt_clear();

        vt_text(
            1,
            1,
            "Usage: MEDIT80 filename"
        );


        return 1;
    }


    load_result =
        load_map(argv[1]);


    if (load_result == 0) {

        vt_clear();

        vt_text(
            1,
            1,
            "Invalid or oversized map file."
        );


        return 1;
    }


    cursor_x = 0;
    cursor_y = 0;

    view_x = 0;
    view_y = 0;

    done = 0;


    vt_clear();
    vt_cursor(0);


    draw_ui();
    draw_symbol_list();
    draw_map();
    draw_help(argv[1]);


    if (load_result < 0) {

        vt_text(
            MESSAGE_X + 30,
            MESSAGE_Y + 1,
            "New blank map."
        );
    }


    vt_cursor(1);
    position_cursor();


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


            if (!done)
                position_cursor();


            break;


        default:

            if (valid_tile(key))
                place_tile(key);

            break;
        }
    }


    vt_cursor(1);
    vt_clear();

    vt_text(
        1,
        1,
        "MEDIT80 finished."
    );


    vt_goto(0, SCREEN_H - 1);


    return 0;
}
