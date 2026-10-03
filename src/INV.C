#include "game.h"


static char inventory[INVENTORY_SIZE];
static unsigned char inventory_count[INVENTORY_SIZE];
static unsigned char inventory_equipped[INVENTORY_SIZE];
static unsigned int inventory_power[INVENTORY_SIZE];


static int equippable_item();


void inventory_init()
{
    int i;


    for (i = 0;
         i < INVENTORY_SIZE;
         ++i) {

        inventory[i] = 0;
        inventory_count[i] = 0;
        inventory_equipped[i] = 0;
        inventory_power[i] = 0;
    }
}


static unsigned int item_power_for_level(level_number)
int level_number;
{
    if (level_number >= 0 &&
        level_number <= 994)
        return
            ((unsigned int)level_number / 2) + 1;


    return 1;
}


static int number_width(n)
unsigned int n;
{
    if (n >= 100)
        return 3;

    if (n >= 10)
        return 2;

    return 1;
}


static int equipped_slot(item)
char item;
{
    int i;


    for (i = 0;
         i < INVENTORY_SIZE;
         ++i) {

        if (inventory[i] == item &&
            inventory_equipped[i])
            return i;
    }


    return -1;
}


static int equipped_power(item)
char item;
{
    int slot;


    slot =
        equipped_slot(item);


    if (slot < 0)
        return 0;


    if (inventory_power[slot] == 0)
        return 1;


    return (int)inventory_power[slot];
}


int inventory_sword_power()
{
    return equipped_power('!');
}


int inventory_shield_power()
{
    return equipped_power('*');
}


int inventory_armour_power()
{
    return equipped_power('A');
}


int inventory_crown_equipped()
{
    if (equipped_slot('K') >= 0)
        return 1;

    return 0;
}


int inventory_has_crown()
{
    int i;


    for (i = 0;
         i < INVENTORY_SIZE;
         ++i) {

        if (inventory[i] == 'K')
            return 1;
    }


    return 0;
}


static char *item_name(c)
char c;
{
    if (c == '!')
        return "Sword";

    if (c == '*')
        return "Shield";

    if (c == 'A')
        return "Armour";

    if (c == 'p')
        return "Potion";

    if (c == 'f')
        return "Food";

    if (c == 'K')
        return "Bone Crown";

    return "Item";
}


static int stackable_item(c)
char c;
{
    if (c == 'p' || c == 'f')
        return 1;

    return 0;
}


static int inventory_stack_slot(c)
char c;
{
    int i;


    if (!stackable_item(c))
        return -1;


    for (i = 0;
         i < INVENTORY_SIZE;
         ++i) {

        if (inventory[i] == c &&
            inventory_count[i] < 99)
            return i;
    }


    return -1;
}


static int first_empty_inventory_slot()
{
    int i;


    for (i = 0;
         i < INVENTORY_SIZE;
         ++i) {

        if (inventory[i] == 0)
            return i;
    }


    return -1;
}


static void show_inventory_slot(slot)
int slot;
{
    char *name;
    int i;
    int n;


    if (slot < 0 ||
        slot >= INVENTORY_SIZE)
        return;


    pcw_goto(
        PANEL_X,
        INV_FIRST_Y + slot
    );


    putch('1' + slot);


    if (inventory[slot] != 0 &&
        inventory_equipped[slot])
        putch('*');
    else
        putch(' ');


    if (inventory[slot] == 0) {

        name = "{ empty }";

    } else {

        name = item_name(
            inventory[slot]
        );
    }


    n = 0;


    while (*name && n < 10) {

        putch(*name++);

        ++n;
    }


    if ((inventory[slot] == '!' ||
         inventory[slot] == '*' ||
         inventory[slot] == 'A') &&
        n < 10) {

        putch(' ');
        ++n;


        if (n < 10) {

            putch('+');
            ++n;


            pcw_number(
                inventory_power[slot]
            );


            n += number_width(
                inventory_power[slot]
            );
        }
    }


    if (inventory[slot] != 0 &&
        stackable_item(inventory[slot]) &&
        inventory_count[slot] > 1) {

        if (n < 10) {
            putch(' ');
            ++n;
        }

        if (n < 10) {
            putch('x');
            ++n;
        }

        if (inventory_count[slot] >= 10) {

            if (n < 10) {
                putch('0' +
                    (inventory_count[slot] / 10));
                ++n;
            }

            if (n < 10) {
                putch('0' +
                    (inventory_count[slot] % 10));
                ++n;
            }

        } else {

            if (n < 10) {
                putch('0' +
                    inventory_count[slot]);
                ++n;
            }
        }
    }


    for (i = n; i < 10; ++i)
        putch(' ');
}


