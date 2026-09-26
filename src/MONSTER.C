#include "game.h"


struct monster {
    char type;
    int x;
    int y;
    int skill;
    int stamina;
    int dx;
    int dy;
};


static struct monster monsters[MAX_MONSTERS];

static int monster_count;
static int skip_monster;


/*
 * Remove all monsters from memory.
 */

void monster_reset()
{
    monster_count = 0;
    skip_monster = -1;
}


/*
 * Add a monster loaded from LEVEL.MAP.
 *
 * Returns zero if the monster table
 * is full or the type is invalid.
 */

int monster_add(type, x, y, level_number)
char type;
int x;
int y;
int level_number;
{
    int bonus;


    if (monster_count >= MAX_MONSTERS)
        return 0;


    if (type != 'S' &&
        type != 'Z' &&
        type != 'V' &&
        type != '"')
        return 0;


    monsters[monster_count].type = type;

    monsters[monster_count].x = x;
    monsters[monster_count].y = y;
    monsters[monster_count].dx = 0;
    monsters[monster_count].dy = 0;


    bonus = 0;


    if (level_number >= 0 &&
        level_number <= 994)
        bonus = level_number / 2;


    if (type == 'S') {

        monsters[monster_count].skill =
            11 + bonus;
        monsters[monster_count].stamina =
            10 + bonus;

    } else if (type == 'Z') {

        monsters[monster_count].skill =
            10 + bonus;
        monsters[monster_count].stamina =
            16 + bonus;

    } else if (type == 'V') {

        monsters[monster_count].skill = 20;
        monsters[monster_count].stamina = 28;

    } else if (type == '"') {

        monsters[monster_count].skill = 5;
        monsters[monster_count].stamina = 1;

    } else {

        monsters[monster_count].skill = 0;
        monsters[monster_count].stamina = 0;
    }


    ++monster_count;


    return 1;
}


/*
 * Return the monster character at
 * a map square.
 *
 * Zero means no monster.
 */

char monster_char(x, y)
int x;
int y;
{
    int i;


    for (i = 0;
         i < monster_count;
         ++i) {

        if (monsters[i].type == 0)
            continue;


        if (monsters[i].x == x &&
            monsters[i].y == y)
            return monsters[i].type;
    }


    return 0;
}

/*
 * Return non-zero if a square contains a
 * monster which blocks player movement and
 * can be fought.
 *
 */

int monster_blocks_at(x, y)
int x;
int y;
{
    if (monster_char(x, y) == 0)
        return 0;


    return 1;
}

/*
 * Combat values for the monster at a square.
 */

int monster_skill_at(x, y)
int x;
int y;
{
    int i;


    for (i = 0;
         i < monster_count;
         ++i) {

        if ((monsters[i].type == 'S' ||
             monsters[i].type == 'Z' ||
             monsters[i].type == 'V' ||
             monsters[i].type == '"') &&
            monsters[i].x == x &&
            monsters[i].y == y) {

            return monsters[i].skill;
        }
    }


    return 0;
}


int monster_damage_at(x, y, damage)
int x;
int y;
int damage;
{
    int i;


    for (i = 0;
         i < monster_count;
         ++i) {

        if ((monsters[i].type == 'S' ||
             monsters[i].type == 'Z' ||
             monsters[i].type == 'V' ||
             monsters[i].type == '"') &&
            monsters[i].x == x &&
            monsters[i].y == y) {

            monsters[i].stamina -= damage;


            /*
             * A badly wounded vampire changes
             * to bat form at 3 STAMINA or less.
             *
             * If a very large hit overshoots
             * zero, keep the bat alive at one
             * point so the bat phase cannot be
             * skipped completely.
             */

            if (monsters[i].type == 'V' &&
                monsters[i].stamina <= 3) {

                /*
                 * Bat form cannot carry the
                 * Bone Crown.
                 */

                if (monsters[i].skill == 20)
                    level[y][x] = 'K';

                if (monsters[i].stamina < 1)
                    monsters[i].stamina = 1;

                monsters[i].skill = 5;
                monsters[i].type = '"';

                draw_old_position(x, y);

                return 0;
            }


            if (monsters[i].stamina <= 0) {

                if (monsters[i].type == 'S' &&
                    (rand() % 100) < 20) {

                    level[y][x] = 'x';
                }


                /*
                 * Normally V changes into a bat
                 * before this point.  Keep the
                 * death rule explicit as well.
                 */

                if (monsters[i].type == 'V' &&
                    monsters[i].skill == 20)
                    level[y][x] = 'K';


                monsters[i].stamina = 0;
                monsters[i].type = 0;

                return 1;
            }


            return 0;
        }
    }


    return 0;
}


