#include "game.h"


#define STEP_CANCELLED  5


char copyright_notice[] =
    "(c) 2026 Tony Blews tonyblews@gmail.com";


int player_x;
int player_y;


int player_skill;
int player_stamina;
int player_gold;
int player_keys;
int player_kills;

static unsigned int player_zombies;
static unsigned int player_skeletons;

char player_name[31];


static int game_over;
static int escaped;

static int message_active;

static unsigned int turn_count;

#ifdef VT100_80
static unsigned int seed_noise;
#endif
static int current_level;


static int step_player();
static int confirm_action();

static void run_player();
void do_turn();
static void make_level_name();
static int change_level();

static void win_game();
static int try_exit();
static void show_death_grave();
static void quit_game();
static void confirm_quit();

static void seed_random();
#ifdef VT100_80
static void wait_seed_key();
#endif
static void create_player();
static void ask_player_name();

static void show_splash();

static void show_stats();
static void show_stamina();
static void show_gold();
static void show_keys();

static void pick_up_gold();
static void pick_up_key();
static int search_bones();

void show_message();
void clear_message();

static void close_adjacent_door();
static void open_adjacent_door();
static void lock_adjacent_door();
static void draw_door();


#ifdef VT100_80

static void show_splash()
{
    pcw_text(16, 5,  "BBBB   OOO  N   N EEEEE K  K  EEEEE EEEEE PPPP");
    pcw_text(16, 6,  "B   B O   O NN  N E     K K   E     E     P   P");
    pcw_text(16, 7,  "BBBB  O   O N N N EEEE  KK    EEEE  EEEE  PPPP");
    pcw_text(16, 8,  "B   B O   O N  NN E     K K   E     E     P");
    pcw_text(16, 9,  "BBBB   OOO  N   N EEEEE K  K  EEEEE EEEEE P");


    pcw_text(
        22,
        13,
        "Generic CP/M 2.2 / VT100 edition"
    );


    pcw_text(
        20,
        16,
        "(c) 2026 Tony Blews tonyblews\100gmail.com"
    );


    pcw_text(
        27,
        20,
        "Press any key to enter..."
    );
}

#else

static void show_splash()
{
    pcw_text(
        0,
        6,
        "     \047||\047\047|.    ..|\047\047||   \047|.   \047|\047 \047||\047\047\047\047|"
    );

    pcw_text(
        44,
        6,
        "  \047||\047  |\047  \047||\047\047\047\047|  \047||\047\047\047\047|  \047||\047\047|."
    );


    pcw_text(
        0,
        7,
        "      ||   ||  .|\047    ||   |\047|   |   ||  .  "
    );

    pcw_text(
        44,
        7,
        "   || .\047     ||  .     ||  .     ||   ||"
    );


    pcw_text(
        0,
        8,
        "      ||\047\047\047|.  ||      ||  | \047|. |   ||\047\047|  "
    );

    pcw_text(
        44,
        8,
        "   ||\047|.     ||\047\047|     ||\047\047|     ||...|\047"
    );


    pcw_text(
        0,
        9,
        "      ||    || \047|.     ||  |   |||   ||     "
    );

    pcw_text(
        44,
        9,
        "   ||  ||    ||        ||        ||"
    );


    pcw_text(
        0,
        10,
        "     .||...|\047   \047\047|...|\047  .|.   \047|  .||....."
    );

    pcw_text(
        44,
        10,
        "| .||.  ||. .||.....| .||.....| .||."
    );


    pcw_text(
        25,
        16,
        "(c) 2026 Tony Blews tonyblews\100gmail.com"
    );


    pcw_text(
        32,
        21,
        "Press any key to enter..."
    );
}

#endif


static void ask_player_name()
{
    char key;
    int len;


    len = 0;
    player_name[0] = 0;


    pcw_clear();


    pcw_text(
        29,
        13,
        "What is your name?"
    );


    pcw_text(
        31,
        15,
        "Name: "
    );


    pcw_cursor(1);


    pcw_goto(
        37,
        15
    );


    for (;;) {

        key = getch();


        if (key == 13 ||
            key == 10) {

            if (len > 0)
                break;


            continue;
        }


        if (key == 8 ||
            key == 127) {

            if (len > 0) {

                --len;

                player_name[len] = 0;


                putch(8);
                putch(' ');
                putch(8);
            }


            continue;
        }


        if (key >= 32 &&
            key <= 126 &&
            len < 30) {

            player_name[len] = key;

            ++len;

            player_name[len] = 0;


            putch(key);
        }
    }


    pcw_cursor(0);
}


#ifdef VT100_80

static void wait_seed_key()
{
    int c;


    seed_noise = 0x1357;


    for (;;) {

        c = bdos(6, 255);


        seed_noise =
            (seed_noise << 1) ^
            (seed_noise >> 3) ^
            0x41;


        if (c != 0)
            break;
    }
}


