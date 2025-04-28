// snake_game.cpp
#include <stdint.h>
#include "stm32f429xx.h"
#include "q_matrix.h"
#include "snake_game.h"
#include <stdlib.h>
#include <stdio.h> 

void update_state(SnakeGame* game);

void init_snake_game(SnakeGame* game) {
    // Clear board
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            game->board[y][x] = 0;

    // Initialize snake
    game->snake.length = 1;
    game->snake.segments[0].x = WIDTH / 2;
    game->snake.segments[0].y = HEIGHT / 2;
    game->board[HEIGHT / 2][WIDTH / 2] = HEAD_VAL;

    // Spawn food
    int fx, fy;
    do {
        fx = rand() % WIDTH;
        fy = rand() % HEIGHT;
    } while (game->board[fy][fx] != 0);
    game->food.x = fx;
    game->food.y = fy;
    game->board[fy][fx] = FOOD_VAL;

    update_state(game);
}


Coord get_potential_position(Coord head, int direction) {
    Coord next = head;
    if (direction == 0) next.y--;
    else if (direction == 1) next.x++;
    else if (direction == 2) next.y++;
    else if (direction == 3) next.x--;
    return next;
}


bool is_valid_move(SnakeGame* game, Coord next) {
    if (next.x < 0 || next.x >= WIDTH || next.y < 0 || next.y >= HEIGHT)
        return false;
    int val = game->board[next.y][next.x];
    return val != BODY_VAL;
}


void update_state(SnakeGame* game) {
    Coord head = game->snake.segments[0];
    for (int i = 0; i < 4; ++i) {
        Coord next = get_potential_position(head, i);
        game->state[i] = !is_valid_move(game, next);
    }
    for (int i = 0; i < 4; ++i) game->state[4 + i] = 0;
    if (game->food.y < head.y) game->state[0 + 4] = 1;
    if (game->food.x > head.x) game->state[1 + 4] = 1;
    if (game->food.y > head.y) game->state[2 + 4] = 1;
    if (game->food.x < head.x) game->state[3 + 4] = 1;
}


MoveResult make_move(SnakeGame* game, int direction) {
    MoveResult result = {false, -2, game->snake.length};
    Coord head = game->snake.segments[0];
    Coord next = get_potential_position(head, direction);

    if (!is_valid_move(game, next)) {
        result.game_over = true;
        return result;
    }

    int is_food = (next.x == game->food.x && next.y == game->food.y);
		
		Coord old_tail=game->snake.segments[game->snake.length-1];
    // Shift body
    for (int i = game->snake.length - 1; i > 0; --i) {
        game->snake.segments[i] = game->snake.segments[i - 1];
    }
    game->snake.segments[0] = next;

    if (is_food) {
        game->snake.length++;
				game->snake.segments[game->snake.length-1]=old_tail;
        result.reward = 2;
        result.new_length = game->snake.length;
    } else {
       // Coord tail = game->snake.segments[game->snake.length-1];
        game->board[old_tail.y][old_tail.x] = 0;
        result.reward = (game->state[direction + 4] == 1) ? 1 : 0;
    }

    // Update board
    for (int y = 0; y < HEIGHT; ++y)
        for (int x = 0; x < WIDTH; ++x)
            if (game->board[y][x] == HEAD_VAL || game->board[y][x] == BODY_VAL)
                game->board[y][x] = 0;

    for (int i = 0; i < game->snake.length; ++i) {
        Coord s = game->snake.segments[i];
        game->board[s.y][s.x] = (i == 0) ? HEAD_VAL : BODY_VAL;
    }

    if (is_food) {
        int fx, fy;
        do {
            fx = rand() % WIDTH;
            fy = rand() % HEIGHT;
        } while (game->board[fy][fx] != 0);
        game->food.x = fx;
        game->food.y = fy;
        game->board[fy][fx] = FOOD_VAL;
    }

    update_state(game);
    return result;
}


void update_led_matrix(int board[HEIGHT][WIDTH]) {
    // Implement this to update 8x8 LED matrix based on board contents
}


void game_tick(SnakeGame* game, int direction) {
    MoveResult result = make_move(game, direction);
    update_led_matrix(game->board);
    // Optional: hook result to feedback logic
}



void USART_Init(void) {
    // Enable clocks for GPIOB and USART3
  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->AHB1ENR |= (1<<0); 
	GPIOA->MODER |= (1<<21);  
	GPIOA->MODER |= (1<<19);  
	GPIOA->AFR[1] |= (7<<4);  
	GPIOA->AFR[1] |= (7<<8);
	USART1->CR1 = 0x00;  
	USART1->CR1 |= (1<<13);  	
	USART1->CR1 &= ~(1<<12); 
	USART1->CR2 =0x00;	
	USART1->BRR	 =0x249F; 	
	USART1->CR1 |= (1<<2); 
	USART1->CR1 |= (1<<3); 
}


void send_char(uint8_t c) {
    while(!(USART1->SR & (1<<7)));       // Wait until TXE=1
    USART1->DR = c;
}

// Add this function to send a string via USART
void USART_SendString(const char* str) {
    // Send each character in the string
    int i = 0;
    while(str[i] != '\0') {
        send_char(str[i]);
        i++;
    }
}



// Function to print the game board via UART
void print_board(SnakeGame* game) {
    char buffer[100];
    
    // Print header
    send_str("\r\n+--------+\r\n");
    
    // Print board
    for (int y = 0; y < HEIGHT; y++) {
        send_str("|");
        for (int x = 0; x < WIDTH; x++) {
            char cell[2] = {' ', '\0'};
            switch (game->board[y][x]) {
                case HEAD_VAL: cell[0] = 'H'; break;
                case BODY_VAL: cell[0] = 'O'; break;
                case FOOD_VAL: cell[0] = '*'; break;
                default: cell[0] = ' '; break;
            }
            send_str(cell);
        }
        send_str("|\r\n");
    }
    
    // Print footer
    send_str("+--------+\r\n");
    
    // Calculate state value
    int state = 0;
    for (int i = 0; i < 8; ++i) {
        state |= (game->state[i] << i);
    }
    
    // Print state information
    sprintf(buffer, "State: %d\r\n", state);
    send_str(buffer);
    
    sprintf(buffer, "Direction bits: %d%d%d%d\r\n", 
            game->state[0], game->state[1], game->state[2], game->state[3]);
    send_str(buffer);
            
    sprintf(buffer, "Food bits: %d%d%d%d\r\n",
            game->state[4], game->state[5], game->state[6], game->state[7]);
    send_str(buffer);
}


void send_str(char * str) {
    int i = 0;
    while(str[i] != '\0') {
        send_char(str[i]);
        i++;
    }
}

// Function to print the Q-values for current state
void print_q_values(int state) {
    char buffer[100];
    send_str("Q-values: ");
    
    sprintf(buffer, "Up: %.2f, ", Q[state][0]);
    send_str(buffer);
    
    sprintf(buffer, "Right: %.2f, ", Q[state][1]);
    send_str(buffer);
    
    sprintf(buffer, "Down: %.2f, ", Q[state][2]);
    send_str(buffer);
    
    sprintf(buffer, "Left: %.2f\r\n", Q[state][3]);
    send_str(buffer);
}