/*
 * The monster fought by the player has already
 * used its interaction for this game turn.
 */

void monster_skip_at(x, y)
int x;
int y;
{
    int i;


    skip_monster = -1;


    for (i = 0;
         i < monster_count;
         ++i) {

        if ((monsters[i].type == 'S' ||
             monsters[i].type == 'Z' ||
             monsters[i].type == 'V' ||
             monsters[i].type == '"') &&
            monsters[i].x == x &&
            monsters[i].y == y) {

            skip_monster = i;

            return;
        }
    }
}


void monster_clear_skip()
{
    skip_monster = -1;
}


/*
 * Return non-zero if terrain blocks a
 * pursuer's line of sight.
 *
 * Walls and closed or locked doors block
 * sight.  Open doors and ordinary floor
 * do not.
 */

static int pursuer_sight_blocked(x, y)
int x;
int y;
{
    char c;


    if (x < 0 || y < 0)
        return 1;


    if (x >= level_w || y >= level_h)
        return 1;


    c = level[y][x];


    if (c == '#')
        return 1;


    if (c == '+')
        return 1;


    if (c == 'L')
        return 1;


    if (c == ' ')
        return 1;


    return 0;
}


/*
 * Bresenham line-of-sight test from a
 * pursuing monster to the player.
 *
 * The monster's own square and the player's
 * square are never treated as blockers.
 */

static int pursuer_can_see_player(mx, my)
int mx;
int my;
{
    int x;
    int y;

    int dx;
    int dy;

    int sx;
    int sy;

    int err;
    int e2;


    if (player_invisible())
        return 0;


    x = mx;
    y = my;


    dx = player_x - x;

    if (dx < 0)
        dx = -dx;


    dy = player_y - y;

    if (dy < 0)
        dy = -dy;


    if (x < player_x)
        sx = 1;
    else
        sx = -1;


    if (y < player_y)
        sy = 1;
    else
        sy = -1;


    err = dx - dy;


    while (x != player_x ||
           y != player_y) {

        e2 = err * 2;


        if (e2 > -dy) {

            err -= dy;
            x += sx;
        }


        if (e2 < dx) {

            err += dx;
            y += sy;
        }


        if (x == player_x &&
            y == player_y)
            return 1;


        if (pursuer_sight_blocked(x, y))
            return 0;
    }


    return 1;
}


/*
 * Return non-zero if a pursuing monster
 * may move into this square.
 *
 * Pursuers use ordinary map passability,
 * cannot enter the player's square, and
 * cannot share a square with another monster.
 */

static int pursuer_can_move(x, y)
int x;
int y;
{
    if (x == player_x &&
        y == player_y)
        return 0;


    if (!map_passable(x, y))
        return 0;


    if (monster_char(x, y) != 0)
        return 0;


    return 1;
}


/*
 * If the uncrowned vampire reaches the
 * Bone Crown, it takes it and returns to
 * full vampire SKILL.
 */

static void vampire_take_crown(i)
int i;
{
    int x;
    int y;


    if (monsters[i].type != 'V')
        return;


    x = monsters[i].x;
    y = monsters[i].y;


    if (level[y][x] != 'K')
        return;


    level[y][x] = '.';
    monsters[i].skill = 20;


    vampire_message(
        "",
        " takes the Bone Crown."
    );
}


