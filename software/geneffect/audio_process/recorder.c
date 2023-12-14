#include "recorder.h" 

#include "usart.h"
#include "wm8978.h"
#include "i2s.h"
#include "delay.h"
#include "string.h"  
#include "math.h"
//#include "arm_math.h"
#include "reverb.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//YTCE STM32F407开发板
//录音机 应用代码	   
//原野电子@YTCE
//http://s8088.taobao.com
//创建日期:2014/6/6
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 原野电子 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	
  
uint8_t *i2srecbuf1;
uint8_t *i2srecbuf2; 

uint16_t* i2srecbuf16;
uint16_t* i2soutbuf16;

uint8_t *i2soutbuf1; 
uint8_t *i2soutbuf2; 

const uint32_t FFT_LEN = 2048;
const float freq_scale = 0.9765625;
float* g_fft_buf;
float* g_fft_buf_temp;

float* array_FFT_output;        
float* array_arm_cmplx_mag;    

I2sDataForm i2sdata[100];

FIL* f_rec=0;		//录音文件	
uint32_t wavsize;		//wav数据大小(字节数,不包括文件头!!)
uint8_t rec_sta=0;		//录音状态
					//[7]:0,没有开启录音;1,已经开启录音;
					//[6:1]:保留
					//[0]:0,正在录音;1,暂停录音;
					
//录音 I2S_DMA接收中断服务函数.在中断里面写入数据
void rec_i2s_dma_rx_callback(void) 
{    
	uint16_t bw;
	uint8_t res;
	if(rec_sta==0X80)//录音模式
	{  
		if(DMA1_Stream3->CR&(1<<19))
		{
			res=f_write(f_rec,i2srecbuf1,I2S_RX_DMA_BUF_SIZE,(UINT*)&bw);//写入文件
			if(res)
			{
				printf("write error:%d\r\n",res);
			}
			 
		}else 
		{
			res=f_write(f_rec,i2srecbuf2,I2S_RX_DMA_BUF_SIZE,(UINT*)&bw);//写入文件
			if(res)
			{
				printf("write error:%d\r\n",res);
			}
		}
		wavsize+=I2S_RX_DMA_BUF_SIZE;
	} 
} 

const double ADC_FLOAT_SCALE = 0.000030517578125;
const double DAC_FLOAT_SCALE = 32768.0;

void adc_float_transfer(I2sDataForm* inputptr, AdcDataForm* outputptr){
	outputptr->left = inputptr->left * ADC_FLOAT_SCALE;
	if (outputptr->left >1.0){
	  outputptr->left -= 2.0;
	}
	outputptr->right = inputptr->right * ADC_FLOAT_SCALE;
	if (outputptr->right >1.0){
	  outputptr->right -= 2.0;
	}
}

void dac_float_transfer( AdcDataForm* inputptr, I2sDataForm* outputptr){
	if (inputptr->left < 0.0){
	  inputptr->left += 2.0;
	}
	outputptr->left = inputptr->left * DAC_FLOAT_SCALE;
	if (inputptr->right < 0.0){
	  inputptr->right += 2.0;
	}
	outputptr->right = inputptr->right * DAC_FLOAT_SCALE;
}

void tremolo_function(AdcDataForm* inputptr){
	static double timecnt = 0;
	double factor = 1;
	timecnt += 2.0833333333e-4;
	//factor = cos(timecnt);
	inputptr->left *= factor;
	inputptr->right *= factor;
}

