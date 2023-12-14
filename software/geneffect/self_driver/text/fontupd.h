#ifndef __FONTUPD_H__
#define __FONTUPD_H__	 
#include <stm32f4xx.h>
//////////////////////////////////////////////////////////////////////////////////	 
 

//字体信息保存地址,占33个字节,第1个字节用于标记字库是否存在.后续每8个字节一组,分别保存起始地址和文件大小														   
extern uint32_t FONTINFOADDR;	
//字库信息结构体定义
//用来保存字库基本信息，地址，大小等
__packed typedef struct 
{
	uint8_t fontok;				//字库存在标志，0XAA，字库正常；其他，字库不存在
	uint32_t ugbkaddr; 			//unigbk的地址
	uint32_t ugbksize;			//unigbk的大小	 
	uint32_t f12addr;			//gbk12地址	
	uint32_t gbk12size;			//gbk12的大小	 
	uint32_t f16addr;			//gbk16地址
	uint32_t gbk16size;			//gbk16的大小		 
	uint32_t f24addr;			//gbk24地址
	uint32_t gkb24size;			//gbk24的大小 
}_font_info; 


extern _font_info ftinfo;	//字库信息结构体

uint32_t fupd_prog(uint16_t x,uint16_t y,uint8_t size,uint32_t fsize,uint32_t pos);	//显示更新进度
uint8_t updata_fontx(uint16_t x,uint16_t y,uint8_t size,uint8_t *fxpath,uint8_t fx);	//更新指定字库
uint8_t update_font(uint16_t x,uint16_t y,uint8_t size,uint8_t* src);			//更新全部字库
uint8_t font_init(void);										//初始化字库
#endif





















