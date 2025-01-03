/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ws28xx.h"
/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/
#define LED_ROWS 8
#define LED_COLS 32
#define DELAY_TIME 50  // Faster shifting delay

/* Private variables ---------------------------------------------------------*/
WS28XX_HandleTypeDef ws;

/* Function to calculate the correct LED index based on the zig-zag pattern */
int Get_LED_Index(int row, int col) {
    if (row % 2 == 0) {
        // Even row (left-to-right)
        return (row * LED_COLS / 2) + col;
    } else {
        // Odd row (right-to-left)
        return (row * LED_COLS / 2) + (LED_COLS / 2 - 1 - col);
    }
}

/* Function to draw the arrow */
void Draw_Arrow(WS28XX_HandleTypeDef *ws, int *arrow, int color, int brightness) {
    for (int i = 0; i < 26; i++) {
        // Ensure arrow doesn't affect the first 2 rows or last 2 rows of the LED matrix (top and bottom strips)
        if (arrow[i] >= 16 && arrow[i] <= 239) {
            WS28XX_SetPixel_RGBW_565(ws, arrow[i], color, brightness);  // Only set pixels between the top and bottom strips
        }
    }
    WS28XX_Update(ws);  // Update the matrix to display the arrow
}

/* Function to clear the arrow */
void Clear_Arrow(WS28XX_HandleTypeDef *ws, int *arrow) {
    for (int i = 0; i < 26; i++) {
        if (arrow[i] >= 16 && arrow[i] <= 239) {
            WS28XX_SetPixel_RGBW_565(ws, arrow[i], COLOR_RGB565_BLACK, 0);  // Clear the arrow LEDs between the top and bottom strips
        }
    }
    WS28XX_Update(ws);  // Update the matrix to clear the arrow
}

/* Function to shift the arrow forward */
void Shift_Arrow(int *arrow) {
    for (int i = 0; i < 26; i++) {
        arrow[i] = arrow[i] + 8;  // Shifting each LED forward by one row (8 LEDs in each row)
        if (arrow[i] >= 240) {  // Reset the arrow to the starting position (after the bottom strip)
            arrow[i] = arrow[i] % (LED_ROWS * LED_COLS - 16);  // Ensure the arrow stays between rows 2 to 6
        }
    }
}

/* Function to draw the top and bottom strips */
void Draw_Strips(WS28XX_HandleTypeDef *ws, int color, int brightness) {
    // Top strip (LEDs from 240 to 255)
    for (int i = 240; i <= 255; i++) {
        WS28XX_SetPixel_RGBW_565(ws, i, color, brightness);
    }

    // Bottom strip (LEDs from 0 to 15)
    for (int i = 0; i <= 15; i++) {
        WS28XX_SetPixel_RGBW_565(ws, i, color, brightness);
    }

    WS28XX_Update(ws);  // Update the matrix to display the strips
}

/* Function to update brightness for both arrow and strips */
void Update_Brightness(WS28XX_HandleTypeDef *ws, int arrow_brightness, int strip_brightness) {
    // Arrow shape LED indices
    int arrow[] = {
        83, 84, 91, 92, 99, 100, 107, 108, 115, 116, 123, 124,
        131, 132, 137, 138, 139, 140, 141, 142, 146, 147, 148, 149,
        155, 156
    };

    // Update brightness for the arrow
    Draw_Arrow(ws, arrow, COLOR_RGB565_GREEN, arrow_brightness);

    // Update brightness for the strips
    Draw_Strips(ws, COLOR_RGB565_CYAN, strip_brightness);
}

/* The application entry point. */
int main(void) {
  /* MCU Configuration--------------------------------------------------------*/

  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();

  WS28XX_Init(&ws, &htim3, 72, TIM_CHANNEL_1, 256);  // Initialize with total LEDs set to 256

  /* Arrow shape LED indices */
  int arrow[] = {
      83, 84, 91, 92, 99, 100, 107, 108, 115, 116, 123, 124,
      131, 132, 137, 138, 139, 140, 141, 142, 146, 147, 148, 149,
      155, 156
  };

  /* Draw the strips once, keeping them constant */
  Draw_Strips(&ws, COLOR_RGB565_CYAN, 255);  // Max brightness for strips

  /* Infinite loop */
  while (1) {
    // Clear the arrow from its previous position
    Clear_Arrow(&ws, arrow);

    // Shift the arrow to the next position
    Shift_Arrow(arrow);

    // Draw the arrow with adjustable brightness, keeping strips constant
    Draw_Arrow(&ws, arrow, COLOR_RGB565_GREEN, 255);  // Max brightness for the arrow

    HAL_Delay(DELAY_TIME);  // Adjust the delay as per your speed requirement
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Error Handler
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
