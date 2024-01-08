#ifndef EFFECT_CHAIN_H
#define EFFECT_CHAIN_H
#include "i2c.h"
#include "arm_math.h"
#include "core_cm4.h"
typedef struct AudioData_s{
	int16_t left_audio;
	int16_t right_audio;
}AudioData;

typedef struct Header_s{
	uint32_t header_id;
	uint32_t datatype;
	uint32_t datalen;
	uint8_t data[2048 * 4];
}Header;

typedef struct FunctionMode_e{
	uint8_t tunner_state;
	uint8_t boost_state;
	uint8_t od_state;
	uint8_t reverb_state;
	float std_freq;
	float freq;
	uint8_t  string_name;
}FunctionMode;

extern volatile FunctionMode function_mode;

typedef struct AudioDataFloat_s{
	float left_audio;
	float right_audio;
}AudioDataFloat;

typedef struct Effect_Controller_s{
	uint8_t boost_switch;
	uint8_t od_switch;
	uint8_t reberb_switch;
	uint8_t tune_switch;
	float boost_level;
	float od_level;
	float od_drive;
	float od_tone;
	float reverb_time;
	float reverb_mix;
	float delay_time;
	float delay_mix;
}EffectController;

extern volatile EffectController g_effect_controller;
void init_input_output_confit(void);
void boost_pedal(AudioDataFloat* data, uint16_t tap, uint16_t level);
void tuner_function(float inputsig, uint16_t switch_tap);
float Do_Reverb(float* inSample);
void reverb_init();
void oled_handle();
void spi_lcd_handle();
void uninit_tuner();
#endif
