#ifndef __DELAY_H
#define __DELAY_H 			   
#include <sys.h>	  
//////////////////////////////////////////////////////////////////////////////////  
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ytce STM32F407������
//ʹ��SysTick����ͨ����ģʽ���ӳٽ��й���(֧��ucosii)
//����delay_us,delay_ms
 
////////////////////////////////////////////////////////////////////////////////// 	 
void delay_init(uint8_t SYSCLK);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);

#endif





























