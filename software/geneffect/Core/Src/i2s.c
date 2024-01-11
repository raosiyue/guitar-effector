/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    i2s.c
  * @brief   This file provides code for the configuration
  *          of the I2S instances.
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
#include "i2s.h"

/* USER CODE BEGIN 0 */
uint8_t i2s_init_flag = 1;
/* USER CODE END 0 */

I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi3_tx;
DMA_HandleTypeDef hdma_spi3_rx;

/* I2S3 init function */
void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */
	if (i2s_init_flag == 0){
		hi2s3.Instance = SPI3;
		hi2s3.Init.Mode = I2S_MODE_MASTER_FULLDUPLEX;
		hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
		hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
		hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
		hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_16K;
		hi2s3.Init.CPOL = I2S_CPOL_LOW;
		hi2s3.Init.FirstBit = I2S_FIRSTBIT_MSB;
		hi2s3.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
		hi2s3.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
		hi2s3.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_ENABLE;
		if (HAL_I2S_Init(&hi2s3) != HAL_OK)
		{
			Error_Handler();
		}
		i2s_init_flag = 1;
		return;
	}
	
  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_MASTER_FULLDUPLEX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_16B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.FirstBit = I2S_FIRSTBIT_MSB;
  hi2s3.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
  hi2s3.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
  hi2s3.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_ENABLE;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

}

