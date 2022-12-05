#include <iostream>
#include <stdio.h>
#include <cstddef>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <array>

void print_board(size_t board[4][4]) {
    const int block_size = 1; 

    for (int i = 0; i < block_size * 4; i++) {
        for (int j = 0; j < 4; j++) {
            for (int k = 0; k < block_size; k++) {
                std::cout << board[i / block_size][j];
            }
            std::cout << '\t';
        }
        if (i % block_size == block_size - 1) {
            std::cout << std::endl;
        }
    }
}

void insert_number(size_t board[4][4]) {
    std::vector<size_t *> place;
    
     
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (board[i][j] == 0) {
                place.push_back(&(board[i][j]));
            }
        }
    }
    
    if (place.size() > 0) {
        int r = rand() % place.size();

        *(place[r]) = rand() % 2 ? 2 : 4;
    }
}

void clear_board(size_t board[4][4]) {
    for (int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            board[i][j] = 0;
        }
    }
}   

enum DIRECTION {LEFT, RIGHT, UP, DOWN, dir_count};


void move(size_t board[4][4], std::pair<int, int> xy, DIRECTION dir) {
    int y = xy.first, x = xy.second;
    
    if (dir == RIGHT) {
        for (int i = x + 1; i < 4; i++) {
            if (board[y][i] == 0) {
                std::swap(board[y][i], board[y][i - 1]);
            }
            else if (board[y][i] == board[y][i - 1]) {
                board[y][i] *= 2;
                board[y][i - 1] = 0;
                break;
            }
            else {
                break;
            }
        }
    }
    if (dir == LEFT) {
        for (int i = x - 1; i > -1; i--) {
            if (board[y][i] == 0) {
                std::swap(board[y][i + 1], board[y][i]);
            }
            else if (board[y][i + 1] == board[y][i]) {
                board[y][i] *= 2;
                board[y][i + 1] = 0;
                break;
            }
            else {
                break;
            }
        }
    }
    if (dir == DOWN) {
        for (int i = y + 1; i < 4; i++) {
            if (board[i][x] == 0) {
                std::swap(board[i - 1][x], board[i][x]);
            }
            else if (board[i - 1][x] == board[i][x]) {
                board[i][x] *= 2;
                board[i - 1][x] = 0;
                break;
            }
            else {
                break;
            }
        }
    }
    if (dir == UP) {
        for (int i = y - 1; i > -1; i--) {
            if (board[i][x] == 0) {
                std::swap(board[i + 1][x], board[i][x]);
            }
            else if (board[i + 1][x] == board[i][x]) {
                board[i][x] *= 2;
                board[i + 1][x] = 0;
                break;
            }
            else {
                break;
            }
        }
    }


}

void shake(size_t board[4][4], DIRECTION dir) {
    for (int i = 0; i < 4; i++) {
        for (int j = 3; j > -1; j--) {
            if (dir == RIGHT) {
                move(board, {i, j}, dir);
            } 
            if (dir == LEFT) {
                move(board, {i, abs(j - 3)}, dir);
            }
            if (dir == DOWN) {
                move(board, {j, i}, dir);
            }
            if (dir == UP) {
                move(board, {abs(j - 3), i}, dir);
            }
        }
    }
}

bool check_loss(size_t board[4][4]) {
    auto f = [&board](int y, int x) {
       if (x > 0 and (board[y][x - 1] == 0 or board[y][x - 1] == board[y][x])) return true; 
       if (x < 3 and (board[y][x + 1] == 0 or board[y][x + 1] == board[y][x])) return true; 
       if (y < 3 and (board[y + 1][x] == 0 or board[y + 1][x] == board[y][x])) return true; 
       if (y > 0 and (board[y - 1][x] == 0 or board[y - 1][x] == board[y][x])) return true; 

       return false;
    };

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << f(i, j) << std::endl;
            if (f(i, j)) return false;
        }
    }

    return true;
}

int main(void) {
    srand(time(NULL));
    size_t board[4][4];

    clear_board(board);
    while(!check_loss(board)) {
        insert_number(board);
        print_board(board);
        char c = EOF;
        while ((c = getchar())) {
            
            if (c == 'a') {
                shake(board, LEFT);
            }
            else if (c == 'd') { 
                
                shake(board, RIGHT);
            }
            else if (c == 'w') {

                shake(board, UP);
            }
            else if (c == 's') {

                shake(board, DOWN);
            }
            else {
                continue;
            }

            break;
        }
        if (c == EOF) {
            break;
        }

        
    }
}
