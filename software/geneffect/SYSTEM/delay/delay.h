#ifndef __DELAY_H
#define __DELAY_H 			   
#include <sys.h>	  
//////////////////////////////////////////////////////////////////////////////////  
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ytce STM32F407开发板
//使用SysTick的普通计数模式对延迟进行管理(支持ucosii)
//包括delay_us,delay_ms
 
////////////////////////////////////////////////////////////////////////////////// 	 
void delay_init(uint8_t SYSCLK);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);

#endif





























