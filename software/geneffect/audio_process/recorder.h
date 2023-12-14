#ifndef __RECORDER_H
#define __RECORDER_H
#include "sys.h"

#define I2S_RX_DMA_BUF_SIZE    	100		//定义RX DMA 数组大小

typedef struct I2sDataForm_s{
	  uint16_t left;
	  uint16_t right;
}I2sDataForm;

typedef struct AdcDataForm_s{
	  float left;
	  float right;
}AdcDataForm;

void rec_i2s_dma_rx_callback(void);
void recoder_enter_rec_mode(void);
void recoder_msg_show(uint32_t tsec,uint32_t kbps);
void recoder_remindmsg_show(uint8_t mode);
void recoder_new_pathname(uint8_t *pname); 
void wav_recorder(void);
void guitar_effect(void);

#endif












