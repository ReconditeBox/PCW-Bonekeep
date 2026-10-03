#include "game.h"


#define ST_HDRSZ  10
#define ST_RECSZ   8

#define ST_M0     'B'
#define ST_M1     'K'
#define ST_M2     'S'
#define ST_M3     '2'


static int rdall();
static int wrall();
static int valid_tile();
static int state_validate();
static int state_commit();
static void tmpname();


/*
 * Read exactly count bytes.
 */

static int rdall(fd, buf, count)
int fd;
char *buf;
int count;
{
    int n;
    int done;


    done = 0;


    while (done < count) {

        n = read(
            fd,
            buf + done,
            count - done
        );


        if (n <= 0)
            return 0;


        done += n;
    }


    return 1;
}


/*
 * Write exactly count bytes.
 */

static int wrall(fd, buf, count)
int fd;
char *buf;
int count;
{
    int n;
    int done;


    done = 0;


    while (done < count) {

        n = write(
            fd,
            buf + done,
            count - done
        );


        if (n <= 0)
            return 0;


        done += n;
    }


    return 1;
}


/*
 * Runtime map characters which may be saved in a
 * level snapshot.  Player and monsters are stored
 * separately, so @ S Z never appear here.
 */

static int valid_tile(c, levno)
char c;
int levno;
{
    if (c == ' ' ||
        c == '#' ||
        c == '.' ||
        c == '>' ||
        c == '<' ||
        c == '$')
        return 1;


    if (c == 'k' ||
        c == '+' ||
        c == 'L' ||
        c == '/' ||
        c == 'x' ||
        c == '!')
        return 1;


    if (c == '*' ||
        c == 'A' ||
        c == 'p' ||
        c == 'f' ||
        c == 'C')
        return 1;


    if (c == 'K')
        return 1;


    if (c == 'X' &&
        levno == 0)
        return 1;


    return 0;
}


static void tmpname(number, name)
int number;
char *name;
{
    char *base;
    int i;


    base = "BK000.TMP";


    for (i = 0; base[i]; ++i)
        name[i] = base[i];

    name[i] = 0;


    name[2] = '0' + ((number / 100) % 10);
    name[3] = '0' + ((number / 10) % 10);
    name[4] = '0' + (number % 10);
}


/*
 * Save the current runtime state of one level.
 *
 * BKS2 snapshot:
 *   10 byte header
 *   width * height runtime map characters
 *   active monster records (8 bytes each)
 *   width * height visibility bytes
 */

int save_level_state(levno)
int levno;
{
    int fd;
    int x;
    int y;
    int i;
    int count;

    char name[10];
    char hdr[ST_HDRSZ];
    char row[LEVEL_W];
    unsigned char rec[ST_RECSZ];


    if (levno < 0 || levno > 999)
        return 0;


    if (level_w <= 0 || level_w > LEVEL_W ||
        level_h <= 0 || level_h > LEVEL_H)
        return 0;


    count = monster_state_count();

    if (count < 0 || count > MAX_MONSTERS)
        return 0;


    tmpname(levno, name);


    fd = creat(name, 0);

    if (fd < 0)
        return 0;


    hdr[0] = ST_M0;
    hdr[1] = ST_M1;
    hdr[2] = ST_M2;
    hdr[3] = ST_M3;
    hdr[4] = (char)level_w;
    hdr[5] = (char)level_h;
    hdr[6] = (char)(levno & 255);
    hdr[7] = (char)((levno >> 8) & 255);
    hdr[8] = (char)count;
    hdr[9] = 0;


    if (!wrall(fd, hdr, ST_HDRSZ)) {
        close(fd);
        return 0;
    }


    for (y = 0; y < level_h; ++y) {

        for (x = 0; x < level_w; ++x) {

            if (!valid_tile(level[y][x], levno)) {
                close(fd);
                return 0;
            }

            row[x] = level[y][x];
        }


        if (!wrall(fd, row, level_w)) {
            close(fd);
            return 0;
        }
    }


    for (i = 0; i < count; ++i) {

        if (!monster_state_get(i, rec) ||
            !wrall(fd, (char *)rec, ST_RECSZ)) {

            close(fd);
            return 0;
        }
    }


    for (y = 0; y < level_h; ++y) {

        for (x = 0; x < level_w; ++x)
            row[x] = (char)visibility_state_get(x, y);


        if (!wrall(fd, row, level_w)) {
            close(fd);
            return 0;
        }
    }


    close(fd);

    return 1;
}


/*
 * Load a runtime snapshot.
 *
 * Returns:
 *   1  loaded
 *   0  no snapshot exists
 *  -1  invalid snapshot
 */

