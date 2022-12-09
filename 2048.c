#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ncurses.h>
#include <time.h>

#define OPTIONS "h:w:m:"


typedef struct GameStateObj {
    size_t width;
    size_t height; 
    size_t max;
    size_t **board;
    size_t **canidates;
    size_t **line_helper;
    size_t *spawn_pool;
    size_t spawn_pool_len;
} GameStateObj;

typedef struct GameStateObj *GameState;

int digits(size_t i) {
    int j = 0;
    while(i > 0) {
        i/=10;
        j++;
    }

    return j;
}

GameState board_create(size_t width, size_t height, size_t max, size_t *spawn_pool, size_t spawn_pool_len) {
    GameState gs = malloc(sizeof(GameStateObj));
    gs->board = calloc(sizeof(size_t *), height);
    gs->canidates = calloc(sizeof(size_t *), width * height);
    gs->line_helper = calloc(sizeof(size_t *), height > width ? height : width);
    gs->spawn_pool = spawn_pool;
    gs->spawn_pool_len = spawn_pool_len;

    for (size_t y = 0; y < height; y++) {
        gs->board[y] = calloc(sizeof(size_t), width);
        for (size_t x = 0; x < width; x++) {
            gs->board[y][x] = 0;
        }
    } 

    gs->width = width;
    gs->height = height;

    gs->max = max;
    
    return gs;
}

void board_delete(GameState *pgs) {
    if (pgs && *pgs) {
        for (size_t i = 0; i < (*pgs)->height; i++) {
            free((*pgs)->board[i]);
        }
        free((*pgs)->board);
        free((*pgs)->canidates);
        free((*pgs)->line_helper);
        free(*pgs);
    }
}


#define CROSS           (110 | A_ALTCHARSET)
#define LEFT_TREE       (117 | A_ALTCHARSET)
#define RIGHT_TREE      (116 | A_ALTCHARSET)
#define UP_TREE         (118 | A_ALTCHARSET)
#define DOWN_TREE       (119 | A_ALTCHARSET)
#define BOTTOM_LEFT     (109 | A_ALTCHARSET) 
#define TOP_LEFT        (108 | A_ALTCHARSET) 
#define TOP_RIGHT       (107 | A_ALTCHARSET) 
#define BOTTOM_RIGHT    (106 | A_ALTCHARSET) 
#define HORIZANTAL      (113 | A_ALTCHARSET)
#define VERTICAL        (120 | A_ALTCHARSET)

typedef enum {
    DEFAULT,
    _COLOR_2, 
    _COLOR_4, 
    _COLOR_8,
    _COLOR_16,
    _COLOR_32,
    _COLOR_64,
    _COLOR_128,
    _COLOR_256,
    _COLOR_512,
    _COLOR_1024,
    _COLOR_2048,
    _COLOR_GREATER
} DEFINED_COLOR_PAIR;

DEFINED_COLOR_PAIR set_color_by_num(size_t i) {
    if (i == 2) return _COLOR_2;
    if (i == 4) return _COLOR_4;
    if (i == 8) return _COLOR_8;
    if (i == 16) return _COLOR_16;
    if (i == 32) return _COLOR_32;
    if (i == 64) return _COLOR_64;
    if (i == 128) return _COLOR_128;
    if (i == 256) return _COLOR_256;
    if (i == 512) return _COLOR_512;
    if (i == 1024) return _COLOR_1024;
    if (i == 2048)return _COLOR_2048;
    if (i > 2048)return _COLOR_GREATER;

    return DEFAULT;
}

