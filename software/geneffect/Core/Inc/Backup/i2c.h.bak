/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2c.h
  * @brief   This file contains all the function prototypes for
  *          the i2c.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#ifndef __I2C_H__
#define __I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern I2C_HandleTypeDef hi2c1;

extern I2C_HandleTypeDef hi2c3;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_I2C1_Init(void);
void MX_I2C3_Init(void);

/* USER CODE BEGIN Prototypes */
typedef struct ad_info_s{
	uint16_t tune_tap;
	uint16_t boost_tap;
	uint16_t od_tap;
	uint16_t reverb_tap;
	uint16_t boostlevel;
	uint16_t odlevel;
	uint16_t oddrive;
	uint16_t odtone;
	uint16_t reverb_mix;
	uint16_t reverb_time;
	uint16_t delay_mix;
	uint16_t delay_time;
}AdInfo;

extern AdInfo* adinfoget;
extern uint8_t i2cram[24];  
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __I2C_H__ */

