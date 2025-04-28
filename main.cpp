#include "snake_game.h"
#include "q_matrix.h"  // This should define: extern const float Q[256][4]
#include "stm32f429xx.h"
#include <stdio.h> 
//#include "rcc_config1.h"

void SysClockConfig (void)
{
	#define PLL_M 	4
	#define PLL_N 	180
	#define PLL_P 	0  // PLLP = 2

	// 1. ENABLE HSE and wait for the HSE to become Ready
	RCC->CR |= RCC_CR_HSEON;  
	while (!(RCC->CR & RCC_CR_HSERDY));
	
	// 2. Set the POWER ENABLE CLOCK and VOLTAGE REGULATOR
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR |= PWR_CR_VOS; 
	
	
	// 3. Configure the FLASH PREFETCH and the LATENCY Related Settings
	FLASH->ACR = FLASH_ACR_ICEN | FLASH_ACR_DCEN | FLASH_ACR_PRFTEN | FLASH_ACR_LATENCY_5WS;
	
	// 4. Configure the PRESCALARS HCLK, PCLK1, PCLK2
	// AHB PR
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1;
	
	// APB1 PR
	RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;
	
	// APB2 PR
	RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;
	
	
	// 5. Configure the MAIN PLL
	RCC->PLLCFGR = (PLL_M <<0) | (PLL_N << 6) | (PLL_P <<16) | (RCC_PLLCFGR_PLLSRC_HSE);

	// 6. Enable the PLL and wait for it to become ready
	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY));
	
	// 7. Select the Clock Source and wait for it to be set
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
}

// Function to get the best action based on Q-values
static int get_best_action(const float q_row[4]) {
    float max_val = q_row[0];
    int best_action = 0;
    for (int i = 1; i < 4; ++i) {
        if (q_row[i] > max_val) {
            max_val = q_row[i];
            best_action = i;
        }
    }
    return best_action;
}

int main(void) {
    // Initialize USART
		SysClockConfig ();
    USART_Init();
    USART_SendString("Snake Game with Q-learning AI\r\n");
    
    SnakeGame game;
    init_snake_game(&game);
    
    USART_SendString("Game initialized\r\n");
    print_board(&game);
    
    int total_reward = 0;
    int steps = 0;
    while (1) {
        // Calculate state number (0-255)
        int state = 0;
        for (int i = 0; i < 8; ++i) {
            state |= (game.state[i] << i);
        }
        
        // Print Q values for current state
        print_q_values(state);
        
        // Choose action using Q-table
        int action = get_best_action(Q[state]);
        
        // Print chosen action
        char* action_names[] = {"UP", "RIGHT", "DOWN", "LEFT"};
        char buffer[50];
        sprintf(buffer, "Step %d: Chose action: %s\r\n", steps + 1, action_names[action]);
        USART_SendString(buffer);
        
        // Make move
        MoveResult result = make_move(&game, action);
        update_led_matrix(game.board);  // Assumes LED update is implemented
        
        // Print updated board and results
        print_board(&game);
        
        total_reward += result.reward;
        steps++;
        
        sprintf(buffer, "Reward: %d, Total: %d, Snake Length: %d\r\n", 
                result.reward, total_reward, result.new_length);
        USART_SendString(buffer);
        
        if (result.game_over) {
            USART_SendString("\r\nGAME OVER!\r\n");
            sprintf(buffer, "Final score: %d, Steps: %d\r\n", total_reward, steps);
            USART_SendString(buffer);
            break;
        }
        
        // Add delay to make the game visible (adjust as needed)
        for (volatile int i = 0; i < 500000; i++); // Simple delay loop
    }
    
    while (1); // Infinite loop after game ends
}