static void seed_random()
{
    unsigned int seed;
    int i;


    seed = seed_noise;


    for (i = 0; player_name[i]; ++i) {

        seed =
            (seed * 33) ^
            (unsigned int)
                (unsigned char)player_name[i];
    }


    seed ^=
        ((unsigned int)player_x << 8);

    seed ^=
        (unsigned int)player_y;


    if (seed == 0)
        seed = 1;


    srand(seed);
}

#else

static void seed_random()
{
    unsigned char dt[4];

    unsigned int seed;
    unsigned int seconds;


    seconds = bdos(105, dt);


    seed =
        ((unsigned int)dt[0] << 8);


    seed ^=
        (unsigned int)dt[1];


    seed ^=
        ((unsigned int)dt[2] << 4);


    seed ^=
        (unsigned int)dt[3];


    seed ^=
        (seconds & 255);


    srand(seed);
}

#endif


static void create_player()
{
    player_skill =
        10 +
        (rand() % 4) + 1;


    player_stamina =
        10 +
        (rand() % 6) + 1;


    player_health_init();


    player_gold = 0;
    player_keys = 0;
    player_kills = 0;

    player_zombies = 0;
    player_skeletons = 0;


    inventory_init();
}


static void show_stats()
{
    pcw_text(
        PANEL_X,
        STAT_SKILL_Y,
        " SKILL      "
    );


    pcw_goto(
        PANEL_VALUE_X,
        STAT_SKILL_Y
    );


    pcw_number2(
        player_skill
    );


    pcw_text(
        PANEL_X,
        STAT_STAMINA_Y,
        " STAMINA    "
    );


    pcw_goto(
        PANEL_VALUE_X,
        STAT_STAMINA_Y
    );


    pcw_number2(
        player_stamina
    );


    pcw_text(
        PANEL_X,
        STAT_GOLD_Y,
        " GOLD       "
    );


    pcw_goto(
        PANEL_VALUE_X,
        STAT_GOLD_Y
    );


    pcw_number2(
        player_gold
    );


    pcw_text(
        PANEL_X,
        STAT_KEYS_Y,
        " KEYS       "
    );


    pcw_goto(
        PANEL_VALUE_X,
        STAT_KEYS_Y
    );


    pcw_number2(
        player_keys
    );
}


static void show_stamina()
{
    pcw_goto(
        PANEL_VALUE_X,
        STAT_STAMINA_Y
    );


    pcw_number2(
        player_stamina
    );
}


static void show_gold()
{
    pcw_goto(
        PANEL_VALUE_X,
        STAT_GOLD_Y
    );


    pcw_number2(
        player_gold
    );
}


static void show_keys()
{
    pcw_goto(
        PANEL_VALUE_X,
        STAT_KEYS_Y
    );


    pcw_number2(
        player_keys
    );
}


void show_message(text)
char *text;
{
    if (message_active)
        pcw_clear_messages();


    pcw_text(
        MESSAGE_X,
        MESSAGE_Y,
        text
    );


    message_active = 1;
}


void clear_message()
{
    if (!message_active)
        return;


    pcw_clear_messages();


    message_active = 0;
}


static void make_level_name(number, name)
int number;
char *name;
{
    char *base;
    int i;


    base = "LEVEL.000";


    for (i = 0; base[i]; ++i)
        name[i] = base[i];

    name[i] = 0;


    name[6] = '0' + ((number / 100) % 10);
    name[7] = '0' + ((number / 10) % 10);
    name[8] = '0' + (number % 10);
}


