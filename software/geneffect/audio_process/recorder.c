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
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//YTCE STM32F407������
//¼���� Ӧ�ô���	   
//ԭҰ����@YTCE
//http://s8088.taobao.com
//��������:2014/6/6
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ԭҰ���� 2014-2024
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

FIL* f_rec=0;		//¼���ļ�	
uint32_t wavsize;		//wav���ݴ�С(�ֽ���,�������ļ�ͷ!!)
uint8_t rec_sta=0;		//¼��״̬
					//[7]:0,û�п���¼��;1,�Ѿ�����¼��;
					//[6:1]:����
					//[0]:0,����¼��;1,��ͣ¼��;
					
//¼�� I2S_DMA�����жϷ�����.���ж�����д������
void rec_i2s_dma_rx_callback(void) 
{    
	uint16_t bw;
	uint8_t res;
	if(rec_sta==0X80)//¼��ģʽ
	{  
		if(DMA1_Stream3->CR&(1<<19))
		{
			res=f_write(f_rec,i2srecbuf1,I2S_RX_DMA_BUF_SIZE,(UINT*)&bw);//д���ļ�
			if(res)
			{
				printf("write error:%d\r\n",res);
			}
			 
		}else 
		{
			res=f_write(f_rec,i2srecbuf2,I2S_RX_DMA_BUF_SIZE,(UINT*)&bw);//д���ļ�
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

const uint16_t i2splaybuf[2]={0XFFFF,0X0000};//2��16λ����,����¼��ʱI2S Master����.ѭ������0.
//����PCM ¼��ģʽ 		  
void recoder_enter_rec_mode(void)
{
	WM8978_ADDA_Cfg(0,1);		//����ADC
	WM8978_Input_Cfg(1,1,0);	//��������ͨ��(MIC&LINE IN)
	WM8978_Output_Cfg(0,1);		//����BYPASS��� 
	WM8978_MIC_Gain(46);		//MIC�������� 
	
	WM8978_I2S_Cfg(2,0);		//�����ֱ�׼,16λ���ݳ���
	I2S2_Init(I2S_Standard_Phillips,I2S_Mode_MasterTx,I2S_CPOL_Low,I2S_DataFormat_16b);			//�����ֱ�׼,��������,ʱ�ӵ͵�ƽ��Ч,16λ֡���� 
	I2S2ext_Init(I2S_Standard_Phillips,I2S_Mode_SlaveRx,I2S_CPOL_Low,I2S_DataFormat_16b);		//�����ֱ�׼,�ӻ�����,ʱ�ӵ͵�ƽ��Ч,16λ֡����	
	I2S2_SampleRate_Set(16000);	//���ò����� 
 	I2S2_TX_DMA_Init((uint8_t*)&i2splaybuf[0],(uint8_t*)&i2splaybuf[1],1); 		//����TX DMA 
	DMA1_Stream4->CR&=~(1<<4);	//�رմ�������ж�(���ﲻ���ж�������) 
	I2S2ext_RX_DMA_Init(i2srecbuf1,i2srecbuf2,I2S_RX_DMA_BUF_SIZE/2); 	//����RX DMA
  	i2s_rx_callback=rec_i2s_dma_rx_callback;//�ص�����ָwav_i2s_dma_callback
 	I2S_Play_Start();	//��ʼI2S���ݷ���(����)
	I2S_Rec_Start(); 	//��ʼI2S���ݽ���(�ӻ�)
	recoder_remindmsg_show(0);
}  

void guitar_enter_effet_mode(void)
{
	WM8978_ADDA_Cfg(1,1);		//����ADC
	WM8978_Input_Cfg(0,1,0);	//��������ͨ��(MIC&LINE IN)
	WM8978_Output_Cfg(1,0);		//����BYPASS��� 
	WM8978_MIC_Gain(46);		//MIC�������� 
	
	WM8978_I2S_Cfg(2,0);		//�����ֱ�׼,16λ���ݳ���
	I2S2_Init(I2S_Standard_Phillips,I2S_Mode_MasterTx,I2S_CPOL_Low,I2S_DataFormat_16b);			//�����ֱ�׼,��������,ʱ�ӵ͵�ƽ��Ч,16λ֡���� 
	I2S2ext_Init(I2S_Standard_Phillips,I2S_Mode_SlaveRx,I2S_CPOL_Low,I2S_DataFormat_16b);		//�����ֱ�׼,�ӻ�����,ʱ�ӵ͵�ƽ��Ч,16λ֡����	
	I2S2_SampleRate_Set(16000);	//���ò����� 
 
	//DMA1_Stream4->CR&=~(1<<4);	//�رմ�������ж�(���ﲻ���ж�������) 
	I2S2ext_RX_DMA_Init(i2srecbuf1,i2srecbuf2,I2S_RX_DMA_BUF_SIZE/2); 	//����RX DMA
  	i2s_rx_callback=effect_dma_rx_callback;//�ص�����ָwav_i2s_dma_callback
	
	I2S2_TX_DMA_Init(i2soutbuf1,i2soutbuf2,I2S_RX_DMA_BUF_SIZE/2); 		//����TX DMA 
	i2s_tx_callback=effect_dma_tx_callback;//�ص�����ָwav_i2s_dma_callback
 	I2S_Play_Start();	//��ʼI2S���ݷ���(����)
	
	I2S_Rec_Start(); 	//��ʼI2S���ݽ���(�ӻ�)
	recoder_remindmsg_show(0);
}  
//����PCM ����ģʽ 		  
void recoder_enter_play_mode(void)
{
	WM8978_ADDA_Cfg(1,0);		//����DAC 
	WM8978_Input_Cfg(0,0,0);	//�ر�����ͨ��(MIC&LINE IN)
	WM8978_Output_Cfg(1,0);		//����DAC��� 
	WM8978_MIC_Gain(0);			//MIC��������Ϊ0 
	I2S_Play_Stop();			//ֹͣʱ�ӷ���
	I2S_Rec_Stop(); 			//ֹͣ¼��
	recoder_remindmsg_show(1);
}
//��ʼ��WAVͷ.
void recoder_wav_init(__WaveHeader* wavhead) //��ʼ��WAVͷ			   
{
	wavhead->riff.ChunkID=0X46464952;	//"RIFF"
	wavhead->riff.ChunkSize=0;			//��δȷ��,�����Ҫ����
	wavhead->riff.Format=0X45564157; 	//"WAVE"
	wavhead->fmt.ChunkID=0X20746D66; 	//"fmt "
	wavhead->fmt.ChunkSize=16; 			//��СΪ16���ֽ�
	wavhead->fmt.AudioFormat=0X01; 		//0X01,��ʾPCM;0X01,��ʾIMA ADPCM
 	wavhead->fmt.NumOfChannels=2;		//˫����
 	wavhead->fmt.SampleRate=16000;		//16Khz������ ��������
 	wavhead->fmt.ByteRate=wavhead->fmt.SampleRate*4;//�ֽ�����=������*ͨ����*(ADCλ��/8)
 	wavhead->fmt.BlockAlign=4;			//���С=ͨ����*(ADCλ��/8)
 	wavhead->fmt.BitsPerSample=16;		//16λPCM
   	wavhead->data.ChunkID=0X61746164;	//"data"
 	wavhead->data.ChunkSize=0;			//���ݴ�С,����Ҫ����  
} 						    
//��ʾ¼��ʱ�������
//tsec:������.
void recoder_msg_show(uint32_t tsec,uint32_t kbps)
{   
	//��ʾ¼��ʱ��			 
	LCD_ShowString(30,210,200,16,16,"TIME:");	  	  
	LCD_ShowxNum(30+40,210,tsec/60,2,16,0X80);	//����
	LCD_ShowChar(30+56,210,':',16,0);
	LCD_ShowxNum(30+64,210,tsec%60,2,16,0X80);	//����	
	//��ʾ����		 
	LCD_ShowString(140,210,200,16,16,"KPBS:");	  	  
	LCD_ShowxNum(140+40,210,kbps/1000,4,16,0X80);	//������ʾ 	
}  	
//��ʾ��Ϣ
//mode:0,¼��ģʽ;1,����ģʽ
void recoder_remindmsg_show(uint8_t mode)
{
	LCD_Fill(30,120,lcddev.width,180,WHITE);//���ԭ������ʾ
	POINT_COLOR=RED;
	if(mode==0)	//¼��ģʽ
	{
		Show_Str(30,120,200,16,"KEY0:REC/PAUSE",16,0); 
		Show_Str(30,140,200,16,"KEY2:STOP&SAVE",16,0); 
		Show_Str(30,160,200,16,"WK_UP:PLAY",16,0); 
	}else		//����ģʽ
	{
		Show_Str(30,120,200,16,"KEY0:STOP Play",16,0);  
		Show_Str(30,140,200,16,"WK_UP:PLAY/PAUSE",16,0); 
	}
}
//ͨ��ʱ���ȡ�ļ���
//������SD������,��֧��FLASH DISK����
//��ϳ�:����"0:RECORDER/REC20120321210633.wav"���ļ���
void recoder_new_pathname(uint8_t *pname)
{	 
	uint8_t res;					 
	uint16_t index=0;
	while(index<0XFFFF)
	{
		sprintf((char*)pname,"0:RECORDER/REC%05d.wav",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//���Դ�����ļ�
		if(res==FR_NO_FILE)break;		//���ļ���������=����������Ҫ��.
		index++;
	}
} 
//WAV¼�� 
void guitar_effect(void){
    Show_Str(30,230,240,16,"guitar effect start",16,0);
	  i2srecbuf1 = mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S¼���ڴ�1����
	  i2srecbuf2 = mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S¼���ڴ�2����
		i2soutbuf1 = mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S¼���ڴ�1����
	  i2soutbuf2 = mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S¼���ڴ�2����
	
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
 	DIR recdir;	 					//Ŀ¼  
 	uint8_t *pname=0;
	uint8_t timecnt=0;					//��ʱ��   
	uint32_t recsec=0;					//¼��ʱ�� 
  	while(f_opendir(&recdir,"0:/RECORDER"))//��¼���ļ���
 	{	 
		Show_Str(30,230,240,16,"RECORDER�ļ��д���!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,230,240,246,WHITE);		//�����ʾ	     
		delay_ms(200);				  
		f_mkdir("0:/RECORDER");				//������Ŀ¼   
	}   
	i2srecbuf1=mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S¼���ڴ�1����
	i2srecbuf2=mymalloc(SRAMIN,I2S_RX_DMA_BUF_SIZE);//I2S¼���ڴ�2����  
  	f_rec=(FIL *)mymalloc(SRAMIN,sizeof(FIL));		//����FIL�ֽڵ��ڴ�����  
 	wavhead=(__WaveHeader*)mymalloc(SRAMIN,sizeof(__WaveHeader));//����__WaveHeader�ֽڵ��ڴ����� 
	pname=mymalloc(SRAMIN,30);						//����30���ֽ��ڴ�,����"0:RECORDER/REC00001.wav" 
	if(!i2srecbuf1||!i2srecbuf2||!f_rec||!wavhead||!pname)rval=1; 	if(rval==0)		
	{
		recoder_enter_rec_mode();	//����¼��ģʽ,��ʱ��������������ͷ�ɼ�������Ƶ   
		pname[0]=0;					//pnameû���κ��ļ��� 
 	   	while(rval==0)
		{
			key=KEY_Scan(0);
			switch(key)
			{		
				case KEY2_PRES:	//STOP&SAVE
					if(rec_sta&0X80)//��¼��
					{
						rec_sta=0;	//�ر�¼��
						wavhead->riff.ChunkSize=wavsize+36;		//�����ļ��Ĵ�С-8;
				   		wavhead->data.ChunkSize=wavsize;		//���ݴ�С
						f_lseek(f_rec,0);						//ƫ�Ƶ��ļ�ͷ.
				  		f_write(f_rec,(const void*)wavhead,sizeof(__WaveHeader),&bw);//д��ͷ����
						f_close(f_rec);
						wavsize=0;
					}
					rec_sta=0;
					recsec=0;
				 	LED1=1;	 						//�ر�DS1
					LCD_Fill(30,190,lcddev.width,lcddev.height,WHITE);//�����ʾ,���֮ǰ��ʾ��¼���ļ���		      
					break;	 
				case KEY0_PRES:	//REC/PAUSE
					if(rec_sta&0X01)//ԭ������ͣ,����¼��
					{
						rec_sta&=0XFE;//ȡ����ͣ
					}else if(rec_sta&0X80)//�Ѿ���¼����,��ͣ
					{
						rec_sta|=0X01;	//��ͣ
					}else				//��û��ʼ¼�� 
					{
						recsec=0;	 
						recoder_new_pathname(pname);			//�õ��µ�����
						Show_Str(30,190,lcddev.width,16,"¼��:",16,0);		   
						Show_Str(30+40,190,lcddev.width,16,pname+11,16,0);//��ʾ��ǰ¼���ļ�����
				 		recoder_wav_init(wavhead);				//��ʼ��wav����	
	 					res=f_open(f_rec,(const TCHAR*)pname, FA_CREATE_ALWAYS | FA_WRITE); 
						if(res)			//�ļ�����ʧ��
						{
							rec_sta=0;	//�����ļ�ʧ��,����¼��
							rval=0XFE;	//��ʾ�Ƿ����SD��
						}else 
						{
							res=f_write(f_rec,(const void*)wavhead,sizeof(__WaveHeader),&bw);//д��ͷ����
							recoder_msg_show(0,0);
 							rec_sta|=0X80;	//��ʼ¼��	 
						} 
 					}
					if(rec_sta&0X01)LED1=0;	//��ʾ������ͣ
					else LED1=1;
					break;  
				case WKUP_PRES:	//�������һ��¼��
					if(rec_sta!=0X80)//û����¼��
					{   	 		 				  
						if(pname[0])//�����������������,��pname��Ϊ��
						{				 
							Show_Str(30,190,lcddev.width,16,"����:",16,0);		   
							Show_Str(30+40,190,lcddev.width,16,pname+11,16,0);//��ʾ�����ŵ��ļ�����
							recoder_enter_play_mode();	//���벥��ģʽ
							audio_play_song(pname);		//����pname
							LCD_Fill(30,190,lcddev.width,lcddev.height,WHITE);//�����ʾ,���֮ǰ��ʾ��¼���ļ���	  
							recoder_enter_rec_mode();	//���½���¼��ģʽ 
						}
					}
					break;
			} 
			delay_ms(5);
			timecnt++;
			if((timecnt%20)==0)LED0=!LED0;//DS0��˸  
 			if(recsec!=(wavsize/wavhead->fmt.ByteRate))	//¼��ʱ����ʾ
			{	   
				LED0=!LED0;//DS0��˸ 
				recsec=wavsize/wavhead->fmt.ByteRate;	//¼��ʱ��
				recoder_msg_show(recsec,wavhead->fmt.SampleRate*wavhead->fmt.NumOfChannels*wavhead->fmt.BitsPerSample);//��ʾ����
			}
		}		 
	}    
	myfree(SRAMIN,i2srecbuf1);	//�ͷ��ڴ�
	myfree(SRAMIN,i2srecbuf2);	//�ͷ��ڴ�  
	myfree(SRAMIN,f_rec);		//�ͷ��ڴ�
	myfree(SRAMIN,wavhead);		//�ͷ��ڴ�  
	myfree(SRAMIN,pname);		//�ͷ��ڴ�  
}



