/*
 * Try one orthogonal pursuit step toward
 * the player.
 *
 * The axis with the greatest distance is
 * tried first.  If that route is blocked,
 * the other axis is tried instead.
 */

static void move_pursuer(i)
int i;
{
    int old_x;
    int old_y;

    int dx;
    int dy;

    int ax;
    int ay;

    int nx;
    int ny;


    old_x = monsters[i].x;
    old_y = monsters[i].y;


    vampire_take_crown(i);


    if (!pursuer_can_see_player(
            old_x,
            old_y))
        return;


    dx = player_x - old_x;
    dy = player_y - old_y;


    if ((dx == 0 &&
         (dy == -1 || dy == 1)) ||
        (dy == 0 &&
         (dx == -1 || dx == 1))) {

        game_combat_monster(
            old_x,
            old_y
        );

        return;
    }


    if (dx < 0)
        ax = -dx;
    else
        ax = dx;


    if (dy < 0)
        ay = -dy;
    else
        ay = dy;


    nx = old_x;
    ny = old_y;


    if (ax >= ay && dx != 0) {

        if (dx < 0)
            nx = old_x - 1;
        else
            nx = old_x + 1;


        if (!pursuer_can_move(nx, ny)) {

            nx = old_x;
            ny = old_y;
        }
    }


    if (nx == old_x &&
        ny == old_y &&
        dy != 0) {

        if (dy < 0)
            ny = old_y - 1;
        else
            ny = old_y + 1;


        if (!pursuer_can_move(nx, ny)) {

            nx = old_x;
            ny = old_y;
        }
    }


    if (nx == old_x &&
        ny == old_y &&
        ay > ax &&
        dx != 0) {

        if (dx < 0)
            nx = old_x - 1;
        else
            nx = old_x + 1;


        if (!pursuer_can_move(nx, ny)) {

            nx = old_x;
            ny = old_y;
        }
    }


    if (nx == old_x &&
        ny == old_y)
        return;


    monsters[i].x = nx;
    monsters[i].y = ny;


    vampire_take_crown(i);


    draw_old_position(
        old_x,
        old_y
    );


    draw_old_position(
        nx,
        ny
    );
}


/*
 * Skeletons act once every game turn.
 */

void monster_move_skeletons()
{
    int i;
    int crown;


    crown =
        inventory_crown_equipped();


    for (i = 0;
         i < monster_count;
         ++i) {

        if (i == skip_monster)
            continue;


        if (monsters[i].type == 'V') {

            move_pursuer(i);
            continue;
        }


        if (monsters[i].type == 'S' &&
            !crown)
            move_pursuer(i);
    }
}



/*
 * Return Manhattan distance from a square
 * to the player.
 */

static int bat_distance(x, y)
int x;
int y;
{
    int dx;
    int dy;


    dx = x - player_x;

    if (dx < 0)
        dx = -dx;


    dy = y - player_y;

    if (dy < 0)
        dy = -dy;


    return dx + dy;
}


/*
 * Move one square to the legal neighbour
 * farthest from the player.
 *
 * Returns non-zero if the bat moved.
 */

static int bat_flee_step(i)
int i;
{
    static int dxs[4] = {
         0,  0, -1,  1
    };

    static int dys[4] = {
        -1,  1,  0,  0
    };

    int old_x;
    int old_y;

    int best_x;
    int best_y;
    int best_distance;

    int nx;
    int ny;
    int distance;
    int d;


    old_x = monsters[i].x;
    old_y = monsters[i].y;

    best_x = old_x;
    best_y = old_y;
    best_distance = -1;


    for (d = 0; d < 4; ++d) {

        nx = old_x + dxs[d];
        ny = old_y + dys[d];


        if (!pursuer_can_move(nx, ny))
            continue;


        distance =
            bat_distance(nx, ny);


        if (distance > best_distance) {

            best_distance = distance;
            best_x = nx;
            best_y = ny;
        }
    }


    if (best_distance < 0)
        return 0;


    monsters[i].x = best_x;
    monsters[i].y = best_y;


    return 1;
}


