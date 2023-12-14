#include "sys.h"  
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//YTCE STM32F407开发板
//系统时钟初始化	
//包括时钟设置/中断管理/GPIO设置等
//原野电子@YTCE
 
//Copyright(C) 原野数码电子 2014-2024
//All rights reserved
//********************************************************************************
//修改说明
//无
//////////////////////////////////////////////////////////////////////////////////  


//THUMB指令不支持汇编内联
//采用如下方法实现执行汇编指令WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//关闭所有中断(但是不包括fault和NMI中断)
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//开启所有中断
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//设置栈顶地址
//addr:栈顶地址
__asm void MSR_MSP(uint32_t addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
