// FFT
float FFT_Calculation(float *DATA, int num)
{
    static uint8_t init_flag = 0;
		float maxid = 0;
		float maxmag = 0;
		int i = 0;
		int window_size = 40.0 / freq_scale ;
    //static arm_rfft_fast_instance_f32 S;
		if (init_flag == 0){
			//arm_rfft_fast_init_f32(&S, FFT_LEN);        //??????S????
			init_flag = 1;
		}
    
    //arm_rfft_fast_f32(&S, DATA, array_FFT_output, 0);          //fft??? 
    //arm_cmplx_mag_f32(array_FFT_output, array_arm_cmplx_mag, FFT_LEN);  //????  
	  //LCD_Clear(WHITE);
		
		for (i = window_size; i < FFT_LEN / 2 - window_size; i++){
			float x_start = i  * freq_scale * 0.5;
			float y_start = 0;
			
			
			//LCD_DrawLine(x_start, y_start, x_start, array_arm_cmplx_mag[i] *5);
			if ((array_arm_cmplx_mag[i] > 5)){
				int larger_cnt = 0;
				int j = 0;
				float sum_41 = 0 ;
				float sum_p = 0;
				for (j = -window_size; j <= window_size; j++){
						sum_41 += array_arm_cmplx_mag[i +j];
						sum_p += array_arm_cmplx_mag[i +j] * (float)(i +j);
						if (j == 0){
							continue;
						}
						if((array_arm_cmplx_mag[i] > array_arm_cmplx_mag[i +j])){
							larger_cnt++;
						}
						
				}
				
				if (larger_cnt >= window_size *2){
						//maxmag = array_arm_cmplx_mag[i];
						
						
						//POINT_COLOR=GREEN;
						sum_p = sum_p / sum_41;
						maxid = sum_p;
						//LCD_DrawLine(x_start - window_size * freq_scale *0.5, y_start, x_start, 300);
						//LCD_DrawLine(x_start + window_size * freq_scale *0.5,y_start, x_start, 300);
						
						//POINT_COLOR=RED;
						return maxid;
				}
			}
			
		}
    return maxid;  
}  

int fft_tuner_buffer_keeper(float inputsig, float* freq){
		static uint32_t fft_buffer_idx = 0;
	  static uint32_t caller_cnt = 0;
		static uint32_t fft_cnt = 0;
		float temp_freq = 0;
		//float fft_temp[FFT_LEN];
		caller_cnt++;
		fft_cnt++;
	  if (caller_cnt == 8){
			caller_cnt = 0;
			g_fft_buf[fft_buffer_idx] = inputsig;
			fft_buffer_idx++;
			if (fft_buffer_idx > FFT_LEN - 1){
				fft_buffer_idx = 0;
				
			}
			
			if (fft_cnt == 1600){
				fft_cnt = 0;
				
				memset(g_fft_buf_temp, 0, 4 * 2 *FFT_LEN);
				memcpy(g_fft_buf_temp, &(g_fft_buf[fft_buffer_idx]), 4 * (FFT_LEN - fft_buffer_idx));
				if (fft_buffer_idx != 0){
					memcpy(&(g_fft_buf_temp[FFT_LEN - fft_buffer_idx]), g_fft_buf, 4 * (fft_buffer_idx));
				}
				
				//memcpy(g_fft_buf_temp, g_fft_buf, 4 * FFT_LEN);
				temp_freq = FFT_Calculation(g_fft_buf_temp, FFT_LEN);	
				//if (temp_freq > 5){
				*freq = temp_freq;
				//}
				return 1;	
				
			}
		}
		return 0;
}

int check_witch_string(float* stringlist, float freq){
	int i =0;
	
	float mindis = 100;
	float cudis = 0;
	int minidx = -1;
	if (freq < 5){
		return -1;
	}
	for (i = 0; i <6; i++){
		cudis = fabs(freq - stringlist[i]);
		if (cudis< mindis){
			mindis = cudis;
			minidx = i;
		}
	}
	return minidx;
}

void tuner_function(float inputsig){
	static uint32_t fftcnt = 0;
	char string_name[6] = {'E','A','D','G','B','E'};
	float freq_list[6] = {82.4069, 110, 146.8324, 195.9977, 246.9417, 329.6276};
	static float freq = 0;
	int ret = -1;
	if (fft_tuner_buffer_keeper(4 * inputsig, &freq)){
			fftcnt++;
		  freq = freq * 0.9 + 0.1 * (freq * freq_scale);
		
			LCD_DrawLine(freq *0.5, 0, freq *0.5, 300);
			//LCD_Clear(WHITE);
			Show_Str(30,230,240,16,"frequency : ",16,0);
			LCD_ShowxNum(30,250,freq, 4,16,0X80);
			LCD_ShowxNum(30,270,fftcnt,4,16,0X80);
			ret = check_witch_string(freq_list, freq);
			if (ret == -1){
				Show_Str(lcddev.width / 2, lcddev.height / 2,50,50,"Null",24,0);
			}else{
				LCD_ShowChar(lcddev.width / 2, lcddev.height / 2,string_name[ret],24,0);
				
				LCD_DrawLine(lcddev.width / 2 + 10 * (freq - freq_list[ret]), lcddev.height / 2 + 30, 
								lcddev.width / 2 + 10 * (freq - freq_list[ret]), lcddev.height / 2 + 80);
				LCD_DrawLine(lcddev.width / 2 , lcddev.height / 2 + 30, 
								lcddev.width / 2, lcddev.height / 2 + 80);
			}
		}
}