void HAL_I2S_MspInit(I2S_HandleTypeDef* i2sHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if(i2sHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspInit 0 */
		if (i2s_init_flag == 0){
			PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3;
			PeriphClkInitStruct.PLL3.PLL3M = 24;
			PeriphClkInitStruct.PLL3.PLL3N = 213;
			PeriphClkInitStruct.PLL3.PLL3P = 2;
			PeriphClkInitStruct.PLL3.PLL3Q = 3;
			PeriphClkInitStruct.PLL3.PLL3R = 3;
			PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
			PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
			PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
			PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL3;
			if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
			{
				Error_Handler();
			}

			/* I2S3 clock enable */
			__HAL_RCC_SPI3_CLK_ENABLE();

			__HAL_RCC_GPIOA_CLK_ENABLE();
			__HAL_RCC_GPIOB_CLK_ENABLE();
			__HAL_RCC_GPIOC_CLK_ENABLE();
			/**I2S3 GPIO Configuration
			PA4     ------> I2S3_WS
			PB2     ------> I2S3_SDO
			PC7     ------> I2S3_MCK
			PC10     ------> I2S3_CK
			PC11     ------> I2S3_SDI
			*/
			GPIO_InitStruct.Pin = GPIO_PIN_4;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

			GPIO_InitStruct.Pin = GPIO_PIN_2;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF7_SPI3;
			HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

			GPIO_InitStruct.Pin = GPIO_PIN_7;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
			GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
			HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

			GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
			GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
			HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

			/* I2S3 DMA Init */
			/* SPI3_TX Init */
			hdma_spi3_tx.Instance = DMA1_Stream3;
			hdma_spi3_tx.Init.Request = DMA_REQUEST_SPI3_TX;
			hdma_spi3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
			hdma_spi3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
			hdma_spi3_tx.Init.MemInc = DMA_MINC_ENABLE;
			hdma_spi3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
			hdma_spi3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
			hdma_spi3_tx.Init.Mode = DMA_CIRCULAR;
			hdma_spi3_tx.Init.Priority = DMA_PRIORITY_LOW;
			hdma_spi3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
			if (HAL_DMA_Init(&hdma_spi3_tx) != HAL_OK)
			{
				Error_Handler();
			}

			__HAL_LINKDMA(i2sHandle,hdmatx,hdma_spi3_tx);

			/* SPI3_RX Init */
			hdma_spi3_rx.Instance = DMA1_Stream2;
			hdma_spi3_rx.Init.Request = DMA_REQUEST_SPI3_RX;
			hdma_spi3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
			hdma_spi3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
			hdma_spi3_rx.Init.MemInc = DMA_MINC_ENABLE;
			hdma_spi3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
			hdma_spi3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
			hdma_spi3_rx.Init.Mode = DMA_CIRCULAR;
			hdma_spi3_rx.Init.Priority = DMA_PRIORITY_LOW;
			hdma_spi3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
			if (HAL_DMA_Init(&hdma_spi3_rx) != HAL_OK)
			{
				Error_Handler();
			}

			__HAL_LINKDMA(i2sHandle,hdmarx,hdma_spi3_rx);

			/* I2S3 interrupt Init */
			HAL_NVIC_SetPriority(SPI3_IRQn, 0, 0);
			HAL_NVIC_EnableIRQ(SPI3_IRQn);
			return;
		}
		
  /* USER CODE END SPI3_MspInit 0 */

  /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI3;
    PeriphClkInitStruct.PLL3.PLL3M = 24;
    PeriphClkInitStruct.PLL3.PLL3N = 258;
    PeriphClkInitStruct.PLL3.PLL3P = 3;
    PeriphClkInitStruct.PLL3.PLL3Q = 3;
    PeriphClkInitStruct.PLL3.PLL3R = 3;
    PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
    PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
    PeriphClkInitStruct.PLL3.PLL3FRACN = 0;
    PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL3;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    /* I2S3 clock enable */
    __HAL_RCC_SPI3_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    /**I2S3 GPIO Configuration
    PA4     ------> I2S3_WS
    PB2     ------> I2S3_SDO
    PC7     ------> I2S3_MCK
    PC10     ------> I2S3_CK
    PC11     ------> I2S3_SDI
    */
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_SPI3;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* I2S3 DMA Init */
    /* SPI3_TX Init */
    hdma_spi3_tx.Instance = DMA1_Stream3;
    hdma_spi3_tx.Init.Request = DMA_REQUEST_SPI3_TX;
    hdma_spi3_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi3_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi3_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi3_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi3_tx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_spi3_tx.Init.Mode = DMA_CIRCULAR;
    hdma_spi3_tx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_spi3_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_spi3_tx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(i2sHandle,hdmatx,hdma_spi3_tx);

    /* SPI3_RX Init */
    hdma_spi3_rx.Instance = DMA1_Stream2;
    hdma_spi3_rx.Init.Request = DMA_REQUEST_SPI3_RX;
    hdma_spi3_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi3_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi3_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi3_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_spi3_rx.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hdma_spi3_rx.Init.Mode = DMA_CIRCULAR;
    hdma_spi3_rx.Init.Priority = DMA_PRIORITY_LOW;
    hdma_spi3_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    if (HAL_DMA_Init(&hdma_spi3_rx) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(i2sHandle,hdmarx,hdma_spi3_rx);

    /* I2S3 interrupt Init */
    HAL_NVIC_SetPriority(SPI3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPI3_IRQn);
  /* USER CODE BEGIN SPI3_MspInit 1 */

  /* USER CODE END SPI3_MspInit 1 */
  }
}

void HAL_I2S_MspDeInit(I2S_HandleTypeDef* i2sHandle)
{

  if(i2sHandle->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspDeInit 0 */

  /* USER CODE END SPI3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_SPI3_CLK_DISABLE();

    /**I2S3 GPIO Configuration
    PA4     ------> I2S3_WS
    PB2     ------> I2S3_SDO
    PC7     ------> I2S3_MCK
    PC10     ------> I2S3_CK
    PC11     ------> I2S3_SDI
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_2);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_7|GPIO_PIN_10|GPIO_PIN_11);

    /* I2S3 DMA DeInit */
    HAL_DMA_DeInit(i2sHandle->hdmatx);
    HAL_DMA_DeInit(i2sHandle->hdmarx);

    /* I2S3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(SPI3_IRQn);
  /* USER CODE BEGIN SPI3_MspDeInit 1 */

  /* USER CODE END SPI3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