static int change_level(stair)
char stair;
{
    int target;
    int entry_x;
    int entry_y;
    int state_result;

    char expected;
    char name[10];


    entry_x = player_x;
    entry_y = player_y;


    if (stair == '>') {

        target = current_level + 1;

        if (target > 999)
            target = 0;

        expected = '<';

    } else if (stair == '<') {

        target = current_level - 1;

        if (target < 0)
            target = 999;

        expected = '>';

    } else {

        return 0;
    }


    make_level_name(
        target,
        name
    );


    show_message(
        "Loading level..."
    );


    /*
     * The PCW build keeps its original pre-load validation.
     *
     * The VT100 build validates after loading the destination
     * into the logical level array.  This deliberately keeps
     * viewport coordinates completely out of stair matching.
     */

#ifndef VT100_80

    if (!level_entry_ok(
            name,
            target,
            entry_x,
            entry_y,
            expected)) {

        show_message(
            "No matching stair on that level."
        );

        return 0;
    }

#endif


    /*
     * Snapshot the level we are leaving before replacing it.
     */

    if (!save_level_state(current_level)) {

        show_message(
            "Cannot save current level."
        );

        return 0;
    }


    /*
     * A previously visited level is restored from BKxxx.TMP.
     * Otherwise load its original encoded LEVEL.xxx data.
     */

    state_result =
        load_level_state(target);


    if (state_result < 0) {

        show_message(
            "Invalid level state file."
        );

        return 0;
    }


    if (state_result == 0) {

        if (!load_level(
                name,
                target)) {

            /*
             * The current level was safely snapshotted,
             * so restore it if the destination cannot load.
             */

            load_level_state(current_level);

            player_x = entry_x;
            player_y = entry_y;


            show_message(
                "Cannot load destination level."
            );

            return 0;
        }


        visibility_reset();
    }


#ifdef VT100_80

    /*
     * Check the stair against logical dungeon coordinates
     * only after the destination has been loaded.  The
     * 64 x 18 terminal viewport never participates in this
     * test.
     */

    if (entry_x < 0 ||
        entry_y < 0 ||
        entry_x >= level_w ||
        entry_y >= level_h ||
        level[entry_y][entry_x] != expected) {

        load_level_state(current_level);

        player_x = entry_x;
        player_y = entry_y;


        show_message(
            "No matching stair on that level."
        );

        return 0;
    }

#endif


    current_level = target;

    player_x = entry_x;
    player_y = entry_y;


    pcw_clear_map();

#ifdef VT100_80

    pcw_view_update(
        player_x,
        player_y
    );

#endif


    reveal_position(
        player_x,
        player_y
    );


    draw_player();


    if (stair == '>')
        show_message("You go down.");
    else
        show_message("You go up.");


    return 1;
}


static void pick_up_gold()
{
    if (level[player_y][player_x] == '$') {

        level[player_y][player_x] = '.';


        ++player_gold;


        show_gold();


        show_message(
            "You find some gold."
        );
    }
}


static void pick_up_key()
{
    if (level[player_y][player_x] == 'k') {

        level[player_y][player_x] = '.';


        ++player_keys;


        show_keys();


        show_message(
            "You pick up a key."
        );
    }
}


static int search_bones(x, y)
int x;
int y;
{
    int roll;


    if (x < 0 || y < 0)
        return 0;


    if (x >= level_w || y >= level_h)
        return 0;


    if (level[y][x] != 'x')
        return 0;


    roll = rand() % 100;


    if (roll < 10) {

        level[y][x] = '.';


        if (monster_add('S', x, y,
                current_level))
            return 1;


        return 3;
    }


    if (roll < 60) {

        level[y][x] = '$';

        return 2;
    }


    level[y][x] = '.';


    return 3;
}


void do_turn()
{
    ++turn_count;


    if (game_over) {

        monster_clear_skip();

        return;
    }


    monster_move_skeletons();


    if (!game_over)
        monster_move_bats();


    if (!game_over &&
        (turn_count & 1) == 0) {

        monster_move_zombies();
    }


    if (!game_over)
        player_potion_turn();


    monster_clear_skip();
}


static int confirm_action(text)
char *text;
{
    char key;


    show_message(text);


    key = getch();


    clear_message();


    if (key == 'y' ||
        key == 'Y')
        return 1;


    return 0;
}


static void draw_door(x, y, c)
int x;
int y;
char c;
{
    pcw_map_char(
        x,
        y,
        c
    );
}


static void close_adjacent_door()
{
    int x;
    int y;


    x = player_x;
    y = player_y - 1;


    if (y >= 0 &&
        level[y][x] == '/') {

        clear_message();

        level[y][x] = '+';

        draw_door(x, y, '+');

        show_message(
            "You close the door."
        );

        do_turn();

        return;
    }


    x = player_x;
    y = player_y + 1;


    if (y < level_h &&
        level[y][x] == '/') {

        clear_message();

        level[y][x] = '+';

        draw_door(x, y, '+');

        show_message(
            "You close the door."
        );

        do_turn();

        return;
    }


    x = player_x - 1;
    y = player_y;


    if (x >= 0 &&
        level[y][x] == '/') {

        clear_message();

        level[y][x] = '+';

        draw_door(x, y, '+');

        show_message(
            "You close the door."
        );

        do_turn();

        return;
    }


    x = player_x + 1;
    y = player_y;


    if (x < level_w &&
        level[y][x] == '/') {

        clear_message();

        level[y][x] = '+';

        draw_door(x, y, '+');

        show_message(
            "You close the door."
        );

        do_turn();

        return;
    }


    show_message(
        "No open door nearby."
    );
}