void overdrive_function(AdcDataForm* inputptr){

	inputptr->left *= 4;
	inputptr->right *= 4;
	
	if (inputptr->left > 0.4){
		inputptr->left = 0.4;
	}
	if (inputptr->right > 0.4){
		inputptr->right = 0.4;
	}
	if (inputptr->left < -0.4){
		inputptr->left = -0.4;
	}
	if (inputptr->right < -0.4){
		inputptr->right = -0.4;
	}
}

void effect_dma_rx_callback(void){
	static AdcDataForm adcdataform;
	static float freq = 0;
	static uint32_t fftcnt = 0;
	if(DMA1_Stream3->CR&(1<<19))
		{
			int i = 0;
			
			memcpy(i2sdata, i2srecbuf1, I2S_RX_DMA_BUF_SIZE);
			for (i = 0; i <I2S_RX_DMA_BUF_SIZE /4 ; i++){		
				adc_float_transfer(&(i2sdata[i]), &adcdataform);
				//tremolo_function(&adcdataform);
				
				//tuner_function(adcdataform.right);
				overdrive_function(&adcdataform);
				adcdataform.right = Do_Reverb(adcdataform.right);
				adcdataform.left = adcdataform.right;
				dac_float_transfer(&adcdataform, &(i2sdata[i]));
			}
			memcpy(i2soutbuf1, i2sdata, I2S_RX_DMA_BUF_SIZE);
		}else 
		{
			int i = 0;
			memcpy(i2sdata, i2srecbuf2, I2S_RX_DMA_BUF_SIZE);
			for (i = 0; i <I2S_RX_DMA_BUF_SIZE /4 ; i++){		
				adc_float_transfer(&(i2sdata[i]), &adcdataform);
				//tremolo_function(&adcdataform);
				//tuner_function(adcdataform.right);
				
				overdrive_function(&adcdataform);
				adcdataform.right = Do_Reverb(adcdataform.right);
				adcdataform.left = adcdataform.right;
				dac_float_transfer(&adcdataform, &(i2sdata[i]));
			}
			memcpy(i2soutbuf2, i2sdata, I2S_RX_DMA_BUF_SIZE);
		}
}

void effect_dma_tx_callback(void){
	int ret = 0;
	ret++;
}

const uint16_t i2splaybuf[2]={0XFFFF,0X0000};//2个16位数据,用于录音时I2S Master发送.循环发送0.
//进入PCM 录音模式 		  
void recoder_enter_rec_mode(void)
{
	WM8978_ADDA_Cfg(0,1);		//开启ADC
	WM8978_Input_Cfg(1,1,0);	//开启输入通道(MIC&LINE IN)
	WM8978_Output_Cfg(0,1);		//开启BYPASS输出 
	WM8978_MIC_Gain(46);		//MIC增益设置 
	
	WM8978_I2S_Cfg(2,0);		//飞利浦标准,16位数据长度
	I2S2_Init(I2S_Standard_Phillips,I2S_Mode_MasterTx,I2S_CPOL_Low,I2S_DataFormat_16b);			//飞利浦标准,主机发送,时钟低电平有效,16位帧长度 
	I2S2ext_Init(I2S_Standard_Phillips,I2S_Mode_SlaveRx,I2S_CPOL_Low,I2S_DataFormat_16b);		//飞利浦标准,从机接收,时钟低电平有效,16位帧长度	
	I2S2_SampleRate_Set(16000);	//设置采样率 
 	I2S2_TX_DMA_Init((uint8_t*)&i2splaybuf[0],(uint8_t*)&i2splaybuf[1],1); 		//配置TX DMA 
	DMA1_Stream4->CR&=~(1<<4);	//关闭传输完成中断(这里不用中断送数据) 
	I2S2ext_RX_DMA_Init(i2srecbuf1,i2srecbuf2,I2S_RX_DMA_BUF_SIZE/2); 	//配置RX DMA
  	i2s_rx_callback=rec_i2s_dma_rx_callback;//回调函数指wav_i2s_dma_callback
 	I2S_Play_Start();	//开始I2S数据发送(主机)
	I2S_Rec_Start(); 	//开始I2S数据接收(从机)
	recoder_remindmsg_show(0);
}  

