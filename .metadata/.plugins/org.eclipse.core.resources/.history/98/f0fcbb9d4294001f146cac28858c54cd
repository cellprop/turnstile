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
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LED_ROWS 8
#define LED_COLS 32
#define DELAY_TIME 50
#define BLINK_DELAY 500
volatile int counter = 0;
volatile int rev = 0;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Arrow LED indices */
int arrow[] = {
    83, 84, 91, 92, 99, 100, 107, 108, 115, 116, 123, 124,
    131, 132, 137, 138, 139, 140, 141, 142, 146, 147, 148, 149,
    155, 156
};

/* "X" Cross LED Addresses */
int topCross[] = {224, 231, 222, 217, 213, 210, 203, 204, 196, 195, 189, 186, 182, 177, 175, 168};
int middleCross[] = {103, 96, 105, 110, 114, 117, 123, 124, 132, 131, 138, 141, 145, 150, 152, 159};
int bottomCross[] = {31, 24, 33, 38, 42, 45, 51, 52, 59, 60, 66, 69, 73, 78, 80, 87};

uint8_t rxData[14]; // Array to store received bytes
uint8_t processedData[13]; // Array to store data without start and end byte
uint8_t flag = 0; // Flag to indicate data reception is complete
uint8_t responseData;      // To store the boolean response (0 for false, 1 for true)
uint8_t flagSuccess = 0;   // Flag to indicate if 'true' was received
uint8_t flagFailure = 0;   // Flag to indicate if 'false' was received
char usermsg[12];
int v = 0;
int flag_rev=0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Function to calculate the correct LED index based on the zig-zag pattern */
int Get_LED_Index(int row, int col) {
    if (row % 2 == 0) {
        return (row * LED_COLS / 2) + col;
    } else {
        return (row * LED_COLS / 2) + (LED_COLS / 2 - 1 - col);
    }
}

/* Function to draw the arrow */
void Draw_Arrow(WS28XX_HandleTypeDef *ws, int color, int brightness) {
    for (int i = 0; i < 26; i++) {
        if (arrow[i] >= 16 && arrow[i] <= 239) {
            WS28XX_SetPixel_RGBW_565(ws, arrow[i], color, brightness);
        }
    }
    WS28XX_Update(ws);
}

/* Function to clear the arrow */
void Clear_Arrow(WS28XX_HandleTypeDef *ws) {
    for (int i = 0; i < 26; i++) {
        if (arrow[i] >= 16 && arrow[i] <= 239) {
            WS28XX_SetPixel_RGBW_565(ws, arrow[i], COLOR_RGB565_BLACK, 0);
        }
    }
    WS28XX_Update(ws);
}

/* Function to shift the arrow forward */
void Shift_Arrow() {
    for (int i = 0; i < 26; i++) {
        arrow[i] = arrow[i] + 8;
        if (arrow[i] >= 240) {
            arrow[i] = arrow[i] % (LED_ROWS * LED_COLS - 16);
        }
    }
}

/* Function to draw the strips */
void Draw_Strips(WS28XX_HandleTypeDef *ws, int color, int brightness) {
    for (int i = 240; i <= 255; i++) {
        WS28XX_SetPixel_RGBW_565(ws, i, color, brightness);
    }
    for (int i = 0; i <= 15; i++) {
        WS28XX_SetPixel_RGBW_565(ws, i, color, brightness);
    }
    WS28XX_Update(ws);
}

/* Function to draw the "X" shapes */
void Draw_Cross(WS28XX_HandleTypeDef *ws, int *cross, int color, int brightness) {
    for (int i = 0; i < 16; i++) {
        WS28XX_SetPixel_RGBW_565(ws, cross[i], color, brightness);
    }
    WS28XX_Update(ws);
}

/* Function to clear the "X" shapes */
void Clear_Cross(WS28XX_HandleTypeDef *ws, int *cross) {
    for (int i = 0; i < 16; i++) {
        WS28XX_SetPixel_RGBW_565(ws, cross[i], COLOR_RGB565_BLACK, 0);
    }
    WS28XX_Update(ws);
}

/* Function for arrow animation */
void Arrow_Animation() {
    Draw_Strips(&ws, COLOR_RGB565_CYAN, 255);

    while (1) {
        Clear_Arrow(&ws);
        Shift_Arrow();
        Draw_Arrow(&ws, COLOR_RGB565_GREEN, 255);
        HAL_Delay(DELAY_TIME);
    }
}

/* Function for cross animation */
void Cross_Animation() {
    Draw_Strips(&ws, COLOR_RGB565_CYAN, 255);

    while (1) {
        Draw_Cross(&ws, topCross, COLOR_RGB565_RED, 255);
        Draw_Cross(&ws, middleCross, COLOR_RGB565_RED, 255);
        Draw_Cross(&ws, bottomCross, COLOR_RGB565_RED, 255);
        HAL_Delay(BLINK_DELAY);
        Clear_Cross(&ws, topCross);
        Clear_Cross(&ws, middleCross);
        Clear_Cross(&ws, bottomCross);
        HAL_Delay(BLINK_DELAY);
    }
}

void limit_switch(void){
	if (HAL_GPIO_ReadPin(GPIOG, GPIO_PIN_3) == GPIO_PIN_RESET)
	{
	  // If the limit switch is pressed, turn on the LED (PB7)
	  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
	}
	else
	{
	  // If the switch is not pressed, turn off the LED
	  //HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
	}
}


void Speed_Control(int a)
{
	__HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, a);
}

void Direction(int a)
{
	if(a==1)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_SET);

	}
	if(a==0)
	{
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_12, GPIO_PIN_RESET);

	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_3)
	{
		encoder();
	}
}

void encoder(void)
{
	counter++;
	if(counter == 588)
	{
		Speed_Control(0);
		counter = 0;
	}
}
void quarter_cycle_cw(void)
{
	Direction(0);
	Speed_Control(1000);
}
void quarter_cycle_acw(void)
{
	Direction(1);
	Speed_Control(1000);
}

void read_rfid(void){
	if(flag_rev==1)
		  {
		   HAL_UART_Transmit_IT(&huart3, usermsg, sizeof(usermsg));
		   flag_rev=0;


		  }
		  HAL_Delay(1000);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */
  WS28XX_Init(&ws, &htim1, 72, TIM_CHANNEL_1, 256);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
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

  /** Initializes the CPU, AHB and APB buses clocks
  */
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

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) { // Check if the interrupt is from USART1
	        // Process the received data
	        for (int i = 0; i < 12; i++) {
	            processedData[i] = rxData[i + 1]; // Skip the first byte and take the next 12 bytes
	        }
	        processedData[12] = '\0'; // Add the null terminator at the end

	        sprintf(usermsg,"%s",processedData);

	        flag_rev=1;
	        v++;
	    	// Re-enable UART receive interrupt for USART1 to receive the next set of data
	    	HAL_UART_Receive_IT(&huart1, rxData, sizeof(rxData));
	    }

	if (huart->Instance == USART3) { // Check if the interrupt is from USART1
		HAL_UART_Receive_IT(&huart3, &responseData, 1);

        if (responseData == 1) { // If the response is true
            flagSuccess = 1;
        }
        else if (responseData == 0) { // If the response is false
            flagFailure = 1;
        }

	}






}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