static int state_validate(
    fd,
    levno,
    widthp,
    heightp,
    countp
)
int fd;
int levno;
int *widthp;
int *heightp;
int *countp;
{
    int width;
    int height;
    int stored;
    int count;
    int x;
    int y;
    int i;

    char hdr[ST_HDRSZ];
    char row[LEVEL_W];
    unsigned char rec[ST_RECSZ];


    if (!rdall(fd, hdr, ST_HDRSZ))
        return 0;


    if (hdr[0] != ST_M0 ||
        hdr[1] != ST_M1)
        return 0;


    if (hdr[2] != ST_M2 ||
        hdr[3] != ST_M3)
        return 0;


    if (hdr[9] != 0)
        return 0;


    width = (unsigned char)hdr[4];
    height = (unsigned char)hdr[5];

    stored = (unsigned char)hdr[6];
    stored |=
        ((int)(unsigned char)hdr[7]) << 8;

    count = (unsigned char)hdr[8];


    if (width <= 0 ||
        width > LEVEL_W)
        return 0;


    if (height <= 0 ||
        height > LEVEL_H)
        return 0;


    if (stored != levno)
        return 0;


    if (count < 0 ||
        count > MAX_MONSTERS)
        return 0;


    for (y = 0; y < height; ++y) {

        if (!rdall(fd, row, width))
            return 0;


        for (x = 0; x < width; ++x) {

            if (!valid_tile(
                    row[x],
                    levno))
                return 0;
        }
    }


    for (i = 0; i < count; ++i) {

        if (!rdall(
                fd,
                (char *)rec,
                ST_RECSZ))
            return 0;


        if (rec[0] != 'S' &&
            rec[0] != 'Z') {

            if (rec[0] != 'V' &&
                rec[0] != '"')
                return 0;
        }


        if (rec[1] >= width ||
            rec[2] >= height)
            return 0;


        if (rec[5] > 2 ||
            rec[6] > 2)
            return 0;
    }


    for (y = 0; y < height; ++y) {

        if (!rdall(fd, row, width))
            return 0;
    }


    *widthp = width;
    *heightp = height;
    *countp = count;


    return 1;
}


static int state_commit(
    fd,
    width,
    height,
    count
)
int fd;
int width;
int height;
int count;
{
    int x;
    int y;
    int i;

    char hdr[ST_HDRSZ];
    char row[LEVEL_W];
    unsigned char rec[ST_RECSZ];


    if (!rdall(fd, hdr, ST_HDRSZ))
        return 0;


    for (y = 0; y < LEVEL_H; ++y) {

        for (x = 0; x < LEVEL_W; ++x)
            level[y][x] = ' ';
    }


    level_w = width;
    level_h = height;


    for (y = 0; y < height; ++y) {

        if (!rdall(fd, row, width))
            return 0;


        for (x = 0; x < width; ++x)
            level[y][x] = row[x];
    }


    monster_reset();


    for (i = 0; i < count; ++i) {

        if (!rdall(
                fd,
                (char *)rec,
                ST_RECSZ))
            return 0;


        if (!monster_state_add(rec))
            return 0;
    }


    visibility_reset();


    for (y = 0; y < height; ++y) {

        if (!rdall(fd, row, width))
            return 0;


        for (x = 0; x < width; ++x) {

            visibility_state_set(
                x,
                y,
                (unsigned char)row[x]
            );
        }
    }


    return 1;
}


/*
 * Load a runtime snapshot.
 *
 * Returns:
 *   1  loaded
 *   0  no snapshot exists
 *  -1  invalid snapshot
 *
 * Pass one validates without changing live
 * state.  Pass two streams directly into the
 * game after the file is known to be good.
 */

int load_level_state(levno)
int levno;
{
    int fd;
    int width;
    int height;
    int count;
    int ok;

    char name[10];


    if (levno < 0 ||
        levno > 999)
        return -1;


    tmpname(levno, name);


    fd = open(name, 0);

    if (fd < 0)
        return 0;


    ok = state_validate(
        fd,
        levno,
        &width,
        &height,
        &count
    );


    close(fd);


    if (!ok)
        return -1;


    fd = open(name, 0);

    if (fd < 0)
        return -1;


    ok = state_commit(
        fd,
        width,
        height,
        count
    );


    close(fd);


    if (!ok)
        return -1;


    return 1;
}


/*
 * Delete BK???.TMP files in the current drive/user area.
 */

void level_state_cleanup()
{
    unsigned char fcb[36];
    int i;


    for (i = 0; i < 36; ++i)
        fcb[i] = 0;


    fcb[0] = 0;

    fcb[1] = 'B';
    fcb[2] = 'K';
    fcb[3] = '?';
    fcb[4] = '?';
    fcb[5] = '?';
    fcb[6] = ' ';
    fcb[7] = ' ';
    fcb[8] = ' ';

    fcb[9] = 'T';
    fcb[10] = 'M';
    fcb[11] = 'P';


    bdos(19, fcb);
}