void print_board(GameState gs) {
    size_t board_width = gs->width * digits(gs->max) + gs->width + 1;
    size_t board_height = 2 * gs->height + 1;
    
    size_t max_height, max_width;
    getmaxyx(stdscr, max_height, max_width);

    if (board_width > max_width || board_height > max_height) {
        printw("ERROR! TERMINAL WINDOW IS NOT BIG ENOUGH\n");
        return;
    }

    int curr_width = (max_width) / 2 - board_width / 2, curr_height = (max_height) / 2 - board_height / 2;
    
    move(curr_height, curr_width); 
    char format_string[100];

    for (size_t i = 0; i < board_height; i++) {
        for (size_t j = 0; j < board_width; j++) {
            if (i == 0) {
                if (j == 0) addch(TOP_LEFT);
                else if (j == board_width - 1) addch(TOP_RIGHT);
                else if (j % (digits(gs->max) + 1) == 0) addch(DOWN_TREE);
                else addch(HORIZANTAL);
            }
            else if (i == board_height - 1) {
                if (j == 0) addch(BOTTOM_LEFT);
                else if (j == board_width - 1) addch(BOTTOM_RIGHT);
                else if (j % (digits(gs->max) + 1) == 0) addch(UP_TREE);
                else addch(HORIZANTAL);

            }
            else {
                // Non number row
                if (i & 1) {
                    if (j == 0) addch(VERTICAL);
                    else if (j == board_width - 1) addch(VERTICAL);
                    else if (j % (digits(gs->max) + 1) == 0) addch(VERTICAL);
                    else if (j % (digits(gs->max) + 1) == 1) {
                        size_t num = gs->board[i / 2][j /(digits(gs->max) + 1) ];
                        if (num != 0) {
                            attron(COLOR_PAIR(set_color_by_num(num)));
                            printw("%-*zu", digits(gs->max), num);
                            attroff(COLOR_PAIR(set_color_by_num(num)));
                        }
                        else {
                            printw("%*c", digits(gs->max), ' ');
                        }
                    }
                }
                // Number row
                else {
                    if (j == 0) addch(RIGHT_TREE);
                    else if (j == board_width - 1) addch(LEFT_TREE);
                    else if (j % (digits(gs->max) + 1) == 0) addch(CROSS);
                    else addch(HORIZANTAL);
                }
            }
        }
        move(curr_height + i + 1, curr_width);
    }
}

typedef enum {EXIT = -1, INVALID, LEFT, RIGHT, UP, DOWN} DIRECTION;

DIRECTION char_to_dir(char c) {
    if (c == 'a') return LEFT;
    if (c == 'd') return RIGHT;
    if (c == 'w') return UP;
    if (c == 's') return DOWN;
    if (c == 27) return EXIT;
    
    return INVALID;
}

bool board_insert_number(GameState gs) {
    size_t num_canidates = 0;
    for (size_t y = 0; y < gs->height; y++) {
        for (size_t x = 0; x < gs->width; x++) {
            if (gs->board[y][x] == 0) {
                gs->canidates[num_canidates++] = gs->board[y] + x;
            }
        }
    }

    if (num_canidates == 0) {
        return false;
    }

    size_t canidate = rand() % num_canidates;
    
    *(gs->canidates[canidate]) = gs->spawn_pool[rand() % gs->spawn_pool_len];

    return true;
}
bool board_line_transform(GameState gs, DIRECTION dir, size_t line_number) {
    if ((dir == LEFT || dir == RIGHT) && line_number >= gs->height) return false; 
    else if ((dir == UP || dir == DOWN) && line_number >= gs->width) return false;
    
    switch (dir) {
        case LEFT:
            for (size_t i = 0; i < gs->width; i++) {
                gs->line_helper[i] = gs->board[line_number] + i;
            }
            break;
        case RIGHT:
            for (size_t i = 0; i < gs->width; i++) {
                gs->line_helper[i] = gs->board[line_number] + (gs->width - i - 1);
            }
            break;
        case UP:
            for (size_t i = 0; i < gs->height; i++) {
                gs->line_helper[i] = gs->board[i] + line_number;
            }
            break;
        case DOWN:
            for (size_t i = 0; i < gs->height; i++) {
                gs->line_helper[i] = gs->board[gs->height - i - 1] + line_number;
            }
            break;
        default: return false;
    }

    bool moved = false;
    
    size_t **turtle = gs->line_helper;
    size_t **hare = gs->line_helper + 1;

    for (size_t i = 1; i < ((dir == UP || dir == DOWN) ? gs->height : gs->width); i++) {
        size_t *t = *turtle;
        size_t *h = *hare;
        // If there is an empty slot for h to move to do so
        if (*t == 0 && *h > 0) {
            *t = *h;
            *h = 0;
            moved = true;
        }
        // If two blocks can be combined do so
        else if (*t != 0 && *t == *h) {
            *t *= 2;
            *h = 0;
            turtle++;
            moved = true;
        }
        // If a hare will not combine with the turtle put it after the turtle
        else if (*h != 0 && *t != *h) {
            turtle++;
            if (hare - turtle > 0) {
                **turtle = *h;
                *h= 0;
                moved = true;
            }
        }
    
        hare++;
    }

    return moved;
}

