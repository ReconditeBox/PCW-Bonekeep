#include "game.h"


#define V_SEEN  1
#define V_NEW   2


static unsigned char vis[LEVEL_H][LEVEL_W];

static int redraw_pending;


static int in_map();
static int sight_blocked();
static int position_visible_now();
static int local_needs_reveal();

static void mark_visible();
static void cast_ray();
static void flush_area();
static void flush_all_new();
static void find_view_bounds();

static char terrain_char();
static char display_char();


static int in_map(x, y)
int x;
int y;
{
    if (x < 0 || y < 0)
        return 0;

    if (x >= level_w || y >= level_h)
        return 0;

    return 1;
}


/*
 * Terrain which stops sight.
 *
 * The blocking square itself may still be seen;
 * it only prevents seeing beyond it.
 */

static int sight_blocked(x, y)
int x;
int y;
{
    char c;


    if (!in_map(x, y))
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
 * Is this square visible from the player's current
 * position right now?
 *
 * This is used only when deciding whether a moving
 * monster may be drawn on already explored terrain.
 * Remembered terrain remains visible even when the
 * monster itself is out of sight.
 */

static int position_visible_now(tx, ty)
int tx;
int ty;
{
    int x;
    int y;

    int dx;
    int dy;

    int sx;
    int sy;

    int err;
    int e2;

    int old_x;
    int old_y;

    int moved_x;
    int moved_y;


    if (!in_map(tx, ty))
        return 0;


    x = player_x;
    y = player_y;


    if (x == tx && y == ty)
        return 1;


    dx = tx - x;

    if (dx < 0)
        dx = -dx;


    dy = ty - y;

    if (dy < 0)
        dy = -dy;


    if (x < tx)
        sx = 1;
    else
        sx = -1;


    if (y < ty)
        sy = 1;
    else
        sy = -1;


    err = dx - dy;


    while (x != tx || y != ty) {

        old_x = x;
        old_y = y;

        moved_x = 0;
        moved_y = 0;


        e2 = err * 2;


        if (e2 > -dy) {

            err -= dy;
            x += sx;

            moved_x = 1;
        }


        if (e2 < dx) {

            err += dx;
            y += sy;

            moved_y = 1;
        }


        if (!in_map(x, y))
            return 0;


        /*
         * Do not see diagonally through a closed
         * corner where both touching squares block.
         */

        if (moved_x && moved_y) {

            if (sight_blocked(
                    old_x + sx,
                    old_y) &&
                sight_blocked(
                    old_x,
                    old_y + sy)) {

                return 0;
            }
        }


        /*
         * The target blocking square itself may be
         * visible.  Anything blocking before it is not.
         */

        if (x == tx && y == ty)
            return 1;


        if (sight_blocked(x, y))
            return 0;
    }


    return 1;
}


static char terrain_char(x, y)
int x;
int y;
{
    if (level[y][x] == 'L')
        return '+';

    return level[y][x];
}


/*
 * Player is always on top, then monster,
 * then the underlying terrain.
 */

static char display_char(x, y)
int x;
int y;
{
    char c;


    if (x == player_x &&
        y == player_y)
        return '@';


    if (!sight_blocked(x, y) &&
        position_visible_now(x, y)) {

        c = monster_char(x, y);


        if (c != 0)
            return c;
    }


    return terrain_char(x, y);
}


static void mark_visible(x, y)
int x;
int y;
{
    if (!in_map(x, y))
        return;


    if (level[y][x] == ' ')
        return;


    if (!(vis[y][x] & V_SEEN))
        vis[y][x] |= V_SEEN | V_NEW;
}


/*
 * Cheap test used on ordinary movement.
 *
 * If everything within two squares of the player
 * has already been explored, there is normally no
 * new visibility work to do.  This makes movement
 * around an already-seen room very cheap.
 */

static int local_needs_reveal()
{
    int x;
    int y;

    int left;
    int right;
    int top;
    int bottom;


    left = player_x - 2;
    right = player_x + 2;

    top = player_y - 2;
    bottom = player_y + 2;


    if (left < 0)
        left = 0;

    if (top < 0)
        top = 0;

    if (right >= level_w)
        right = level_w - 1;

    if (bottom >= level_h)
        bottom = level_h - 1;


    for (y = top; y <= bottom; ++y) {

        for (x = left; x <= right; ++x) {

            if (level[y][x] == ' ')
                continue;


            if (!(vis[y][x] & V_SEEN))
                return 1;
        }
    }


    return 0;
}


/*
 * Cast one Bresenham ray from the player.
 *
 * Unlike the old visibility code, this is not done
 * once for every map square.  Rays are only cast to
 * the perimeter of the useful viewing rectangle.
 *
 * The first blocking square is revealed, then the
 * ray stops.  A diagonal ray may not squeeze through
 * a corner where both touching orthogonal squares
 * block sight.
 */

static void cast_ray(tx, ty)
int tx;
int ty;
{
    int x;
    int y;

    int dx;
    int dy;

    int sx;
    int sy;

    int err;
    int e2;

    int old_x;
    int old_y;

    int moved_x;
    int moved_y;


    x = player_x;
    y = player_y;


    dx = tx - x;

    if (dx < 0)
        dx = -dx;


    dy = ty - y;

    if (dy < 0)
        dy = -dy;


    if (x < tx)
        sx = 1;
    else
        sx = -1;


    if (y < ty)
        sy = 1;
    else
        sy = -1;


    err = dx - dy;


    while (x != tx || y != ty) {

        old_x = x;
        old_y = y;

        moved_x = 0;
        moved_y = 0;


        e2 = err * 2;


        if (e2 > -dy) {

            err -= dy;
            x += sx;

            moved_x = 1;
        }


        if (e2 < dx) {

            err += dx;
            y += sy;

            moved_y = 1;
        }


        if (!in_map(x, y))
            return;


        if (moved_x && moved_y) {

            if (sight_blocked(
                    old_x + sx,
                    old_y) &&
                sight_blocked(
                    old_x,
                    old_y + sy)) {

                return;
            }
        }


        mark_visible(x, y);


        if (sight_blocked(x, y))
            return;
    }
}


/*
 * Find the useful rectangular area around the player.
 *
 * The first wall/closed door/void square in each
 * cardinal direction forms an edge of the area.
 */

static void find_view_bounds(left, right, top, bottom)
int *left;
int *right;
int *top;
int *bottom;
{
    int x;
    int y;


    x = player_x;


    while (x > 0) {

        --x;


        if (sight_blocked(x, player_y))
            break;
    }


    *left = x;


    x = player_x;


    while (x < level_w - 1) {

        ++x;


        if (sight_blocked(x, player_y))
            break;
    }


    *right = x;


    y = player_y;


    while (y > 0) {

        --y;


        if (sight_blocked(player_x, y))
            break;
    }


    *top = y;


    y = player_y;


    while (y < level_h - 1) {

        ++y;


        if (sight_blocked(player_x, y))
            break;
    }


    *bottom = y;
}


/*
 * Draw newly revealed squares only inside the area
 * which has just been examined.  Consecutive cells
 * on a row are written as one screen run.
 */

static void flush_area(left, right, top, bottom)
int left;
int right;
int top;
int bottom;
{
    int x;
    int y;


    if (left < 0)
        left = 0;

    if (top < 0)
        top = 0;

    if (right >= level_w)
        right = level_w - 1;

    if (bottom >= level_h)
        bottom = level_h - 1;


    for (y = top; y <= bottom; ++y) {

#ifdef VT100_80

        /*
         * A scrolling viewport cannot emit one raw run
         * using logical dungeon coordinates.  Let the
         * terminal driver clip and translate each cell.
         */

        for (x = left; x <= right; ++x) {

            if (vis[y][x] & V_NEW) {

                pcw_map_char(
                    x,
                    y,
                    display_char(x, y)
                );


                vis[y][x] &= ~V_NEW;
            }
        }

#else

        x = left;


        while (x <= right) {

            while (x <= right &&
                   !(vis[y][x] & V_NEW)) {

                ++x;
            }


            if (x > right)
                break;


            pcw_goto(
                MAP_X + x,
                MAP_Y + y
            );


            while (x <= right &&
                   (vis[y][x] & V_NEW)) {

                putch(
                    display_char(x, y)
                );


                vis[y][x] &= ~V_NEW;

                ++x;
            }
        }

#endif
    }
}


/*
 * Used only after restoring a saved level state.
 * Normal movement never scans the whole map merely
 * to find V_NEW squares.
 */

static void flush_all_new()
{
    flush_area(
        0,
        level_w - 1,
        0,
        level_h - 1
    );
}


void visibility_reset()
{
    int x;
    int y;


    for (y = 0; y < LEVEL_H; ++y) {

        for (x = 0; x < LEVEL_W; ++x)
            vis[y][x] = 0;
    }


    redraw_pending = 0;
}


void reveal_position(x, y)
int x;
int y;
{
    int left;
    int right;
    int top;
    int bottom;

    int xx;
    int yy;


    if (!in_map(x, y))
        return;


    if (redraw_pending) {

        flush_all_new();

        redraw_pending = 0;
    }


    mark_visible(x, y);


    /*
     * Most moves inside an already explored room
     * finish here.  draw_player() will paint @.
     */

    if (!local_needs_reveal())
        return;


    find_view_bounds(
        &left,
        &right,
        &top,
        &bottom
    );


    /*
     * Cast to the perimeter, not every square.
     * In an open rectangle these rays cover every
     * cell, while walls and doors stop individual
     * rays before they can reveal around corners.
     */

    for (xx = left; xx <= right; ++xx) {

        cast_ray(xx, top);


        if (bottom != top)
            cast_ray(xx, bottom);
    }


    for (yy = top + 1;
         yy < bottom;
         ++yy) {

        cast_ray(left, yy);


        if (right != left)
            cast_ray(right, yy);
    }


    flush_area(
        left,
        right,
        top,
        bottom
    );
}


/*
 * Restore what is underneath the player.
 *
 * This may now be a monster if another
 * creature has moved onto the square.
 */

#ifdef VT100_80

static void redraw_seen()
{
    int x;
    int y;


    for (y = 0; y < level_h; ++y) {

        for (x = 0; x < level_w; ++x) {

            if (vis[y][x] & V_SEEN) {

                pcw_map_char(
                    x,
                    y,
                    display_char(x, y)
                );
            }
        }
    }
}

#endif


void draw_old_position(x, y)
int x;
int y;
{
    if (!in_map(x, y))
        return;


    if (!(vis[y][x] & V_SEEN))
        return;


    pcw_map_char(
        x,
        y,
        display_char(x, y)
    );
}


void draw_player()
{
#ifdef VT100_80

    if (pcw_view_update(
            player_x,
            player_y)) {

        pcw_clear_map();
        redraw_seen();
    }

#endif


    pcw_map_char(
        player_x,
        player_y,
        '@'
    );
}


/*
 * Export/import persistent visibility state for
 * per-level temporary snapshots.
 *
 * Only the explored bit is persistent.  V_NEW is a
 * transient drawing flag.  Restored explored squares
 * are marked V_NEW so the cleared map is repainted once.
 */

int visibility_state_get(x, y)
int x;
int y;
{
    if (x < 0 || y < 0 ||
        x >= LEVEL_W || y >= LEVEL_H)
        return 0;


    return vis[y][x] & V_SEEN;
}


void visibility_state_set(x, y, value)
int x;
int y;
int value;
{
    unsigned char v;


    if (x < 0 || y < 0 ||
        x >= LEVEL_W || y >= LEVEL_H)
        return;


    v = (unsigned char)value;
    v &= V_SEEN;


    if (v & V_SEEN) {

        v |= V_NEW;
        redraw_pending = 1;
    }


    vis[y][x] = v;
}