static void open_adjacent_door()
{
    int x;
    int y;


    x = player_x;
    y = player_y - 1;


    if (y >= 0) {

        if (level[y][x] == '+') {

            clear_message();

            level[y][x] = '/';

            draw_door(x, y, '/');

            show_message(
                "You open the door."
            );

            do_turn();

            return;
        }


        if (level[y][x] == 'L') {

            if (player_keys <= 0) {

                show_message(
                    "LOCKED"
                );

                do_turn();

                return;
            }


            if (!confirm_action("Unlock door? (y/n)"))
                return;


            --player_keys;

            show_keys();

            level[y][x] = '/';

            draw_door(x, y, '/');

            show_message(
                "You unlock the door."
            );

            do_turn();

            return;
        }
    }


    x = player_x;
    y = player_y + 1;


    if (y < level_h) {

        if (level[y][x] == '+') {

            clear_message();

            level[y][x] = '/';

            draw_door(x, y, '/');

            show_message(
                "You open the door."
            );

            do_turn();

            return;
        }


        if (level[y][x] == 'L') {

            if (player_keys <= 0) {

                show_message(
                    "LOCKED"
                );

                do_turn();

                return;
            }


            if (!confirm_action("Unlock door? (y/n)"))
                return;


            --player_keys;

            show_keys();

            level[y][x] = '/';

            draw_door(x, y, '/');

            show_message(
                "You unlock the door."
            );

            do_turn();

            return;
        }
    }


    x = player_x - 1;
    y = player_y;


    if (x >= 0) {

        if (level[y][x] == '+') {

            clear_message();

            level[y][x] = '/';

            draw_door(x, y, '/');

            show_message(
                "You open the door."
            );

            do_turn();

            return;
        }


        if (level[y][x] == 'L') {

            if (player_keys <= 0) {

                show_message(
                    "LOCKED"
                );

                do_turn();

                return;
            }


            if (!confirm_action("Unlock door? (y/n)"))
                return;


            --player_keys;

            show_keys();

            level[y][x] = '/';

            draw_door(x, y, '/');

            show_message(
                "You unlock the door."
            );

            do_turn();

            return;
        }
    }


    x = player_x + 1;
    y = player_y;


    if (x < level_w) {

        if (level[y][x] == '+') {

            clear_message();

            level[y][x] = '/';

            draw_door(x, y, '/');

            show_message(
                "You open the door."
            );

            do_turn();

            return;
        }


        if (level[y][x] == 'L') {

            if (player_keys <= 0) {

                show_message(
                    "LOCKED"
                );

                do_turn();

                return;
            }


            if (!confirm_action("Unlock door? (y/n)"))
                return;


            --player_keys;

            show_keys();

            level[y][x] = '/';

            draw_door(x, y, '/');

            show_message(
                "You unlock the door."
            );

            do_turn();

            return;
        }
    }


    show_message(
        "No closed door nearby."
    );
}


static void lock_adjacent_door()
{
    int x;
    int y;


    if (player_keys <= 0) {

        show_message(
            "You have no key."
        );

        return;
    }


    x = player_x;
    y = player_y - 1;


    if (y >= 0 &&
        level[y][x] == '+') {

        if (!confirm_action("Lock door? (y/n)"))
            return;


        --player_keys;

        show_keys();

        level[y][x] = 'L';


        show_message(
            "You lock the door."
        );

        do_turn();

        return;
    }


    x = player_x;
    y = player_y + 1;


    if (y < level_h &&
        level[y][x] == '+') {

        if (!confirm_action("Lock door? (y/n)"))
            return;


        --player_keys;

        show_keys();

        level[y][x] = 'L';


        show_message(
            "You lock the door."
        );

        do_turn();

        return;
    }


    x = player_x - 1;
    y = player_y;


    if (x >= 0 &&
        level[y][x] == '+') {

        if (!confirm_action("Lock door? (y/n)"))
            return;


        --player_keys;

        show_keys();

        level[y][x] = 'L';


        show_message(
            "You lock the door."
        );

        do_turn();

        return;
    }


    x = player_x + 1;
    y = player_y;


    if (x < level_w &&
        level[y][x] == '+') {

        if (!confirm_action("Lock door? (y/n)"))
            return;


        --player_keys;

        show_keys();

        level[y][x] = 'L';


        show_message(
            "You lock the door."
        );

        do_turn();

        return;
    }


    show_message(
        "No closed door nearby."
    );
}


