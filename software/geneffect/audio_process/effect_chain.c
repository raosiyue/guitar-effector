#include "i2s.h"
#include "effect_chain.h"
#include "string.h"
#include "malloc.h"
#include "arm_math.h"

#define EFFECT_BUF_LEN 200
 /*
AudioData rxdmabuffer[EFFECT_BUF_LEN];
AudioData txdmabuffer[EFFECT_BUF_LEN]; 
AudioDataFloat rxfloatbuffer[EFFECT_BUF_LEN / 2];
*/

#define USEMALLOC_DEFINE 1
#if USEMALLOC_DEFINE
AudioData* rxdmabuffer;
AudioData* txdmabuffer; 
AudioDataFloat* rxfloatbuffer;

#else
AudioData rxdmabuffer[EFFECT_BUF_LEN];
AudioData txdmabuffer[EFFECT_BUF_LEN]; 
AudioDataFloat rxfloatbuffer[EFFECT_BUF_LEN / 2];
#endif
volatile FunctionMode function_mode;
volatile EffectController g_effect_controller;
#define SWITCH_LEVEL_GATE 32768

static inline void overdrive_function(AudioDataFloat* data, uint16_t tap, uint16_t level, uint16_t drive){
	if (tap < SWITCH_LEVEL_GATE){
		function_mode.od_state = 0;
		return;
	}else{
		function_mode.od_state = 1;
		float level_val = 0;
		float drive_val = 0;

		float tempin[2];
		float temout[2];
		level_val = 0.1 + 0.9 * (float)level / 65535.0;
		drive_val =  1 + 50 * ((float)drive / 65535.0);

		tempin[0] = drive_val * -data->left_audio;
		tempin[1] = drive_val * -data->right_audio;
		arm_vexp_f32(tempin, temout, 1);
		//data->left_audio = level_val * (-1 + (2 / (1 + temout[0])));
		data->right_audio = level_val * (-1 + (2 / (1 + temout[1])));
	}
}
//extern AdInfo* adinfoget;
inline void boost_pedal(AudioDataFloat* data, uint16_t tap, uint16_t level){
	if (tap < SWITCH_LEVEL_GATE){
		function_mode.boost_state = 0;
		return;
	}else{
		function_mode.boost_state = 1;
		float level_val = 0;
		level_val = 1.0 + 4.0 * (float)level / 65535.0;
		//data->left_audio *= level_val;
		data->right_audio *= level_val;
	}
}

static inline void pre_ad_calculation(AudioData* data, AudioDataFloat* dataout){
	//dataout->left_audio = (data->left_audio / 32768.0);
	dataout->right_audio = (data->right_audio / 32768.0);
}

static inline void after_ad_calculation(AudioDataFloat * datain, AudioData*  dataout){
	float temp;
	//temp = (datain->left_audio) * 32768;
	//dataout->left_audio = temp;
	//dataout->left_audio = temp > 32767? 32767 : (temp < -32767 ? -32767 : temp);
	temp = (datain->right_audio) * 32768;
	dataout->right_audio = temp > 32767? 32767 : (temp < -32767 ? -32767 : temp);
	//dataout->right_audio = temp;
}

float g_boost_level_val = 0;
float level_val = 0;
float drive_val = 0;

float tempin[2];
float temout[2];

static inline void effect_process(int index){
	
	HAL_GPIO_WritePin(LED1_GPIO_Port , LED1_Pin, 0);
	uint32_t rxfloatbuffer_idx = index;
	
	//pre_ad_calculation(&rxdmabuffer[index], &rxfloatbuffer[rxfloatbuffer_idx]);
	rxfloatbuffer[rxfloatbuffer_idx].right_audio = rxdmabuffer[index].right_audio / 32768.0;
	//HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, 1);
	
	if (g_effect_controller.tune_switch){
		tuner_function(rxfloatbuffer[rxfloatbuffer_idx].right_audio, g_effect_controller.tune_switch);
		rxfloatbuffer[rxfloatbuffer_idx].right_audio = 0;
		rxfloatbuffer[rxfloatbuffer_idx].left_audio = 0;
	}else{
		//function_mode.tunner_state = 0;
		uninit_tuner();
		
		/*boost code begin*/
		if (g_effect_controller.boost_switch){
			rxfloatbuffer[rxfloatbuffer_idx].right_audio *= g_effect_controller.boost_level;
		}
		/*boost code end*/
		
		/*od code begin*/
		if (g_effect_controller.od_switch){
			tempin[0] = g_effect_controller.od_drive * -rxfloatbuffer[rxfloatbuffer_idx].right_audio;
			arm_vexp_f32(tempin, temout, 1);
			rxfloatbuffer[rxfloatbuffer_idx].right_audio = g_effect_controller.od_level * (-1 + (2 / (1 + temout[0])));
		}
		/*od code end*/
		
		if (g_effect_controller.reberb_switch){
			Do_Reverb(&rxfloatbuffer[rxfloatbuffer_idx].right_audio);
			rxfloatbuffer[rxfloatbuffer_idx].left_audio = rxfloatbuffer[rxfloatbuffer_idx].right_audio;
		}
	}
	//HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, 0);
	//after_ad_calculation(&rxfloatbuffer[rxfloatbuffer_idx], &txdmabuffer[index]);
	rxfloatbuffer[rxfloatbuffer_idx].right_audio = rxfloatbuffer[rxfloatbuffer_idx].right_audio * 32768;
	txdmabuffer[index].right_audio = rxfloatbuffer[rxfloatbuffer_idx].right_audio > 32767? 32767 : (rxfloatbuffer[rxfloatbuffer_idx].right_audio < -32767 ? -32767 : rxfloatbuffer[rxfloatbuffer_idx].right_audio);
	HAL_GPIO_WritePin(LED1_GPIO_Port , LED1_Pin, 1);
}

void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
	int i = 0;
	for (i = 0; i < EFFECT_BUF_LEN / 2; i++){
		effect_process(i);
	}
	//memcpy(txdmabuffer, rxdmabuffer, EFFECT_BUF_LEN * 2);
}

void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s){
	int i = 0;
	for (i = EFFECT_BUF_LEN / 2; i < EFFECT_BUF_LEN; i++){
		effect_process(i);
	}
	//memcpy(&txdmabuffer[EFFECT_BUF_LEN / 2], &rxdmabuffer[EFFECT_BUF_LEN / 2], EFFECT_BUF_LEN * 2);
}

void init_input_output_confit(){
	HAL_StatusTypeDef ret;
#if USEMALLOC_DEFINE
	rxdmabuffer = (AudioData*)mymalloc(0, sizeof(AudioData) * EFFECT_BUF_LEN);
	txdmabuffer = (AudioData*)mymalloc(0, sizeof(AudioData) * EFFECT_BUF_LEN);
	rxfloatbuffer = (AudioDataFloat*)mymalloc(0, sizeof(AudioDataFloat) * EFFECT_BUF_LEN);
#endif
	//ret = HAL_I2S_Transmit_DMA(&hi2s2, i2soutbuf1, EFFECT_BUF_LEN);
	//hi2s2.State
	ret = HAL_I2SEx_TransmitReceive_DMA(&hi2s3, (uint16_t*)txdmabuffer, (uint16_t*)rxdmabuffer, EFFECT_BUF_LEN *2);
	reverb_init();
}
