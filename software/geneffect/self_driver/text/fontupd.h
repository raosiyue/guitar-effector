#ifndef __FONTUPD_H__
#define __FONTUPD_H__	 
#include <stm32f4xx.h>
//////////////////////////////////////////////////////////////////////////////////	 
 

//������Ϣ�����ַ,ռ33���ֽ�,��1���ֽ����ڱ���ֿ��Ƿ����.����ÿ8���ֽ�һ��,�ֱ𱣴���ʼ��ַ���ļ���С														   
extern uint32_t FONTINFOADDR;	
//�ֿ���Ϣ�ṹ�嶨��
//���������ֿ������Ϣ����ַ����С��
__packed typedef struct 
{
	uint8_t fontok;				//�ֿ���ڱ�־��0XAA���ֿ��������������ֿⲻ����
	uint32_t ugbkaddr; 			//unigbk�ĵ�ַ
	uint32_t ugbksize;			//unigbk�Ĵ�С	 
	uint32_t f12addr;			//gbk12��ַ	
	uint32_t gbk12size;			//gbk12�Ĵ�С	 
	uint32_t f16addr;			//gbk16��ַ
	uint32_t gbk16size;			//gbk16�Ĵ�С		 
	uint32_t f24addr;			//gbk24��ַ
	uint32_t gkb24size;			//gbk24�Ĵ�С 
}_font_info; 


extern _font_info ftinfo;	//�ֿ���Ϣ�ṹ��

uint32_t fupd_prog(uint16_t x,uint16_t y,uint8_t size,uint32_t fsize,uint32_t pos);	//��ʾ���½���
uint8_t updata_fontx(uint16_t x,uint16_t y,uint8_t size,uint8_t *fxpath,uint8_t fx);	//����ָ���ֿ�
uint8_t update_font(uint16_t x,uint16_t y,uint8_t size,uint8_t* src);			//����ȫ���ֿ�
uint8_t font_init(void);										//��ʼ���ֿ�
#endif





















