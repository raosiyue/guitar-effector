#include "effect_chain.h"
#include "malloc.h"
#include "arm_math.h"
#include "usart.h"
#include "driver_oled.h"
uint32_t FFT_LEN = 2048;
float freq_scale = 0.48828125 * 2;
float* g_fft_buf;
float* g_fft_buf_temp;

float* array_FFT_output;        
float* array_arm_cmplx_mag;   
volatile uint8_t tuner_init_flag = 0;
extern void usblog(const char* format, ...);
volatile Header* header;
void init_tuner(){
	if (tuner_init_flag == 0){
		g_fft_buf = (float*)mymalloc(0, sizeof(float) * FFT_LEN);
		g_fft_buf_temp = (float*)mymalloc(0, sizeof(float) * 2 *FFT_LEN);
		array_FFT_output = (float*)mymalloc(0, sizeof(float) * 2 *FFT_LEN);;       
		array_arm_cmplx_mag = (float*)mymalloc(0, sizeof(float) * 2 *FFT_LEN);; 
		header = (Header*)mymalloc(0, sizeof(Header));
		tuner_init_flag = 1;
		function_mode.tunner_state = 1;
	}	
}


void uninit_tuner(){
	if (tuner_init_flag == 1){
		//memset();
		myfree(0, g_fft_buf);
		g_fft_buf = 0;
		myfree(0, g_fft_buf_temp);
		g_fft_buf_temp = 0;
		myfree(0, array_FFT_output);
		array_FFT_output = 0;
		myfree(0, array_arm_cmplx_mag);
		array_arm_cmplx_mag = 0;
		myfree(0, header);
		header = 0;
		tuner_init_flag = 0;
		function_mode.tunner_state = 0;
	}
}



float FFT_Calculation(float *DATA, int num)
{
    static uint8_t init_flag = 0;
		float maxid = 0;
		float maxmag = 0;
		float energy = 0;
		float search_cnt = 0;
		float threshould = 0;
		int i = 0;
		int window_size = 40.0 / freq_scale ;
		float valid_freq[10] = {0.0};
		int valid_freq_cnt = 0;
    static arm_rfft_fast_instance_f32 S;
		//if (init_flag == 0){
		arm_rfft_fast_init_f32(&S, FFT_LEN);        //??????S????
		init_flag = 1;
		//}
    
    arm_rfft_fast_f32(&S, DATA, array_FFT_output, 0);          //fft??? 
    arm_cmplx_mag_f32(array_FFT_output, array_arm_cmplx_mag, FFT_LEN);  //????  
	  //LCD_Clear(WHITE);
		header->header_id = 0x12345678;
		header->datatype = 0x88;
		header->datalen = sizeof(float) *FFT_LEN / 2;
		memcpy(header->data, (const uint8_t*)array_arm_cmplx_mag, sizeof(float) *FFT_LEN / 2);
		
		for (i = window_size; i < FFT_LEN / 2 - window_size; i++){
			energy += array_arm_cmplx_mag[i];
			search_cnt += 1.0;
		}
		
		energy = energy / search_cnt;
		threshould = energy * 10;
		if (threshould < 2.0) {
			threshould = 2.0;
		}
		for (i = window_size; i < FFT_LEN / 2 - window_size; i++){
			if ((array_arm_cmplx_mag[i] > threshould)){
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
						sum_p = sum_p / sum_41;
						maxid = sum_p;
						valid_freq[valid_freq_cnt] = maxid;
						valid_freq_cnt++;
				}
			}
			
		}
		if (valid_freq_cnt){
			int out_search = 0;
			float outfreq = 0;
			for (out_search = 0; (out_search <valid_freq_cnt); out_search++){
				
					float scale = round(valid_freq[out_search] / valid_freq[0]);
					outfreq += valid_freq[out_search] / scale;
			}
			outfreq = outfreq / valid_freq_cnt;
			return outfreq;
		}else{
			return 0.0;
		}
    
}  

static uint32_t fft_buffer_idx = 0;
static uint32_t caller_cnt = 0;
static uint32_t fft_cnt = 0;
float temp_freq = 0;

int fft_tuner_buffer_keeper(float inputsig, float* freq){
		
		//float fft_temp[FFT_LEN];
		caller_cnt++;
		fft_cnt++;
	  if (caller_cnt == 24){
			caller_cnt = 0;
			g_fft_buf[fft_buffer_idx] = inputsig;
			fft_buffer_idx++;
			if (fft_buffer_idx > FFT_LEN - 1){
				fft_buffer_idx = 0;
				
			}
			
			if (fft_cnt == 4800){
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

void tuner_function(float inputsig, uint16_t switch_tap){
	static uint32_t fftcnt = 0;
	char string_name[6] = {'E','A','D','G','B','E'};
	float freq_list[6] = {82.4069, 110, 146.8324, 195.9977, 246.9417, 329.6276};
	static float freq = 0;
	float temp_freq = 0;
	int ret = -1;
	
	if (tuner_init_flag == 0){
		init_tuner();
	}
	
	if (fft_tuner_buffer_keeper(4 * inputsig, &temp_freq)){
			fftcnt++;
		  freq = freq * 0.2 + 0.8 * (temp_freq * freq_scale);
		
			//LCD_DrawLine(freq *0.5, 0, freq *0.5, 300);
			////LCD_Clear(WHITE);
			//Show_Str(30,230,240,16,"frequency : ",16,0);
			//LCD_ShowxNum(30,250,freq, 4,16,0X80);
			//LCD_ShowxNum(30,270,fftcnt,4,16,0X80);
			ret = check_witch_string(freq_list, temp_freq * freq_scale);
			if (ret == -1){
				//function_mode.tunner_state = 1;
				function_mode.string_name = 'N';
				function_mode.freq = 0;
				function_mode.std_freq = 0;
				//Show_Str(lcddev.width / 2, lcddev.height / 2,50,50,"Null",24,0);
				/*
				usblog("tuner_mode : NULL\n");
				OLED_PrintString(0, 0, "Tune:");
				OLED_PrintString(7, 0, "N");
				
				*/
			}else{
				//function_mode.tunner_state = 2;
				//LCD_ShowChar(lcddev.width / 2, lcddev.height / 2,string_name[ret],24,0);
				
				//LCD_DrawLine(lcddev.width / 2 + 10 * (freq - freq_list[ret]), lcddev.height / 2 + 30, 
				//				lcddev.width / 2 + 10 * (freq - freq_list[ret]), lcddev.height / 2 + 80);
				//LCD_DrawLine(lcddev.width / 2 , lcddev.height / 2 + 30, 
				//				lcddev.width / 2, lcddev.height / 2 + 80);
				function_mode.string_name = string_name[ret];
				function_mode.freq = freq;
				function_mode.std_freq = freq_list[ret];
				/*
				usblog("tuner_mode : %c, %f, %f \n", string_name[ret], freq - freq_list[ret], freq);
				
				OLED_PrintString(0, 0, "Tune:");
				OLED_PutChar(7, 0, string_name[ret]);
				//OLED_PutChar(7, 0, 'I');
				OLED_ClearLine(5, 2);
				OLED_ClearLine(5, 4);
				OLED_PrintSignedVal(5, 2, freq_list[ret]);
				OLED_PrintSignedVal(5, 4, freq);
				*/
			}
		}
}