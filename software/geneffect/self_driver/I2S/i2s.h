#ifndef __I2S_H
#define __I2S_H
#include "sys.h"    

#include "stm32f4xx_adc.h"
#include "stm32f4xx_crc.h"
#include "stm32f4xx_dbgmcu.h"
#include "stm32f4xx_dma.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_flash.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_i2c.h"
#include "stm32f4xx_iwdg.h"
#include "stm32f4xx_pwr.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_rtc.h"
#include "stm32f4xx_sdio.h"
#include "stm32f4xx_spi.h"
#include "stm32f4xx_syscfg.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_usart.h"
#include "stm32f4xx_wwdg.h"
#include "misc.h" /* High level functions for NVIC and SysTick (add-on to CMSIS functions) */

#include "stm32f4xx_cryp.h"
#include "stm32f4xx_hash.h"
#include "stm32f4xx_rng.h"
#include "stm32f4xx_can.h"
#include "stm32f4xx_dac.h"
#include "stm32f4xx_dcmi.h"
#include "stm32f4xx_fsmc.h"



 

extern void (*i2s_tx_callback)(void);		//IIS TX回调函数指针 
extern void (*i2s_rx_callback)(void);		//IIS RX回调函数指针 

void I2S2_Init(uint16_t I2S_Standard,uint16_t I2S_Mode,uint16_t I2S_Clock_Polarity,uint16_t I2S_DataFormat);
void I2S2ext_Init(uint16_t I2S_Standard,uint16_t I2S_Mode,uint16_t I2S_Clock_Polarity,uint16_t I2S_DataFormat);
uint8_t I2S2_SampleRate_Set(uint32_t samplerate);
void I2S2_TX_DMA_Init(uint8_t* buf0,uint8_t *buf1,uint16_t num);
void I2S2ext_RX_DMA_Init(uint8_t* buf0,uint8_t *buf1,uint16_t num);
void I2S_Play_Start(void);
void I2S_Rec_Start(void);
void I2S_Play_Stop(void);
void I2S_Rec_Stop(void);
 

#endif





