void guitar_enter_effet_mode(void)
{
	WM8978_ADDA_Cfg(1,1);		//开启ADC
	WM8978_Input_Cfg(0,1,0);	//开启输入通道(MIC&LINE IN)
	WM8978_Output_Cfg(1,0);		//开启BYPASS输出 
	WM8978_MIC_Gain(46);		//MIC增益设置 
	
	WM8978_I2S_Cfg(2,0);		//飞利浦标准,16位数据长度
	I2S2_Init(I2S_Standard_Phillips,I2S_Mode_MasterTx,I2S_CPOL_Low,I2S_DataFormat_16b);			//飞利浦标准,主机发送,时钟低电平有效,16位帧长度 
	I2S2ext_Init(I2S_Standard_Phillips,I2S_Mode_SlaveRx,I2S_CPOL_Low,I2S_DataFormat_16b);		//飞利浦标准,从机接收,时钟低电平有效,16位帧长度	
	I2S2_SampleRate_Set(16000);	//设置采样率 
 
	//DMA1_Stream4->CR&=~(1<<4);	//关闭传输完成中断(这里不用中断送数据) 
	I2S2ext_RX_DMA_Init(i2srecbuf1,i2srecbuf2,I2S_RX_DMA_BUF_SIZE/2); 	//配置RX DMA
  	i2s_rx_callback=effect_dma_rx_callback;//回调函数指wav_i2s_dma_callback
	
	I2S2_TX_DMA_Init(i2soutbuf1,i2soutbuf2,I2S_RX_DMA_BUF_SIZE/2); 		//配置TX DMA 
	i2s_tx_callback=effect_dma_tx_callback;//回调函数指wav_i2s_dma_callback
 	I2S_Play_Start();	//开始I2S数据发送(主机)
	
	I2S_Rec_Start(); 	//开始I2S数据接收(从机)
	recoder_remindmsg_show(0);
}  
//进入PCM 放音模式 		  
void recoder_enter_play_mode(void)
{
	WM8978_ADDA_Cfg(1,0);		//开启DAC 
	WM8978_Input_Cfg(0,0,0);	//关闭输入通道(MIC&LINE IN)
	WM8978_Output_Cfg(1,0);		//开启DAC输出 
	WM8978_MIC_Gain(0);			//MIC增益设置为0 
	I2S_Play_Stop();			//停止时钟发送
	I2S_Rec_Stop(); 			//停止录音
	recoder_remindmsg_show(1);
}
//初始化WAV头.
void recoder_wav_init(__WaveHeader* wavhead) //初始化WAV头			   
{
	wavhead->riff.ChunkID=0X46464952;	//"RIFF"
	wavhead->riff.ChunkSize=0;			//还未确定,最后需要计算
	wavhead->riff.Format=0X45564157; 	//"WAVE"
	wavhead->fmt.ChunkID=0X20746D66; 	//"fmt "
	wavhead->fmt.ChunkSize=16; 			//大小为16个字节
	wavhead->fmt.AudioFormat=0X01; 		//0X01,表示PCM;0X01,表示IMA ADPCM
 	wavhead->fmt.NumOfChannels=2;		//双声道
 	wavhead->fmt.SampleRate=16000;		//16Khz采样率 采样速率
 	wavhead->fmt.ByteRate=wavhead->fmt.SampleRate*4;//字节速率=采样率*通道数*(ADC位数/8)
 	wavhead->fmt.BlockAlign=4;			//块大小=通道数*(ADC位数/8)
 	wavhead->fmt.BitsPerSample=16;		//16位PCM
   	wavhead->data.ChunkID=0X61746164;	//"data"
 	wavhead->data.ChunkSize=0;			//数据大小,还需要计算  
} 						    
//显示录音时间和码率
//tsec:秒钟数.
void recoder_msg_show(uint32_t tsec,uint32_t kbps)
{   
	//显示录音时间			 
	LCD_ShowString(30,210,200,16,16,"TIME:");	  	  
	LCD_ShowxNum(30+40,210,tsec/60,2,16,0X80);	//分钟
	LCD_ShowChar(30+56,210,':',16,0);
	LCD_ShowxNum(30+64,210,tsec%60,2,16,0X80);	//秒钟	
	//显示码率		 
	LCD_ShowString(140,210,200,16,16,"KPBS:");	  	  
	LCD_ShowxNum(140+40,210,kbps/1000,4,16,0X80);	//码率显示 	
}  	
//提示信息
//mode:0,录音模式;1,放音模式
void recoder_remindmsg_show(uint8_t mode)
{
	LCD_Fill(30,120,lcddev.width,180,WHITE);//清除原来的显示
	POINT_COLOR=RED;
	if(mode==0)	//录音模式
	{
		Show_Str(30,120,200,16,"KEY0:REC/PAUSE",16,0); 
		Show_Str(30,140,200,16,"KEY2:STOP&SAVE",16,0); 
		Show_Str(30,160,200,16,"WK_UP:PLAY",16,0); 
	}else		//放音模式
	{
		Show_Str(30,120,200,16,"KEY0:STOP Play",16,0);  
		Show_Str(30,140,200,16,"WK_UP:PLAY/PAUSE",16,0); 
	}
}
//通过时间获取文件名
//仅限在SD卡保存,不支持FLASH DISK保存
//组合成:形如"0:RECORDER/REC20120321210633.wav"的文件名
void recoder_new_pathname(uint8_t *pname)
{	 
	uint8_t res;					 
	uint16_t index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:RECORDER/REC%05d.wav",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//尝试打开这个文件
		if(res==FR_NO_FILE)break;		//该文件名不存在=正是我们需要的.
		index++;
	}
} 
//WAV录音 
void guitar_effect(void){
    Show_Str(30,230,240,16,"guitar effect start",16,0);
	  i2srecbuf1 = mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S录音内存1申请
	  i2srecbuf2 = mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S录音内存2申请
		i2soutbuf1 = mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S录音内存1申请
	  i2soutbuf2 = mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S录音内存2申请
	
		/* init buffer for fft*/
		g_fft_buf = mymalloc(SRAMIN, sizeof(float) * FFT_LEN);
		g_fft_buf_temp = mymalloc(SRAMIN, sizeof(float) * 2 *FFT_LEN);
		array_FFT_output = mymalloc(SRAMIN, sizeof(float) * 2 *FFT_LEN);;       
		array_arm_cmplx_mag = mymalloc(SRAMIN, sizeof(float) * 2 *FFT_LEN);;     
	
		/* init buffer for reverb*/
		reverb_init();
		guitar_enter_effet_mode();
	  Show_Str(30,230,240,16,"guitar effect config done",16,0);
    while(1){}	
}

