#include "game.h"


char vampire_name[31];


static char *vampire_first[] = {
    "Mordain",
    "Varnek",
    "Sereth",
    "Veylor",
    "Draven",
    "Malrec",
    "Korvin",
    "Tharos",
    "Velkan",
    "Orsik",
    "Nerez",
    "Caldor",
    "Ravik",
    "Sarkan",
    "Morvek",
    "Zareth"
};


static char *vampire_last[] = {
    "Hollow",
    "Pale",
    "Red",
    "Grim",
    "Cold",
    "Cruel",
    "Lost",
    "Black",
    "Ancient",
    "Bleak",
    "Cursed",
    "Hungry",
    "Silent",
    "Ashen",
    "Forsaken",
    "Unquiet"
};


static void add_text(dst, pos, src)
char *dst;
int *pos;
char *src;
{
    while (*src &&
           *pos < 30) {

        dst[*pos] = *src;

        ++(*pos);
        ++src;
    }


    dst[*pos] = 0;
}


void vampire_name_init()
{
    int first;
    int last;
    int pos;


    first = rand() % 16;
    last = rand() % 16;

    pos = 0;
    vampire_name[0] = 0;


    add_text(
        vampire_name,
        &pos,
        vampire_first[first]
    );


    add_text(
        vampire_name,
        &pos,
        " The "
    );


    add_text(
        vampire_name,
        &pos,
        vampire_last[last]
    );
}

void vampire_message(before, after)
char *before;
char *after;
{
    char line[80];
    char *src;
    int pos;


    pos = 0;


    src = before;

    while (*src &&
           pos < 78)
        line[pos++] = *src++;


    src = vampire_name;

    while (*src &&
           pos < 78)
        line[pos++] = *src++;


    src = after;

    while (*src &&
           pos < 78)
        line[pos++] = *src++;


    line[pos] = 0;


    show_message(line);
}