int game_combat_monster(x, y)
int x;
int y;
{
    int player_attack;
    int monster_attack;
    int monster_skill;

    int sword_power;
    int shield_power;
    int armour_power;

    int player_die1;
    int player_die2;

    int block_attack;
    int damage;
    int critical;
    int dead;

    char monster;


    player_rest_reset();


    monster =
        monster_char(x, y);


    if (!monster_blocks_at(x, y))
        return 0;


    if (inventory_crown_equipped()) {

        if (monster == 'S') {

            show_message(
                "The skeleton ignores you."
            );

            return 0;
        }


        if (monster == 'Z') {

            show_message(
                "The zombie ignores you."
            );

            return 0;
        }
    }


    monster_skill =
        monster_skill_at(x, y);


    sword_power =
        inventory_sword_power();

    shield_power =
        inventory_shield_power();

    armour_power =
        inventory_armour_power();


    player_die1 =
        (rand() % 6) + 1;

    player_die2 =
        (rand() % 6) + 1;


    critical =
        (player_die1 == 6 &&
         player_die2 == 6);


    player_attack =
        player_skill +
        sword_power +
        player_die1 +
        player_die2;


    monster_attack =
        monster_skill +
        (rand() % 6) + 1 +
        (rand() % 6) + 1;


    /*
     * Player wins.
     *
     * Normal damage is 1D4 plus Sword.
     * A natural double six is an automatic
     * win and rolls 2D4 plus Sword.
     */

    if (critical ||
        player_attack > monster_attack) {

        damage =
            (rand() % 4) + 1 +
            sword_power;


        if (critical)
            damage +=
                (rand() % 4) + 1;


        dead =
            monster_damage_at(
                x,
                y,
                damage
            );


        if (monster == 'V' &&
            monster_char(x, y) == '"') {

            if (level[y][x] == 'K')
                vampire_message(
                    "",
                    " becomes a bat and drops the Bone Crown."
                );
            else
                vampire_message(
                    "",
                    " becomes a bat."
                );


            return 0;
        }


        if (dead) {

            ++player_kills;


            if (monster == 'S')
                ++player_skeletons;

            if (monster == 'Z')
                ++player_zombies;


            if (critical &&
                monster == 'V')
                vampire_message(
                    "Critical hit! ",
                    " destroyed."
                );
            else if (critical)
                show_message(
                    "Critical hit! Enemy destroyed."
                );
            else if (monster == 'S')
                show_message(
                    "You destroy the skeleton."
                );
            else if (monster == 'Z')
                show_message(
                    "You destroy the zombie."
                );
            else
                vampire_message(
                    "You destroy ",
                    "."
                );


            draw_old_position(x, y);

            return 1;
        }


        if (critical &&
            monster == 'V')
            vampire_message(
                "Critical hit on ",
                "!"
            );
        else if (critical)
            show_message(
                "Critical hit!"
            );
        else if (monster == 'S')
            show_message(
                "You hit the skeleton."
            );
        else if (monster == 'Z')
            show_message(
                "You hit the zombie."
            );
        else
            vampire_message(
                "You hit ",
                "."
            );


        return 0;
    }


    /*
     * Monster wins.
     *
     * An equipped Shield gets one second
     * SKILL roll against the monster's
     * original attack total.  Ties fail.
     */

    if (monster_attack >
        player_attack) {

        if (shield_power > 0) {

            block_attack =
                player_skill +
                shield_power +
                (rand() % 6) + 1 +
                (rand() % 6) + 1;


            if (block_attack >
                monster_attack) {

                if (monster == 'V')
                    vampire_message(
                        "You block ",
                        "'s attack."
                    );
                else
                    show_message(
                        "You block the attack."
                    );


                return 0;
            }
        }


        if (monster == 'Z')
            damage =
                (rand() % 6) + 1;
        else
            damage =
                (rand() % 4) + 1;


        if (armour_power > 0) {

            if ((unsigned int)(
                    rand() %
                    (armour_power + 3)) <
                (unsigned int)armour_power) {

                --damage;


                if (damage < 1)
                    damage = 1;
            }
        }


        player_stamina -= damage;


        if (player_stamina < 0)
            player_stamina = 0;


        show_stamina();


        if (player_stamina == 0) {

            game_over = 1;
            escaped = 0;


            show_death_grave();


            return -1;
        }


        if (monster == 'S')
            show_message(
                "The skeleton hits you."
            );
        else if (monster == 'Z')
            show_message(
                "The zombie hits you."
            );
        else
            vampire_message(
                "",
                " hits you."
            );


        return 0;
    }


    if (monster == 'S')
        show_message(
            "You and the skeleton draw."
        );
    else if (monster == 'Z')
        show_message(
            "You and the zombie draw."
        );
    else
        vampire_message(
            "You and ",
            " draw."
        );


    return 0;
}

