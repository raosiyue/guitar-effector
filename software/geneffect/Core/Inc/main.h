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
#include "stm32h7xx_hal.h"

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
#define SPI4_CS_Pin GPIO_PIN_3
#define SPI4_CS_GPIO_Port GPIOE
#define SPI4_DC_Pin GPIO_PIN_4
#define SPI4_DC_GPIO_Port GPIOE
#define SPI4_RES_Pin GPIO_PIN_13
#define SPI4_RES_GPIO_Port GPIOC
#define Analog_S0_Pin GPIO_PIN_7
#define Analog_S0_GPIO_Port GPIOF
#define Analog_S1_Pin GPIO_PIN_8
#define Analog_S1_GPIO_Port GPIOF
#define Analog_S2_Pin GPIO_PIN_9
#define Analog_S2_GPIO_Port GPIOF
#define I2C_CODEC_SCK_Pin GPIO_PIN_5
#define I2C_CODEC_SCK_GPIO_Port GPIOC
#define I2C_CODEC_SDA_Pin GPIO_PIN_0
#define I2C_CODEC_SDA_GPIO_Port GPIOB
#define LED4_Pin GPIO_PIN_8
#define LED4_GPIO_Port GPIOA
#define LED3_Pin GPIO_PIN_9
#define LED3_GPIO_Port GPIOA
#define LED2_Pin GPIO_PIN_10
#define LED2_GPIO_Port GPIOA
#define LED1_Pin GPIO_PIN_11
#define LED1_GPIO_Port GPIOA
#define LED0_Pin GPIO_PIN_12
#define LED0_GPIO_Port GPIOA
#define SPI4_BLK_Pin GPIO_PIN_9
#define SPI4_BLK_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