static int offer_better_equipment(
    item,
    power
)
char item;
unsigned int power;
{
    char key;
    char *name;
    char prompt[48];
    int slot;
    int n;


    slot =
        equipped_slot(item);


    if (slot < 0)
        return 0;


    if (power <=
        inventory_power[slot])
        return 0;


    name = item_name(item);
    n = 0;


    prompt[n++] = 'R';
    prompt[n++] = 'e';
    prompt[n++] = 'p';
    prompt[n++] = 'l';
    prompt[n++] = 'a';
    prompt[n++] = 'c';
    prompt[n++] = 'e';
    prompt[n++] = ' ';
    prompt[n++] = 'e';
    prompt[n++] = 'q';
    prompt[n++] = 'u';
    prompt[n++] = 'i';
    prompt[n++] = 'p';
    prompt[n++] = 'p';
    prompt[n++] = 'e';
    prompt[n++] = 'd';
    prompt[n++] = ' ';


    while (*name && n < 34)
        prompt[n++] = *name++;


    prompt[n++] = '?';
    prompt[n++] = ' ';
    prompt[n++] = '(';
    prompt[n++] = 'y';
    prompt[n++] = '/';
    prompt[n++] = 'n';
    prompt[n++] = ')';
    prompt[n] = 0;


    show_message(prompt);


    key = getch();


    clear_message();


    if (key != 'y' &&
        key != 'Y')
        return 0;


    inventory_power[slot] =
        power;


    show_inventory_slot(slot);


    return 1;
}


int offer_item_pickup(x, y, level_number)
int x;
int y;
int level_number;
{
    char key;
    char item;
    char *name;
    char prompt[40];
    int slot;
    int n;
    unsigned int power;


    if (x < 0 || y < 0)
        return 0;


    if (x >= level_w || y >= level_h)
        return 0;


    item = level[y][x];


    if (!item_char(item))
        return 0;


    name = item_name(item);
    n = 0;


    prompt[n++] = 'P';
    prompt[n++] = 'i';
    prompt[n++] = 'c';
    prompt[n++] = 'k';
    prompt[n++] = ' ';
    prompt[n++] = 'u';
    prompt[n++] = 'p';
    prompt[n++] = ' ';


    while (*name && n < 28)
        prompt[n++] = *name++;


    prompt[n++] = '?';
    prompt[n++] = ' ';
    prompt[n++] = '(';
    prompt[n++] = 'y';
    prompt[n++] = '/';
    prompt[n++] = 'n';
    prompt[n++] = ')';
    prompt[n] = 0;


    show_message(prompt);


    key = getch();


    clear_message();


    if (key != 'y' &&
        key != 'Y')
        return 0;


    power = 0;


    if (equippable_item(item) &&
        item != 'K') {

        power =
            item_power_for_level(
                level_number
            );


        if (offer_better_equipment(
                item,
                power)) {

            level[y][x] = '.';

            return 3;
        }
    }


    slot =
        inventory_stack_slot(item);


    if (slot >= 0) {

        ++inventory_count[slot];

        level[y][x] = '.';

        show_inventory_slot(slot);

        return 1;
    }


    slot =
        first_empty_inventory_slot();


    if (slot < 0)
        return 2;


    inventory[slot] = item;
    inventory_count[slot] = 1;


    if (equippable_item(item) &&
        item != 'K')
        inventory_power[slot] =
            power;
    else
        inventory_power[slot] = 0;


    level[y][x] = '.';


    show_inventory_slot(slot);


    return 1;
}


static int equippable_item(c)
char c;
{
    if (c == '!' ||
        c == '*' ||
        c == 'A' ||
        c == 'K')
        return 1;

    return 0;
}


static void inventory_use(slot)
int slot;
{
    int heal;


    if (slot < 0 ||
        slot >= INVENTORY_SIZE)
        return;


    if (inventory[slot] == 0)
        return;


    if (inventory[slot] == 'p') {

        if (inventory_count[slot] > 1) {

            --inventory_count[slot];

        } else {

            inventory[slot] = 0;
            inventory_count[slot] = 0;
            inventory_power[slot] = 0;
        }


        show_inventory_slot(slot);


        player_drink_potion();


        do_turn();

        return;
    }


    if (inventory[slot] == 'f') {

        if (player_stamina >=
            player_max_stamina) {

            show_message(
                "You are not hungry."
            );

            return;
        }


        heal =
            2 +
            (rand() % 4) + 1;


        player_heal(heal);


        if (inventory_count[slot] > 1) {

            --inventory_count[slot];

        } else {

            inventory[slot] = 0;
            inventory_count[slot] = 0;
            inventory_power[slot] = 0;
        }


        show_inventory_slot(slot);


        show_message(
            "You eat the food."
        );


        do_turn();

        return;
    }


    show_message(
        "That item has no use yet."
    );
}


