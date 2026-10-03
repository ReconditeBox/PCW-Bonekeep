#include "game.h"


#define POTION_NONE      0
#define POTION_STAMINA   1
#define POTION_SKILL     2
#define POTION_INVIS     3

#define POTION_TURNS     15


int player_max_stamina;


static int rest_count;

static unsigned char potion_effect;
static unsigned char potion_turns;


static void show_player_skill()
{
    pcw_goto(
        PANEL_VALUE_X,
        STAT_SKILL_Y
    );


    pcw_number2(
        player_skill
    );
}


static void show_player_stamina()
{
    pcw_goto(
        PANEL_VALUE_X,
        STAT_STAMINA_Y
    );


    pcw_number2(
        player_stamina
    );
}


static void potion_remove()
{
    if (potion_effect ==
        POTION_STAMINA) {

        player_max_stamina -= 4;


        if (player_stamina >
            player_max_stamina)
            player_stamina =
                player_max_stamina;


        show_player_stamina();
    }


    if (potion_effect ==
        POTION_SKILL) {

        player_skill -= 2;


        if (player_skill < 1)
            player_skill = 1;


        show_player_skill();
    }


    potion_effect = POTION_NONE;
    potion_turns = 0;
}


void player_health_init()
{
    player_max_stamina =
        player_stamina;

    rest_count = 0;

    potion_effect = POTION_NONE;
    potion_turns = 0;
}


void player_rest_reset()
{
    rest_count = 0;
}


int player_heal(amount)
int amount;
{
    int old;


    if (amount <= 0)
        return 0;


    if (player_stamina >=
        player_max_stamina)
        return 0;


    old = player_stamina;


    player_stamina += amount;


    if (player_stamina >
        player_max_stamina)
        player_stamina =
            player_max_stamina;


    show_player_stamina();


    return player_stamina - old;
}


void player_drink_potion()
{
    int roll;


    /*
     * A new potion always replaces any
     * existing potion effect.
     */

    potion_remove();


    roll = rand() % 4;


    if (roll == 0) {

        potion_effect =
            POTION_STAMINA;

        potion_turns =
            POTION_TURNS;


        player_max_stamina += 4;
        player_stamina += 4;


        show_player_stamina();


        show_message(
            "Your STAMINA surges."
        );

        return;
    }


    if (roll == 1) {

        potion_effect =
            POTION_SKILL;

        potion_turns =
            POTION_TURNS;


        player_skill += 2;


        show_player_skill();


        show_message(
            "Your SKILL sharpens."
        );

        return;
    }


    if (roll == 2) {

        potion_effect =
            POTION_INVIS;

        potion_turns =
            POTION_TURNS;


        show_message(
            "You fade from sight."
        );

        return;
    }


    show_message(
        "Nothing happens."
    );
}


void player_potion_turn()
{
    if (potion_effect ==
        POTION_NONE)
        return;


    if (potion_turns > 0)
        --potion_turns;


    if (potion_turns != 0)
        return;


    potion_remove();


    show_message(
        "The potion wears off."
    );
}


int player_invisible()
{
    if (potion_effect ==
        POTION_INVIS)
        return 1;


    return 0;
}


void player_rest_turn()
{
    ++rest_count;


    do_turn();


    if (player_stamina <= 0)
        return;


    if (rest_count < 4)
        return;


    rest_count = 0;


    if (player_heal(1) > 0)
        show_message(
            "You recover 1 STAMINA."
        );
}
