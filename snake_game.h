#ifndef SNAKE_GAME_H
#define SNAKE_GAME_H
typedef unsigned char uint8_t;


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

#define WIDTH 8
#define HEIGHT 8
#define MAX_SNAKE_LENGTH 64
#define HEAD_VAL 2
#define BODY_VAL 1
#define FOOD_VAL 3

typedef struct {
    uint8_t x;
    uint8_t y;
} Coord;

typedef struct {
    Coord segments[MAX_SNAKE_LENGTH];
    uint8_t length;
} Snake;

typedef struct {
    int board[HEIGHT][WIDTH];
    Snake snake;
    Coord food;
    uint8_t state[8];
} SnakeGame;

typedef struct {
    bool game_over;
    int reward;
    int new_length;
} MoveResult;

void init_snake_game(SnakeGame* game);
MoveResult make_move(SnakeGame* game, int direction);
void update_led_matrix(int board[HEIGHT][WIDTH]);
void game_tick(SnakeGame* game, int direction);
void USART_Init(void) ;
void send_char(uint8_t c);
void USART_SendString(const char* str);
void print_board(SnakeGame* game);
void print_q_values(int state);
void send_str(char * str);

#ifdef __cplusplus
}
#endif

#endif // SNAKE_GAME_H