static void inventory_drop(slot)
int slot;
{
    char item;


    if (slot < 0 ||
        slot >= INVENTORY_SIZE)
        return;


    if (inventory[slot] == 0)
        return;


    /*
     * Items are stored in the terrain map,
     * so only ordinary floor can accept a
     * dropped item without destroying some
     * other map feature.
     */

    if (level[player_y][player_x] != '.') {

        show_message(
            "You cannot drop anything here."
        );

        return;
    }


    item = inventory[slot];


    /*
     * Dropped equipment is always unequipped.
     */

    inventory_equipped[slot] = 0;


    level[player_y][player_x] = item;


    if (inventory_count[slot] > 1) {

        --inventory_count[slot];

    } else {

        inventory[slot] = 0;
        inventory_count[slot] = 0;
        inventory_power[slot] = 0;
    }


    show_inventory_slot(slot);


    show_message(
        "You drop the item."
    );


    do_turn();
}


static void inventory_equip_toggle(slot)
int slot;
{
    if (slot < 0 ||
        slot >= INVENTORY_SIZE)
        return;


    if (inventory[slot] == 0)
        return;


    if (!equippable_item(inventory[slot])) {

        show_message(
            "You cannot equip that."
        );

        return;
    }


    if (inventory_equipped[slot]) {

        inventory_equipped[slot] = 0;
        show_inventory_slot(slot);

        show_message(
            "You unequip it."
        );

        do_turn();

        return;
    }


    if (equippable_item(
            inventory[slot])) {

        int i;


        for (i = 0;
             i < INVENTORY_SIZE;
             ++i) {

            if (i != slot &&
                inventory[i] ==
                    inventory[slot] &&
                inventory_equipped[i]) {

                inventory_equipped[i] = 0;
                show_inventory_slot(i);
            }
        }
    }


    inventory_equipped[slot] = 1;
    show_inventory_slot(slot);


    show_message(
        "You equip it."
    );


    do_turn();
}


void inventory_action(slot)
int slot;
{
    char key;
    char item;
    char *name;
    char prompt[72];
    int n;


    if (slot < 0 ||
        slot >= INVENTORY_SIZE)
        return;


    item = inventory[slot];


    if (item == 0) {

        show_message(
            "That inventory slot is empty."
        );

        return;
    }


    name = item_name(item);
    n = 0;


    while (*name && n < 24)
        prompt[n++] = *name++;


    prompt[n++] = ':';
    prompt[n++] = ' ';
    prompt[n++] = '(';
    prompt[n++] = 'u';
    prompt[n++] = ')';
    prompt[n++] = 's';
    prompt[n++] = 'e';
    prompt[n++] = ' ';
    prompt[n++] = '(';
    prompt[n++] = 'd';
    prompt[n++] = ')';
    prompt[n++] = 'r';
    prompt[n++] = 'o';
    prompt[n++] = 'p';


    if (equippable_item(item)) {

        prompt[n++] = ' ';
        prompt[n++] = '(';
        prompt[n++] = 'e';
        prompt[n++] = ')';


        if (inventory_equipped[slot]) {

            prompt[n++] = 'u';
            prompt[n++] = 'n';
            prompt[n++] = 'e';
            prompt[n++] = 'q';
            prompt[n++] = 'u';
            prompt[n++] = 'i';
            prompt[n++] = 'p';

        } else {

            prompt[n++] = 'e';
            prompt[n++] = 'q';
            prompt[n++] = 'u';
            prompt[n++] = 'i';
            prompt[n++] = 'p';
        }
    }


    prompt[n] = 0;


    show_message(prompt);


    key = getch();


    clear_message();


    if (key == 'u' ||
        key == 'U') {

        inventory_use(slot);

        return;
    }


    if (key == 'd' ||
        key == 'D') {

        inventory_drop(slot);

        return;
    }


    if ((key == 'e' ||
         key == 'E') &&
        equippable_item(item)) {

        inventory_equip_toggle(slot);

        return;
    }
}

