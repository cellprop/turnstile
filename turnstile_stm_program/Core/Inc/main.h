/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define B1_Pin GPIO_PIN_13
#define B1_GPIO_Port GPIOC
#define B1_EXTI_IRQn EXTI15_10_IRQn
#define IR_PULSE_GEN_Pin GPIO_PIN_0
#define IR_PULSE_GEN_GPIO_Port GPIOC
#define IR1_Pin GPIO_PIN_1
#define IR1_GPIO_Port GPIOC
#define IR1_EXTI_IRQn EXTI1_IRQn
#define Encoder_Pin GPIO_PIN_2
#define Encoder_GPIO_Port GPIOC
#define Encoder_EXTI_IRQn EXTI2_IRQn
#define IR_2_Pin GPIO_PIN_3
#define IR_2_GPIO_Port GPIOC
#define IR_2_EXTI_IRQn EXTI3_IRQn
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define IR_3_Pin GPIO_PIN_4
#define IR_3_GPIO_Port GPIOC
#define IR_3_EXTI_IRQn EXTI4_IRQn
#define IR_4_Pin GPIO_PIN_5
#define IR_4_GPIO_Port GPIOC
#define IR_4_EXTI_IRQn EXTI9_5_IRQn
#define Limit_2A_Pin GPIO_PIN_14
#define Limit_2A_GPIO_Port GPIOB
#define Limit_2A_EXTI_IRQn EXTI15_10_IRQn
#define Limit_2B_Pin GPIO_PIN_15
#define Limit_2B_GPIO_Port GPIOB
#define Limit_2B_EXTI_IRQn EXTI15_10_IRQn
#define IR_5_Pin GPIO_PIN_6
#define IR_5_GPIO_Port GPIOC
#define IR_5_EXTI_IRQn EXTI9_5_IRQn
#define IR_6_Pin GPIO_PIN_7
#define IR_6_GPIO_Port GPIOC
#define IR_6_EXTI_IRQn EXTI9_5_IRQn
#define Direction2_Pin GPIO_PIN_8
#define Direction2_GPIO_Port GPIOC
#define Direction1_Pin GPIO_PIN_9
#define Direction1_GPIO_Port GPIOC
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define Limit_1A_Pin GPIO_PIN_10
#define Limit_1A_GPIO_Port GPIOC
#define Limit_1A_EXTI_IRQn EXTI15_10_IRQn
#define Limit_1B_Pin GPIO_PIN_11
#define Limit_1B_GPIO_Port GPIOC
#define Limit_1B_EXTI_IRQn EXTI15_10_IRQn
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