/*
 * Bat-form vampires flee up to two squares
 * per game turn and recover one STAMINA.
 *
 * At five STAMINA they return to vampire
 * form with normal vampire SKILL.
 */

static void move_bat(i)
int i;
{
    int old_x;
    int old_y;
    int moved;


    old_x = monsters[i].x;
    old_y = monsters[i].y;

    moved = 0;


    if (bat_flee_step(i))
        moved = 1;


    if (bat_flee_step(i))
        moved = 1;


    if (monsters[i].stamina < 5)
        ++monsters[i].stamina;


    if (monsters[i].stamina >= 5) {

        monsters[i].stamina = 5;
        monsters[i].skill = 7;
        monsters[i].type = 'V';


        vampire_message(
            "",
            " returns to vampire form."
        );


        vampire_take_crown(i);

        moved = 1;
    }


    if (moved) {

        draw_old_position(
            old_x,
            old_y
        );


        if (monsters[i].x != old_x ||
            monsters[i].y != old_y) {

            draw_old_position(
                monsters[i].x,
                monsters[i].y
            );
        }
    }
}


/*
 * Bats act once every game turn.
 */

void monster_move_bats()
{
    int i;


    for (i = 0;
         i < monster_count;
         ++i) {

        if (i != skip_monster &&
            monsters[i].type == '"')
            move_bat(i);
    }
}


/*
 * Zombies act once whenever this is
 * called.  GAME.C calls it every second
 * game turn.
 */

void monster_move_zombies()
{
    int i;


    if (inventory_crown_equipped())
        return;


    for (i = 0;
         i < monster_count;
         ++i) {

        if (i != skip_monster &&
            monsters[i].type == 'Z')
            move_pursuer(i);
    }
}


/*
 * Export/import compact runtime monster state for
 * per-level temporary snapshots.
 *
 * Record layout, 8 bytes:
 *   0 type ('S', 'Z', 'V' or '"')
 *   1 x
 *   2 y
 *   3 skill
 *   4 stamina
 *   5 dx + 1
 *   6 dy + 1
 *   7 reserved
 */

int monster_state_count()
{
    int i;
    int count;


    count = 0;


    for (i = 0;
         i < monster_count;
         ++i) {

        if (monsters[i].type != 0)
            ++count;
    }


    return count;
}


int monster_state_get(index, rec)
int index;
unsigned char *rec;
{
    int i;
    int n;


    n = 0;


    for (i = 0;
         i < monster_count;
         ++i) {

        if (monsters[i].type == 0)
            continue;


        if (n == index) {

            rec[0] = (unsigned char)monsters[i].type;
            rec[1] = (unsigned char)monsters[i].x;
            rec[2] = (unsigned char)monsters[i].y;
            rec[3] = (unsigned char)monsters[i].skill;
            rec[4] = (unsigned char)monsters[i].stamina;
            rec[5] = (unsigned char)(monsters[i].dx + 1);
            rec[6] = (unsigned char)(monsters[i].dy + 1);
            rec[7] = 0;

            return 1;
        }


        ++n;
    }


    return 0;
}


int monster_state_add(rec)
unsigned char *rec;
{
    char type;
    int x;
    int y;
    int dx;
    int dy;


    type = (char)rec[0];
    x = rec[1];
    y = rec[2];
    dx = ((int)rec[5]) - 1;
    dy = ((int)rec[6]) - 1;


    if (type != 'S' &&
        type != 'Z' &&
        type != 'V' &&
        type != '"')
        return 0;


    if (x < 0 || y < 0 ||
        x >= level_w || y >= level_h)
        return 0;


    if (dx < -1 || dx > 1 ||
        dy < -1 || dy > 1)
        return 0;


    if (!monster_add(type, x, y, 0))
        return 0;


    monsters[monster_count - 1].skill = rec[3];
    monsters[monster_count - 1].stamina = rec[4];
    monsters[monster_count - 1].dx = dx;
    monsters[monster_count - 1].dy = dy;


    return 1;
}