void wav_recorder(void)
{ 
	uint8_t res;
	uint8_t key;
	uint8_t rval=0;
	__WaveHeader *wavhead=0; 
 	DIR recdir;	 					//目录  
 	uint8_t *pname=0;
	uint8_t timecnt=0;					//计时器   
	uint32_t recsec=0;					//录音时间 
  	while(f_opendir(&recdir,"0:/RECORDER"))//打开录音文件夹
 	{	 
		Show_Str(30,230,240,16,"RECORDER文件夹错误!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,230,240,246,WHITE);		//清除显示	     
		delay_ms(200);				  
		f_mkdir("0:/RECORDER");				//创建该目录   
	}   
	i2srecbuf1=mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S录音内存1申请
	i2srecbuf2=mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S录音内存2申请  
  	f_rec=(FIL *)mymalloc(SRAMIN,sizeof(FIL));		//开辟FIL字节的内存区域  
 	wavhead=(__WaveHeader*)mymalloc(SRAMIN,sizeof(__WaveHeader));//开辟__WaveHeader字节的内存区域 
	pname=mymalloc(SRAMIN,30);						//申请30个字节内存,类似"0:RECORDER/REC00001.wav" 
	if(!i2srecbuf1||!i2srecbuf2||!f_rec||!wavhead||!pname)rval=1; 	if(rval==0)		
	{
		recoder_enter_rec_mode();	//进入录音模式,此时耳机可以听到咪头采集到的音频   
		pname[0]=0;					//pname没有任何文件名 
 	   	while(rval==0)
		{
			key=KEY_Scan(0);
			switch(key)
			{		
				case KEY2_PRES:	//STOP&SAVE
					if(rec_sta&0X80)//有录音
					{
						rec_sta=0;	//关闭录音
						wavhead->riff.ChunkSize=wavsize+36;		//整个文件的大小-8;
				   		wavhead->data.ChunkSize=wavsize;		//数据大小
						f_lseek(f_rec,0);						//偏移到文件头.
				  		f_write(f_rec,(const void*)wavhead,sizeof(__WaveHeader),&bw);//写入头数据
						f_close(f_rec);
						wavsize=0;
					}
					rec_sta=0;
					recsec=0;
				 	LED1=1;	 						//关闭DS1
					LCD_Fill(30,190,lcddev.width,lcddev.height,WHITE);//清除显示,清除之前显示的录音文件名		      
					break;	 
				case KEY0_PRES:	//REC/PAUSE
					if(rec_sta&0X01)//原来是暂停,继续录音
					{
						rec_sta&=0XFE;//取消暂停
					}else if(rec_sta&0X80)//已经在录音了,暂停
					{
						rec_sta|=0X01;	//暂停
					}else				//还没开始录音 
					{
						recsec=0;	 
						recoder_new_pathname(pname);			//得到新的名字
						Show_Str(30,190,lcddev.width,16,"录制:",16,0);		   
						Show_Str(30+40,190,lcddev.width,16,pname+11,16,0);//显示当前录音文件名字
				 		recoder_wav_init(wavhead);				//初始化wav数据	
	 					res=f_open(f_rec,(const TCHAR*)pname, FA_CREATE_ALWAYS | FA_WRITE); 
						if(res)			//文件创建失败
						{
							rec_sta=0;	//创建文件失败,不能录音
							rval=0XFE;	//提示是否存在SD卡
						}else 
						{
							res=f_write(f_rec,(const void*)wavhead,sizeof(__WaveHeader),&bw);//写入头数据
							recoder_msg_show(0,0);
 							rec_sta|=0X80;	//开始录音	 
						} 
 					}
					if(rec_sta&0X01)LED1=0;	//提示正在暂停
					else LED1=1;
					break;  
				case WKUP_PRES:	//播放最近一段录音
					if(rec_sta!=0X80)//没有在录音
					{   	 		 				  
						if(pname[0])//如果触摸按键被按下,且pname不为空
						{				 
							Show_Str(30,190,lcddev.width,16,"播放:",16,0);		   
							Show_Str(30+40,190,lcddev.width,16,pname+11,16,0);//显示当播放的文件名字
							recoder_enter_play_mode();	//进入播放模式
							audio_play_song(pname);		//播放pname
							LCD_Fill(30,190,lcddev.width,lcddev.height,WHITE);//清除显示,清除之前显示的录音文件名	  
							recoder_enter_rec_mode();	//重新进入录音模式 
						}
					}
					break;
			} 
			delay_ms(5);
			timecnt++;
			if((timecnt%20)==0)LED0=!LED0;//DS0闪烁  
 			if(recsec!=(wavsize/wavhead->fmt.ByteRate))	//录音时间显示
			{	   
				LED0=!LED0;//DS0闪烁 
				recsec=wavsize/wavhead->fmt.ByteRate;	//录音时间
				recoder_msg_show(recsec,wavhead->fmt.SampleRate*wavhead->fmt.NumOfChannels*wavhead->fmt.BitsPerSample);//显示码率
			}
		}		 
	}    
	myfree(SRAMIN,i2srecbuf1);	//释放内存
	myfree(SRAMIN,i2srecbuf2);	//释放内存  
	myfree(SRAMIN,f_rec);		//释放内存
	myfree(SRAMIN,wavhead);		//释放内存  
	myfree(SRAMIN,pname);		//释放内存  
}



































