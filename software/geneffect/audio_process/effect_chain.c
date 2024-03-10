#include "i2s.h"
#include "effect_chain.h"
#include "string.h"
#include "malloc.h"
#include "arm_math.h"
#include "lowpass_filter.h"
#include "reverb.h"
#include "tremolo.h"
//#include "dragonfly_reverb_hall.h"
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
float buffer_output[2][EFFECT_BUF_LEN];
float buffer_input[2][EFFECT_BUF_LEN];
float* buffer_input_ptr[2] = {buffer_input[0], buffer_input[1]};
float* buffer_output_ptr[2] = {buffer_output[0], buffer_output[1]};



#else
AudioData rxdmabuffer[EFFECT_BUF_LEN];
AudioData txdmabuffer[EFFECT_BUF_LEN]; 
AudioDataFloat rxfloatbuffer[EFFECT_BUF_LEN / 2];
float buffer_output[2][EFFECT_BUF_LEN];
float buffer_input[2][EFFECT_BUF_LEN];
float* buffer_input_ptr[2] = {buffer_input[0], buffer_input[1]};
float* buffer_output_ptr[2] = {buffer_output[0], buffer_output[1]};
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
arm_linear_interp_instance_f32 channel_a;
arm_linear_interp_instance_f32 channel_b;

float channela_y[21] = {-0.167560042983267,-0.164025163142684,-0.161081341613373,-0.1581299121418,-0.155070428348124,-0.151567098992156,-0.147238873839992,-0.140027808357011,-0.124302137029194,-0.0920389285159472,-0.0401939231670865,0.0229546619907531,0.0790180810327095,0.115927986026895,0.136076263194191,0.149169450966101,0.159450399835154,0.16858260358024,0.176944662952015,0.184016831341936,0.193011872253637}; 
float channelb_y[21] = {-0.364415394102135,-0.343279098400792,-0.325914785320255,-0.309461387648919,-0.29101180324297,-0.268653413826881,-0.23705456393864,-0.190454015492191,-0.130630740764401,-0.0651564645099696,-0.000535625153878212,0.0641378054282629,0.129741738878902,0.189737863498686,0.236520373839183,0.268282074158697,0.290889331383481,0.310190052697053,0.328545148391925,0.346881716916853,0.366415718641209};
static inline void effect_process(int index){
	
	HAL_GPIO_WritePin(LED1_GPIO_Port , LED1_Pin, 0);
	uint32_t rxfloatbuffer_idx = index;
	
	//pre_ad_calculation(&rxdmabuffer[index], &rxfloatbuffer[rxfloatbuffer_idx]);
	rxfloatbuffer[rxfloatbuffer_idx].right_audio = (rxdmabuffer[index].left_audio - 7475)/ 32768.0;
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
			rxfloatbuffer[rxfloatbuffer_idx].right_audio *= g_effect_controller.od_drive;
			rxfloatbuffer[rxfloatbuffer_idx].right_audio = arm_linear_interp_f32(&channel_a, rxfloatbuffer[rxfloatbuffer_idx].right_audio);
			rxfloatbuffer[rxfloatbuffer_idx].right_audio *= g_effect_controller.od_level;
			rxfloatbuffer[rxfloatbuffer_idx].right_audio = odlpf.calculate(&odlpf, rxfloatbuffer[rxfloatbuffer_idx].right_audio);
		}
		/*od code end*/
		
		if (g_effect_controller.reberb_switch){
					int tremolotype = g_effect_controller.tremolo_type;
					//rxfloatbuffer[rxfloatbuffer_idx].right_audio = doreverb_new(rxfloatbuffer[rxfloatbuffer_idx].right_audio);
					rxfloatbuffer[rxfloatbuffer_idx].right_audio *= update_tremolo(g_effect_controller.tremolo_rate, 
					g_effect_controller.tremolo_depth, 
					(TremoloType)tremolotype);
		}
	}
	HAL_GPIO_WritePin(LED1_GPIO_Port , LED1_Pin, 1);
}

void HAL_I2SEx_TxRxHalfCpltCallback(I2S_HandleTypeDef *hi2s){
	HAL_GPIO_WritePin(LED0_GPIO_Port , LED0_Pin, 0);
	//return;
	int i = 0;
	for (i = 0; i < EFFECT_BUF_LEN / 2; i++){
		effect_process(i);
	}

	for (i = 0; i < EFFECT_BUF_LEN / 2; i++){
		rxfloatbuffer[i].right_audio = rxfloatbuffer[i].right_audio * 32768;
		txdmabuffer[i].right_audio = rxfloatbuffer[i].right_audio > 32767? 32767 : (rxfloatbuffer[i].right_audio < -32767 ? -32767 : rxfloatbuffer[i].right_audio);
		txdmabuffer[i].left_audio = txdmabuffer[i].right_audio;
	}
	HAL_GPIO_WritePin(LED0_GPIO_Port , LED0_Pin, 1);
	//memcpy(txdmabuffer, rxdmabuffer, EFFECT_BUF_LEN * 2);
	//memset(txdmabuffer, 0, EFFECT_BUF_LEN * 2);
}

void HAL_I2SEx_TxRxCpltCallback(I2S_HandleTypeDef *hi2s){
	//return;
	HAL_GPIO_WritePin(LED0_GPIO_Port , LED0_Pin, 0);
	int i = 0;
	for (i = EFFECT_BUF_LEN / 2; i < EFFECT_BUF_LEN; i++){
		effect_process(i);
	}
	for (i = EFFECT_BUF_LEN / 2; i < EFFECT_BUF_LEN; i++){
		rxfloatbuffer[i].right_audio = rxfloatbuffer[i].right_audio * 32768;
		txdmabuffer[i].right_audio = rxfloatbuffer[i].right_audio > 32767? 32767 : (rxfloatbuffer[i].right_audio < -32767 ? -32767 : rxfloatbuffer[i].right_audio);
		txdmabuffer[i].left_audio = txdmabuffer[i].right_audio;
	}
	HAL_GPIO_WritePin(LED0_GPIO_Port , LED0_Pin, 1);
	//memcpy(&txdmabuffer[EFFECT_BUF_LEN / 2], &rxdmabuffer[EFFECT_BUF_LEN / 2], EFFECT_BUF_LEN * 2);
	//memcpy(&txdmabuffer[EFFECT_BUF_LEN / 2], 0, EFFECT_BUF_LEN * 2);
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
	ret = HAL_I2SEx_TransmitReceive_DMA(&hi2s1, (uint16_t*)txdmabuffer, (uint16_t*)rxdmabuffer, EFFECT_BUF_LEN *2);
	channel_a.nValues = 20;
	channel_a.x1 = -1.0f;
	channel_a.xSpacing = 0.1f;
	channel_a.pYData = channela_y;
	
	channel_b.nValues = 20;
	channel_b.x1 = -1.0f;
	channel_b.xSpacing = 0.1f;
	channel_b.pYData = channelb_y;
	reverb_init_new();
}