static int step_player(dx, dy)
int dx;
int dy;
{
    int old_x;
    int old_y;

    int nx;
    int ny;

    int door;
    int locked;
    int opened;
    int unlocked;

    char monster;

    int bone_result;
    int item_result;


    nx = player_x + dx;
    ny = player_y + dy;


    /*
     * Stairs are confirmed before the player
     * steps onto them.  Declining cancels the
     * move and does not consume a turn.
     */

    if (nx >= 0 &&
        ny >= 0 &&
        nx < level_w &&
        ny < level_h) {

        if (level[ny][nx] == '>' ||
            level[ny][nx] == '<') {

            if (!confirm_action(
                    level[ny][nx] == '>' ?
                    "Go down steps? (y/n)" :
                    "Go up steps? (y/n)"))
                return STEP_CANCELLED;
        }
    }


    /*
     * Coffins are opened before movement.
     * Declining cancels the move and costs no turn.
     */

    if (nx >= 0 &&
        ny >= 0 &&
        nx < level_w &&
        ny < level_h &&
        level[ny][nx] == 'C') {

        if (!confirm_action("Open coffin? (y/n)"))
            return STEP_CANCELLED;


        if (!monster_add('V', nx, ny,
                current_level)) {

            show_message(
                "The coffin will not open."
            );

            return STEP_CANCELLED;
        }


        level[ny][nx] = '.';


        draw_old_position(nx, ny);


        vampire_message(
            "You wake ",
            "."
        );


        monster_skip_at(nx, ny);


        do_turn();

        return STEP_COMBAT;
    }


    /*
     * Stepping onto a pile of bones
     * searches it immediately.
     *
     * 10% becomes a skeleton.
     * 50% becomes gold.
     * 40% becomes ordinary floor.
     */

    bone_result =
        search_bones(nx, ny);


    if (bone_result == 1) {

        draw_old_position(nx, ny);


        show_message(
            "A skeleton rises from the bones!"
        );


        monster_skip_at(nx, ny);


        do_turn();

        return STEP_COMBAT;
    }


    /*
     * Items are passable terrain.  Trying to
     * enter an item square offers pickup first.
     * Declining, or having a full inventory,
     * leaves the item underneath the player.
     */

    item_result =
        offer_item_pickup(
            nx,
            ny,
            current_level
        );


    /*
     * Skeletons and zombies occupy
     * their own square and block the
     * player from entering it.
     *
     */

    monster =
        monster_char(nx, ny);


    if (monster_blocks_at(nx, ny)) {

        int combat_result;


        combat_result =
            game_combat_monster(
                nx,
                ny
            );


        if (combat_result < 0) {

            do_turn();

            return STEP_COMBAT;
        }


        if (combat_result == 0) {

            monster_skip_at(
                nx,
                ny
            );


            do_turn();

            return STEP_COMBAT;
        }


        old_x = player_x;
        old_y = player_y;


        player_x = nx;
        player_y = ny;


        draw_old_position(
            old_x,
            old_y
        );


        reveal_position(
            player_x,
            player_y
        );


        draw_player();


        do_turn();


        if (game_over)
            return STEP_COMBAT;


        if (map_exit(
                player_x,
                player_y)) {

            if (try_exit())
                return STEP_EXIT;
        }


        if (level[player_y][player_x] == '>' ||
            level[player_y][player_x] == '<') {

            change_level(
                level[player_y][player_x]
            );

            return STEP_LEVEL;
        }


        return STEP_COMBAT;
    }


    door =
        map_closed_door(nx, ny);


    locked =
        map_locked_door(nx, ny);


    opened = 0;
    unlocked = 0;


    if (locked) {

        if (player_keys <= 0) {

            show_message(
                "LOCKED"
            );


            return STEP_BLOCKED;
        }


        if (!confirm_action("Unlock door? (y/n)"))
            return STEP_CANCELLED;


        --player_keys;


        show_keys();


        map_unlock_door(
            nx,
            ny
        );


        unlocked = 1;
    }


    else if (door) {

        map_open_door(
            nx,
            ny
        );


        opened = 1;
    }


    else {

        if (!map_passable(nx, ny))
            return STEP_BLOCKED;
    }


    clear_message();


    old_x = player_x;
    old_y = player_y;


    player_x = nx;
    player_y = ny;


    pick_up_gold();
    pick_up_key();


    if (item_result == 1) {

        show_message(
            "You pick it up."
        );

    } else if (item_result == 2) {

        show_message(
            "Inventory full."
        );

    } else if (item_result == 3) {

        show_message(
            "You equip the better item."
        );
    }


    if (bone_result == 3) {

        show_message(
            "You find nothing in the bones."
        );
    }


    if (unlocked) {

        show_message(
            "You unlock the door."
        );

    } else if (opened) {

        show_message(
            "You open the door."
        );
    }


    draw_old_position(
        old_x,
        old_y
    );


    reveal_position(
        player_x,
        player_y
    );


    draw_player();


    do_turn();


    if (game_over)
        return STEP_MOVED;


    if (map_exit(
            player_x,
            player_y)) {

        if (try_exit())
            return STEP_EXIT;
    }


    if (level[player_y][player_x] == '>' ||
        level[player_y][player_x] == '<') {

        change_level(
            level[player_y][player_x]
        );


        return STEP_LEVEL;
    }


    return STEP_MOVED;
}


static void run_player(dx, dy)
int dx;
int dy;
{
    int nx;
    int ny;


    while (!game_over) {

        nx = player_x + dx;
        ny = player_y + dy;


        /*
         * Running stops before a
         * skeleton or zombie.
         */

        if (monster_blocks_at(nx, ny))
            break;


        if (map_closed_door(nx, ny))
            break;


        if (map_locked_door(nx, ny))
            break;


        if (step_player(dx, dy)
            != STEP_MOVED)
            break;
    }
}


