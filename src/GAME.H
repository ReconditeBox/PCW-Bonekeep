#ifndef GAME_H
#define GAME_H


/*
 * BONEKEEP screen layout.
 *
 * Game display uses 90 x 30.
 * PCW rows 30 and 31 are unused.
 */

#define SCREEN_W        90
#define SCREEN_H        30

#define MAP_W           75
#define MAP_H           24

#define MAP_X           1
#define MAP_Y           1

#define DIVIDER_X       76

#define PANEL_X         77
#define PANEL_W         12
#define PANEL_VALUE_X   86

#define RIGHT_X         89

#define MESSAGE_X       1
#define MESSAGE_Y       26
#define MESSAGE_W       88
#define MESSAGE_LINES   3


#define MAX_MONSTERS    64
#define INVENTORY_SIZE  6
#define MONSTER_STATE_SIZE 8


#define STEP_BLOCKED    0
#define STEP_MOVED      1
#define STEP_EXIT       2
#define STEP_COMBAT     3
#define STEP_LEVEL      4


extern char getch();
extern void putch();

extern int open();
extern int creat();
extern int read();
extern int write();
extern int close();
extern int unlink();

extern int rand();
extern int srand();
extern int bdos();


extern char level[MAP_H][MAP_W];

extern int level_w;
extern int level_h;


extern int player_x;
extern int player_y;

extern int player_skill;
extern int player_stamina;
extern int player_max_stamina;
extern int player_gold;
extern int player_keys;
extern int player_kills;

extern char player_name[31];
extern char vampire_name[31];


/*
 * VNAME.C
 */

void vampire_name_init();
void vampire_message();

void show_message();


/*
 * SCREEN.C
 */

void pcw_goto();
void pcw_clear();
void pcw_cursor();
void pcw_erase_eol();
void pcw_text();
void pcw_number();
void pcw_number2();
void pcw_victory();

void pcw_draw_ui();
void pcw_clear_map();
void pcw_clear_messages();


/*
 * LEVEL.C
 */

int load_level();
int level_entry_ok();

/*
 * STATE.C
 */

int save_level_state();
int load_level_state();
void level_state_cleanup();

int map_passable();
int map_exit();

int map_closed_door();
int map_locked_door();

void map_open_door();
void map_unlock_door();

int item_char();


/*
 * MONSTER.C
 */

void monster_reset();
int monster_add();
char monster_char();
int monster_blocks_at();
int monster_skill_at();
int monster_damage_at();
void monster_skip_at();
void monster_clear_skip();
void monster_move_skeletons();
void monster_move_bats();
void monster_move_zombies();
int monster_state_count();
int monster_state_get();
int monster_state_add();


/*
 * COMBAT
 */

int game_combat_monster();


/*
 * GAME.C services used by INVENTORY.C
 */

void do_turn();
void show_message();
void clear_message();


/*
 * INVENTORY.C
 */

void inventory_init();
int offer_item_pickup();
void inventory_action();
int inventory_sword_power();
int inventory_shield_power();
int inventory_armour_power();
int inventory_crown_equipped();
int inventory_has_crown();


/*
 * PLAYER.C
 */

void player_health_init();
void player_rest_reset();
void player_rest_turn();
int player_heal();
void player_drink_potion();
void player_potion_turn();
int player_invisible();


/*
 * VIS.C
 */

void visibility_reset();
int visibility_state_get();
void visibility_state_set();
void reveal_position();
void draw_player();
void draw_old_position();


#endif