bool board_shake(GameState gs, DIRECTION dir) {
    move(0, 0);
    bool moved = false;
    for (size_t i = 0; i < ((dir == UP || dir == DOWN) ? gs->width : gs->height); i++) {
        moved |= board_line_transform(gs, dir, i);
    }

    return moved;
}


bool check_adjacent(GameState gs, size_t y, size_t x) {
    if (y < gs->height && (gs->board[y + 1][x] == 0 || gs->board[y + 1][x] == gs->board[y][x])) return true;
    if (y > 0 && (gs->board[y - 1][x] == 0 || gs->board[y - 1][x] == gs->board[y][x])) return true;
    if (x < gs->width && (gs->board[y][x + 1] == 0 || gs->board[y][x + 1] == gs->board[y][x])) return true;
    if (x > 0 && (gs->board[y][x - 1] == 0 || gs->board[y][x - 1] == gs->board[y][x])) return true;

    return false;
}

// -1 is lose, 0 is play, 1 is win
int board_check_state(GameState gs) {
    bool play = false;
    for (size_t y = 0; y < gs->height; y++) {
        for (size_t x = 0; x < gs->width; x++) {
            if (gs->board[y][x] >= gs->max) return 1;
            if (!play) play |= check_adjacent(gs, y, x);
        }
    }
    if (!play) return -1;
    
    return 0;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    int opt; 
    
    size_t height = 4, width = 4, max = 2048;

    while((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch(opt) {
            case 'h':
                height = strtoul(optarg, NULL, 10);
                if (height > 32) height = 32;
                if (height < 2) height = 4;
                break;
            case 'w':
                width = strtoul(optarg, NULL, 10);
                if (width > 32) width = 32;
                if (width < 2) width = 4;
                break;
        }
    }

    initscr();
    raw(); 
    noecho();
    curs_set(0);
	start_color();

    init_pair(_COLOR_2, COLOR_WHITE, COLOR_BLACK);
    init_pair(_COLOR_4, COLOR_YELLOW, COLOR_BLACK);
    init_pair(_COLOR_8, COLOR_RED, COLOR_BLACK);
    init_pair(_COLOR_16, COLOR_GREEN, COLOR_BLACK);
    init_pair(_COLOR_32, COLOR_BLUE, COLOR_BLACK);
    init_pair(_COLOR_64, COLOR_CYAN, COLOR_BLACK);
    init_pair(_COLOR_128, COLOR_BLACK, COLOR_RED);
    init_pair(_COLOR_256, COLOR_BLACK, COLOR_GREEN);
    init_pair(_COLOR_512, COLOR_BLACK, COLOR_BLUE);
    init_pair(_COLOR_1024, COLOR_BLACK, COLOR_MAGENTA);
    init_pair(_COLOR_2048, COLOR_BLACK, COLOR_YELLOW); 
    init_pair(_COLOR_GREATER, COLOR_BLACK, COLOR_WHITE);
    
    // size_t spawn_pool[10] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};
    size_t spawn_pool[2] = {2, 4};

    GameState gs = board_create(width, height, max, spawn_pool, 2);
    print_board(gs); 
    refresh();
    DIRECTION c = INVALID;
    
    board_insert_number(gs);
    print_board(gs);

    while (true){
        c = char_to_dir(getch());
        
        if (c == EXIT) break;
        if (c == INVALID) continue; 
        if (!board_shake(gs, c)) continue;
        clear();
        board_insert_number(gs);
        print_board(gs);


    };

    endwin();

    board_delete(&gs);

    return 0;
}
