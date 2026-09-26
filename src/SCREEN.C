#include "game.h"


void pcw_goto(x, y)
int x;
int y;
{
    putch(27);
    putch('Y');
    putch(y + 32);
    putch(x + 32);
}


void pcw_clear()
{
    putch(27);
    putch('E');
}


void pcw_cursor(on)
int on;
{
    putch(27);

    if (on)
        putch('e');
    else
        putch('f');
}


void pcw_erase_eol()
{
    putch(27);
    putch('K');
}


void pcw_text(x, y, text)
int x;
int y;
char *text;
{
    pcw_goto(x, y);

    while (*text)
        putch(*text++);
}


void pcw_number(n)
unsigned int n;
{
    char buf[6];
    int i;


    if (n == 0) {

        putch('0');

        return;
    }


    i = 0;


    while (n > 0 && i < 5) {

        buf[i] = '0' + (n % 10);

        ++i;

        n /= 10;
    }


    while (i > 0) {

        --i;

        putch(buf[i]);
    }
}


void pcw_number2(n)
unsigned int n;
{
    if (n > 99)
        n = 99;


    if (n < 10) {

        putch(' ');
        putch('0' + n);

        return;
    }


    putch('0' + (n / 10));
    putch('0' + (n % 10));
}


/*
 * Draw static BONEKEEP screen frame.
 *
 * The game uses rows 0 to 29.
 * PCW rows 30 and 31 are untouched.
 */

void pcw_draw_ui()
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
     * Main vertical borders.
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
     * BONEKEEP title.
     */

    pcw_text(
        PANEL_X,
        2,
        "  BONEKEEP  "
    );


    /*
     * Separator beneath title.
     */

    pcw_goto(
        DIVIDER_X,
        4
    );


    putch('+');


    for (x = PANEL_X;
         x < RIGHT_X;
         ++x) {

        putch('-');
    }


    putch('+');


    /*
     * Inventory separator.
     */

    pcw_goto(
        DIVIDER_X,
        13
    );


    putch('+');


    for (x = PANEL_X;
         x < RIGHT_X;
         ++x) {

        putch('-');
    }


    putch('+');


    /*
     * Inventory heading and six slots.
     * Keys 1-6 select slots 1-6.
     *
     * Inventory starts empty.  Item
     * pickups redraw individual slots.
     */

    pcw_text(
        PANEL_X,
        14,
        " INVENTORY  "
    );


    pcw_text(PANEL_X, 15, "1 { empty } ");
    pcw_text(PANEL_X, 16, "2 { empty } ");
    pcw_text(PANEL_X, 17, "3 { empty } ");
    pcw_text(PANEL_X, 18, "4 { empty } ");
    pcw_text(PANEL_X, 19, "5 { empty } ");
    pcw_text(PANEL_X, 20, "6 { empty } ");


    /*
     * Bottom of map/stats area.
     * Also top of message box.
     *
     * MAP_H = 24, so this is row 25.
     */

    pcw_goto(0, 25);

    putch('+');


    for (x = 1; x < DIVIDER_X; ++x)
        putch('-');


    putch('+');


    for (x = PANEL_X; x < RIGHT_X; ++x)
        putch('-');


    putch('+');


    /*
     * Three message rows:
     *
     * 26
     * 27
     * 28
     */

    for (y = MESSAGE_Y;
         y < MESSAGE_Y + MESSAGE_LINES;
         ++y) {

        pcw_goto(0, y);
        putch('|');


        pcw_goto(RIGHT_X, y);
        putch('|');
    }


    /*
     * Bottom border on row 29.
     */

    pcw_goto(0, 29);

    putch('+');


    for (x = 1; x < RIGHT_X; ++x)
        putch('-');


    putch('+');
}


/*
 * Clear only the 75 x 24 map interior.
 * Borders, sidebar and messages remain intact.
 */

void pcw_clear_map()
{
    int x;
    int y;


    for (y = 0; y < MAP_H; ++y) {

        pcw_goto(
            MAP_X,
            MAP_Y + y
        );


        for (x = 0; x < MAP_W; ++x)
            putch(' ');
    }
}


/*
 * Clear only the inside of the
 * three-line message box.
 */

void pcw_clear_messages()
{
    int x;
    int y;


    for (y = MESSAGE_Y;
         y < MESSAGE_Y + MESSAGE_LINES;
         ++y) {

        pcw_goto(MESSAGE_X, y);


        for (x = 0;
             x < MESSAGE_W;
             ++x) {

            putch(' ');
        }
    }
}

/*
 * Draw one victory page from BONEKEEP.DAT.
 * The data file holds the large text and art so
 * almost none of it occupies the COM file.
 */

static void win_number(line, pos, n)
char *line;
int pos;
unsigned int n;
{
    int i;


    i = pos + 4;


    do {
        line[i--] =
            '0' + (n % 10);

        n /= 10;

    } while (n &&
              i >= pos);
}


static void win_name(line, text, comma)
char *line;
char *text;
int comma;
{
    int len;
    int pos;
    int i;
    char c;


    len = 0;

    while (text[len] &&
           len < 30)
        ++len;


    pos = 7 +
        ((81 - len - comma) / 2);


    for (i = 0;
         i < len;
         ++i) {

        c = text[i];

        if (c >= 'a' &&
            c <= 'z')
            c = c - 'a' + 'A';


        line[pos + i] = c;
    }


    if (comma)
        line[pos + len] = ',';
}


void pcw_victory(zombies, skeletons, gold, turns)
unsigned int zombies;
unsigned int skeletons;
unsigned int gold;
unsigned int turns;
{
    char line[90];
    int fd;
    int page;
    int row;
    int skip;


    fd = open(
        "BONEKEEP.DAT",
        0
    );


    if (fd < 0) {

        pcw_clear();
        pcw_text(
            31,
            13,
            "BONEKEEP.DAT missing"
        );

        return;
    }


    page = rand() % 10;
    skip = page * 27;


    while (skip-- > 0) {

        if (read(fd, line, 89) != 89) {

            close(fd);
            return;
        }
    }


    pcw_clear();


    for (row = 1;
         row <= 27;
         ++row) {

        if (read(fd, line, 89) != 89) {

            close(fd);
            return;
        }


        line[89] = 0;


        if (row == 11)
            win_name(
                line,
                player_name,
                0
            );


        if (row == 13) {

            win_number(
                line,
                19,
                zombies
            );

            win_number(
                line,
                44,
                skeletons
            );
        }


        if (row == 15)
            win_name(
                line,
                vampire_name,
                1
            );


        if (row == 17) {

            win_number(
                line,
                36,
                gold
            );

            win_number(
                line,
                55,
                turns
            );
        }


        pcw_text(
            0,
            row,
            line
        );
    }


    close(fd);


    pcw_text(
        37,
        29,
        "Press any key."
    );
}