#ifdef VT100_80

static void show_death_grave()
{
    char name[31];
    int len;
    int name_x;


    len = 0;


    while (player_name[len] &&
           len < 30) {

        name[len] =
            player_name[len];


        if (name[len] >= 'a' &&
            name[len] <= 'z')
            name[len] -=
                ('a' - 'A');


        ++len;
    }


    name[len] = 0;


    /*
     * Same tombstone layout as the PCW version,
     * shifted up two rows to fit 80 x 24.
     */

    name_x =
        30 + ((30 - len) / 2);


    pcw_clear();
    pcw_cursor(0);


    pcw_text(
        11, 0,
        "                            _____  _____"
    );

    pcw_text(
        11, 1,
        "                           <     `/     |"
    );

    pcw_text(
        11, 2,
        "                            >          ("
    );

    pcw_text(
        11, 3,
        "                           |   _     _  |"
    );

    pcw_text(
        11, 4,
        "                           |  |_) | |_) |"
    );

    pcw_text(
        11, 5,
        "                           |  | \\ | |   |"
    );

    pcw_text(
        11, 6,
        "                           |            |"
    );

    pcw_text(
        11, 7,
        "            ______.______%_|            |__________  _____"
    );

    pcw_text(
        11, 8,
        "          _/                                       \\|     |"
    );

    pcw_text(
        11, 9,
        "         |                                               <"
    );

    pcw_text(
        name_x,
        9,
        name
    );

    pcw_text(
        11, 10,
        "         |_____.-._________              ____/|___________|"
    );

    pcw_text(
        11, 11,
        "                           |            |"
    );

    pcw_text(
        11, 12,
        "                           | GOLD       |"
    );

    pcw_goto(
        46, 12
    );

    pcw_number(
        player_gold
    );


    pcw_text(
        11, 13,
        "                           | KILLS      |"
    );

    pcw_goto(
        46, 13
    );

    pcw_number(
        player_kills
    );


    pcw_text(
        11, 14,
        "                           | TURNS      |"
    );

    pcw_goto(
        46, 14
    );

    pcw_number(
        turn_count
    );


    pcw_text(
        11, 15,
        "                           |   _        <"
    );

    pcw_text(
        11, 16,
        "                           |__/         |"
    );

    pcw_text(
        11, 17,
        "                           / `--.      |"
    );

    pcw_text(
        11, 18,
        "                          %|            |%"
    );

    pcw_text(
        11, 19,
        "                      |/.%%|          -< @%%%"
    );

    pcw_text(
        11, 20,
        "                      `\\%`@|     v      |@@%@%%"
    );

    pcw_text(
        11, 21,
        "                    .%%%@@@|%    |    % @@@%%@%%%%"
    );

    pcw_text(
        11, 22,
        "               _.%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@%%%%%%"
    );


    pcw_text(
        33, 23,
        "Press any key."
    );
}
#else

static void show_death_grave()
{
    char name[31];
    int len;
    int name_x;


    len = 0;


    while (player_name[len] &&
           len < 30) {

        name[len] =
            player_name[len];


        if (name[len] >= 'a' &&
            name[len] <= 'z')
            name[len] -=
                ('a' - 'A');


        ++len;
    }


    name[len] = 0;


    /*
     * The grave has a 30-column name
     * panel beginning at column 19.
     */

    name_x =
        30 + ((30 - len) / 2);


    pcw_clear();
    pcw_cursor(0);


    pcw_text(
        11, 2,
        "                            _____  _____"
    );

    pcw_text(
        11, 3,
        "                           <     `/     |"
    );

    pcw_text(
        11, 4,
        "                            >          ("
    );

    pcw_text(
        11, 5,
        "                           |   _     _  |"
    );

    pcw_text(
        11, 6,
        "                           |  |_) | |_) |"
    );

    pcw_text(
        11, 7,
        "                           |  | \\ | |   |"
    );

    pcw_text(
        11, 8,
        "                           |            |"
    );

    pcw_text(
        11, 9,
        "            ______.______%_|            |__________  _____"
    );

    pcw_text(
        11, 10,
        "          _/                                       \\|     |"
    );

    pcw_text(
        11, 11,
        "         |                                               <"
    );

    pcw_text(
        name_x,
        11,
        name
    );

    pcw_text(
        11, 12,
        "         |_____.-._________              ____/|___________|"
    );

    pcw_text(
        11, 13,
        "                           |            |"
    );

    pcw_text(
        11, 14,
        "                           | GOLD       |"
    );

    pcw_goto(
        46, 14
    );

    pcw_number(
        player_gold
    );


    pcw_text(
        11, 15,
        "                           | KILLS      |"
    );

    pcw_goto(
        46, 15
    );

    pcw_number(
        player_kills
    );


    pcw_text(
        11, 16,
        "                           | TURNS      |"
    );

    pcw_goto(
        46, 16
    );

    pcw_number(
        turn_count
    );


    pcw_text(
        11, 17,
        "                           |   _        <"
    );

    pcw_text(
        11, 18,
        "                           |__/         |"
    );

    pcw_text(
        11, 19,
        "                           / `--.      |"
    );

    pcw_text(
        11, 20,
        "                          %|            |%"
    );

    pcw_text(
        11, 21,
        "                      |/.%%|          -< @%%%"
    );

    pcw_text(
        11, 22,
        "                      `\\%`@|     v      |@@%@%%"
    );

    pcw_text(
        11, 23,
        "                    .%%%@@@|%    |    % @@@%%@%%%%"
    );

    pcw_text(
        11, 24,
        "               _.%%%%%%@@@@@@@@@@@@@@@@@@@@@@@@%%%%%%"
    );


    pcw_text(
        37, 27,
        "Press any key."
    );
}

