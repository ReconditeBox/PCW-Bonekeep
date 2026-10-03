#include "game.h"


static int view_x;
static int view_y;
static int view_ready;


static void ansi_number(n)
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


void pcw_goto(x, y)
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
    ansi_number(y + 1);
    putch(';');
    ansi_number(x + 1);
    putch('H');
}


void pcw_clear()
{
    putch(27);
    putch('[');
    putch('2');
    putch('J');

    putch(27);
    putch('[');
    putch('H');
}


void pcw_cursor(on)
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


void pcw_erase_eol()
{
    putch(27);
    putch('[');
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


int pcw_view_update(x, y)
int x;
int y;
{
    int nx;
    int ny;
    int max_x;
    int max_y;


    max_x = level_w - MAP_W;
    max_y = level_h - MAP_H;

    if (max_x < 0)
        max_x = 0;
    if (max_y < 0)
        max_y = 0;


    nx = view_x;
    ny = view_y;


    if (!view_ready) {

        nx = x - (MAP_W / 2);
        ny = y - (MAP_H / 2);

        view_ready = 1;

    } else {

        if (x < nx + 5)
            nx = x - 5;

        if (x >= nx + MAP_W - 5)
            nx = x - MAP_W + 6;

        if (y < ny + 3)
            ny = y - 3;

        if (y >= ny + MAP_H - 3)
            ny = y - MAP_H + 4;
    }


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


void pcw_map_char(x, y, c)
int x;
int y;
char c;
{
    if (x < view_x ||
        y < view_y)
        return;

    if (x >= view_x + MAP_W ||
        y >= view_y + MAP_H)
        return;


    pcw_goto(
        MAP_X + (x - view_x),
        MAP_Y + (y - view_y)
    );


    putch(c);
}


void pcw_draw_ui()
{
    int x;
    int y;


    view_x = 0;
    view_y = 0;
    view_ready = 0;


    pcw_goto(0, 0);
    putch('+');

    for (x = 1; x < DIVIDER_X; ++x)
        putch('-');

    putch('+');

    for (x = PANEL_X; x < RIGHT_X; ++x)
        putch('-');

    putch('+');


    for (y = 1; y <= MAP_H; ++y) {

        pcw_goto(0, y);
        putch('|');

        pcw_goto(DIVIDER_X, y);
        putch('|');

        pcw_goto(RIGHT_X, y);
        putch('|');
    }


    pcw_text(PANEL_X, 1, "  BONEKEEP   ");


    pcw_goto(DIVIDER_X, 2);
    putch('+');

    for (x = PANEL_X; x < RIGHT_X; ++x)
        putch('-');

    putch('+');


    pcw_goto(DIVIDER_X, 7);
    putch('+');

    for (x = PANEL_X; x < RIGHT_X; ++x)
        putch('-');

    putch('+');


    pcw_text(PANEL_X, INV_TITLE_Y, " INVENTORY   ");

    pcw_text(PANEL_X, INV_FIRST_Y + 0, "1 { empty }  ");
    pcw_text(PANEL_X, INV_FIRST_Y + 1, "2 { empty }  ");
    pcw_text(PANEL_X, INV_FIRST_Y + 2, "3 { empty }  ");
    pcw_text(PANEL_X, INV_FIRST_Y + 3, "4 { empty }  ");
    pcw_text(PANEL_X, INV_FIRST_Y + 4, "5 { empty }  ");
    pcw_text(PANEL_X, INV_FIRST_Y + 5, "6 { empty }  ");


    pcw_text(PANEL_X, 16, " WASD move   ");
    pcw_text(PANEL_X, 17, " 1-6 items   ");


    pcw_goto(0, 19);
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

        pcw_goto(0, y);
        putch('|');

        pcw_goto(RIGHT_X, y);
        putch('|');
    }


    pcw_goto(0, SCREEN_H - 1);
    putch('+');

    for (x = 1; x < RIGHT_X; ++x)
        putch('-');

    putch('+');
}


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
             ++x)
            putch(' ');
    }
}


static int text_length(s)
char *s;
{
    int n;

    n = 0;
    while (s[n])
        ++n;

    return n;
}


static void centred(row, text)
int row;
char *text;
{
    int len;
    int x;


    len = text_length(text);

    if (len > SCREEN_W - 2)
        len = SCREEN_W - 2;

    x = (SCREEN_W - len) / 2;

    pcw_goto(x, row);

    while (len-- > 0)
        putch(*text++);
}


static void ending_line(line, row)
char *line;
int row;
{
    char text[80];
    int first;
    int last;
    int n;


    first = 0;

    while (first < 89 &&
           line[first] == ' ')
        ++first;

    if (first < 89 &&
        line[first] == '|')
        ++first;

    while (first < 89 &&
           line[first] == ' ')
        ++first;


    last = 88;

    while (last >= first &&
           line[last] == ' ')
        --last;

    if (last >= first &&
        line[last] == '|')
        --last;

    while (last >= first &&
           line[last] == ' ')
        --last;


    n = 0;

    while (first <= last &&
           n < 78)
        text[n++] = line[first++];

    text[n] = 0;


    centred(row, text);
}


void pcw_victory(zombies, skeletons, gold, turns)
unsigned int zombies;
unsigned int skeletons;
unsigned int gold;
unsigned int turns;
{
    char line[89];
    int fd;
    int page;
    int row;
    int skip;


    pcw_clear();
    pcw_cursor(0);


    centred(2, "BONEKEEP");
    centred(4, "Dungeon completed by");
    centred(5, player_name);


    pcw_text(12, 8, "ZOMBIES");
    pcw_goto(20, 8);
    pcw_number(zombies);

    pcw_text(29, 8, "SKELETONS");
    pcw_goto(39, 8);
    pcw_number(skeletons);

    pcw_text(50, 8, "GOLD");
    pcw_goto(55, 8);
    pcw_number(gold);

    pcw_text(63, 8, "TURNS");
    pcw_goto(69, 8);
    pcw_number(turns);


    centred(10, "The Crown Vampire is destroyed.");
    centred(11, vampire_name);


    fd = open("BONEKEEP.DAT", 0);

    if (fd >= 0) {

        page = rand() % 10;
        skip = page * 27;

        while (skip-- > 0) {

            if (read(fd, line, 89) != 89) {
                close(fd);
                fd = -1;
                break;
            }
        }


        if (fd >= 0) {

            for (row = 1; row <= 27; ++row) {

                if (read(fd, line, 89) != 89)
                    break;

                if (row >= 19 && row <= 22)
                    ending_line(
                        line,
                        13 + (row - 19)
                    );
            }


            close(fd);
        }
    }


    if (fd < 0)
        centred(15, "BONEKEEP.DAT missing");


    centred(21, "Press any key.");
}