#endif


static void win_game()
{
    game_over = 1;
    escaped = 1;


    pcw_victory(
        player_zombies,
        player_skeletons,
        (unsigned int) player_gold,
        turn_count
    );


    message_active = 0;
}


static int try_exit()
{
    if (!inventory_has_crown()) {

        show_message(
            "You need the Bone Crown."
        );

        return 0;
    }


    win_game();

    return 1;
}


static void quit_game()
{
    game_over = 1;
    escaped = 0;


    pcw_clear_messages();


    pcw_text(
        MESSAGE_X,
        MESSAGE_Y,
        "Quit after      turns."
    );


    pcw_goto(
        MESSAGE_X + 11,
        MESSAGE_Y
    );


    pcw_number(
        turn_count
    );


    message_active = 1;
}


static void confirm_quit()
{
    char key;


    show_message(
        "Really quit? (y/n)"
    );


    key = getch();


    if (key == 'y' ||
        key == 'Y') {

        quit_game();

        return;
    }


    clear_message();
}


int main()
{
    char key;
    int result;
    int slot;


    if (copyright_notice[0] == 0)
        return 1;


    pcw_clear();
    pcw_cursor(0);


    show_splash();


#ifdef VT100_80
    wait_seed_key();
#else
    getch();
#endif


    ask_player_name();


    pcw_clear();


    /*
     * A new game never inherits stale runtime snapshots
     * from an earlier run or an interrupted session.
     */

    level_state_cleanup();


    pcw_text(
        1,
        1,
        "Loading level..."
    );


    if (!load_level("LEVEL.000", 0)) {

        pcw_text(
            1,
            1,
            "Cannot load or invalid LEVEL.000"
        );


        pcw_cursor(1);


        return 1;
    }


    seed_random();
    create_player();
    vampire_name_init();


    game_over = 0;
    escaped = 0;

    message_active = 0;

    turn_count = 0;
    current_level = 0;


    pcw_clear();


    pcw_draw_ui();


    show_stats();


    visibility_reset();

#ifdef VT100_80

    pcw_view_update(
        player_x,
        player_y
    );

#endif


    reveal_position(
        player_x,
        player_y
    );


    draw_player();


    while (!game_over) {

        key = getch();


        if (key != '.')
            player_rest_reset();


        if (key >= '1' &&
            key <= '6') {

            slot = key - '1';


            inventory_action(slot);

            continue;
        }


        switch (key) {

        case 'w':

            result =
                step_player(0, -1);


            if (result == STEP_BLOCKED)
                do_turn();


            break;


        case 's':

            result =
                step_player(0, 1);


            if (result == STEP_BLOCKED)
                do_turn();


            break;


        case 'a':

            result =
                step_player(-1, 0);


            if (result == STEP_BLOCKED)
                do_turn();


            break;


        case 'd':

            result =
                step_player(1, 0);


            if (result == STEP_BLOCKED)
                do_turn();


            break;


        case 'W':

            run_player(0, -1);

            break;


        case 'S':

            run_player(0, 1);

            break;


        case 'A':

            run_player(-1, 0);

            break;


        case 'D':

            run_player(1, 0);

            break;


        case '.':

            player_rest_turn();

            break;


        case 'c':
        case 'C':

            close_adjacent_door();

            break;


        case 'o':
        case 'O':

            open_adjacent_door();

            break;


        case 'l':

            lock_adjacent_door();

            break;


        case 'q':
        case 'Q':

            confirm_quit();

            break;
        }
    }


    if (escaped ||
        player_stamina <= 0)
        getch();


    /*
     * Runtime level snapshots belong only to this session.
     */

    level_state_cleanup();


    pcw_cursor(1);


    pcw_goto(0, SCREEN_H - 1);


    return 0;
}