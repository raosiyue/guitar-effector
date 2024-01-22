#include "effect_chain.h"
//#include "malloc.h"
#include "reverb.h"

void init_func(struct comb_filter_s* filter, uint16_t delaylenmax, float gain){
	filter->gainout = gain;
	filter->delaylen = delaylenmax;
	filter->delay_buffer = (float*)mymalloc(0, sizeof(float) *delaylenmax);
}
void update_para(struct comb_filter_s* filter, uint16_t delaycnt, float gain){
	filter->gainout = gain;
	filter->delaycnt = delaycnt;
}
float calculate_comb_filter(struct comb_filter_s* filter, float input){
	filter->current_ptr++;
	if (filter->current_ptr >= filter->delaylen){
		filter->current_ptr = 0;
	}
	int32_t delayptr = filter->current_ptr - filter->delaycnt;
	delayptr = ((delayptr) > 0 ? delayptr : delayptr + filter->delaylen);
	filter->delay_buffer[filter->current_ptr] = input + filter->delay_buffer[delayptr] * filter->gainout;
	return filter->delay_buffer[delayptr];
}

float calculate_ap_filter(struct comb_filter_s* filter, float input){
	filter->current_ptr++;
	if (filter->current_ptr >= filter->delaylen){
		filter->current_ptr = 0;
	}
	int32_t delayptr = filter->current_ptr - filter->delaycnt;
	delayptr = ((delayptr) > 0 ? delayptr : delayptr + filter->delaylen);
	float output = filter->delay_buffer[delayptr] + input * -filter->gainout;
	filter->delay_buffer[filter->current_ptr] = input + output* filter->gainout;
	return output;
}

CombFilter ap1 = {0.0, 0, 0, 0, 0, init_func, update_para, calculate_ap_filter};
CombFilter ap2 = {0.0, 0, 0, 0, 0, init_func, update_para, calculate_ap_filter};
CombFilter ap3 = {0.0, 0, 0, 0, 0, init_func, update_para, calculate_ap_filter}; 
CombFilter fbcf1 = {0.0, 0, 0, 0, 0, init_func, update_para, calculate_comb_filter};
CombFilter fbcf2 = {0.0, 0, 0, 0, 0, init_func, update_para, calculate_comb_filter}; 
CombFilter fbcf3 = {0.0, 0, 0, 0, 0, init_func, update_para, calculate_comb_filter}; 
CombFilter fbcf4 = {0.0, 0, 0, 0, 0, init_func, update_para, calculate_comb_filter};

#define ap1_len 250 * 4
#define ap2_len 84 * 4
#define ap3_len 1730 * 4
#define cf1_len 1730 * 4
#define cf2_len 1494 * 4
#define cf3_len 2022 * 4
#define cf4_len 2246 * 4

void reverb_init_new(){
	ap1.init_func(&ap1, ap1_len, 0.7);
	ap2.init_func(&ap2, ap2_len, 0.7);
	ap3.init_func(&ap3, ap3_len, 0.7);
	fbcf1.init_func(&fbcf1, cf1_len, 0.805);
	fbcf2.init_func(&fbcf2, cf2_len, 0.827);
	fbcf3.init_func(&fbcf3, cf3_len, 0.783);
	fbcf4.init_func(&fbcf4, cf4_len, 0.764);
}

float doreverb_new(float sample){
	ap1.update_para(&ap1, ap1_len, 0.7 *g_effect_controller.delay_mix);
	ap2.update_para(&ap2, ap2_len, 0.7 * g_effect_controller.delay_mix);
	ap3.update_para(&ap3, ap3_len, 0.7 * g_effect_controller.delay_mix);
	fbcf1.update_para(&fbcf1, cf1_len *g_effect_controller.reverb_time, -0.805 * g_effect_controller.delay_time);
	fbcf2.update_para(&fbcf2, cf2_len *g_effect_controller.reverb_time, -0.827 * g_effect_controller.delay_time);
	fbcf3.update_para(&fbcf3, cf3_len*g_effect_controller.reverb_time, -0.783 * g_effect_controller.delay_time);
	fbcf4.update_para(&fbcf4, cf4_len *g_effect_controller.reverb_time, -0.764 * g_effect_controller.delay_time);
	
	float newsample = (fbcf1.calculate(&fbcf1, sample) + fbcf2.calculate(&fbcf2, sample) 
											+ fbcf3.calculate(&fbcf3, sample) + fbcf4.calculate(&fbcf4, sample))/4.0f;
	newsample = ap1.calculate(&ap1, newsample);
	newsample = ap2.calculate(&ap2, newsample);
	newsample = ap3.calculate(&ap3, newsample);
	
	newsample = (1.0f - g_effect_controller.reverb_mix)* (sample) + g_effect_controller.reverb_mix * newsample;
	return newsample;
	
}

#define l_CB0 1730
#define l_CB1 1494
#define l_CB2 1011*2
#define l_CB3 1123*2
#define l_AP0 125*2
#define l_AP1 42*2
#define l_AP2 12*2

//define wet 0.0 <-> 1.0
float wet = 0.5f;
//define time delay 0.0 <-> 1.0 (max)
float g_reverb_time = 1.0f;

//define pointer limits = delay time
int cf0_lim, cf1_lim, cf2_lim, cf3_lim, ap0_lim, ap1_lim, ap2_lim;

//define buffer for comb- and allpassfilters
float* cfbuf0;
float* cfbuf1;
float* cfbuf2;
float* cfbuf3;
float* apbuf0;
float* apbuf1;
float* apbuf2;
//feedback defines as of Schroeder
float cf0_g = -0.805f, cf1_g=-0.827f, cf2_g=-0.783f, cf3_g=-0.764f;
float ap0_g = 0.7f, ap1_g = 0.7f, ap2_g = 0.7f;
//buffer-pointer
int cf0_p=0, cf1_p=0, cf2_p=0, cf3_p=0, ap0_p=0, ap1_p=0, ap2_p=0;

float Do_Comb0(float inSample) {
	float readback;
	readback = cfbuf0[cf0_p];
	cfbuf0[cf0_p] = readback*cf0_g + inSample;
	cf0_p++;
	if (cf0_p>=cf0_lim) cf0_p = 0;
	return readback;
}

float Do_Comb1(float inSample) {
	float readback;
	readback = cfbuf1[cf1_p];
	cfbuf1[cf1_p] = readback*cf1_g + inSample;
	cf1_p++;
	if (cf1_p>=cf1_lim) cf1_p = 0;
	return readback;
}
float Do_Comb2(float inSample) {
	float readback;
	readback = cfbuf2[cf2_p];
	cfbuf2[cf2_p] = readback*cf2_g + inSample;
	cf2_p++;
	if (cf2_p>=cf2_lim) cf2_p = 0;
	return readback;
}
float Do_Comb3(float inSample) {
	float readback;
	readback = cfbuf3[cf3_p];
	cfbuf3[cf3_p] = readback*cf3_g + inSample;
	cf3_p++;
	if (cf3_p>=cf3_lim) cf3_p = 0;
	return readback;
}

float Do_Allpass0(float inSample) {
	float readback;
	readback = apbuf0[ap0_p];
	readback += (-ap0_g) * inSample;
	apbuf0[ap0_p] = readback*ap0_g + inSample;
	ap0_p++;
	if (ap0_p >= ap0_lim) ap0_p=0;
	return readback;
}

float Do_Allpass1(float inSample) {
	float readback;
	readback = apbuf1[ap1_p];
	readback += (-ap1_g) * inSample;
	apbuf1[ap1_p] = readback*ap1_g + inSample;
	ap1_p++;
	if (ap1_p >= ap1_lim) ap1_p=0;
	return readback;
}
float Do_Allpass2(float inSample) {
	float readback;
	readback = apbuf2[ap2_p];
	readback += (-ap2_g) * inSample;
	apbuf2[ap2_p] = readback*ap2_g + inSample;
	ap2_p++;
	if (ap2_p >= ap2_lim) ap2_p=0;
	return readback;
}

float Do_Reverb(float* inSample) {
	
	cf0_lim = (int)(g_effect_controller.reverb_time*l_CB0);
	cf1_lim = (int)(g_effect_controller.reverb_time*l_CB1);
	cf2_lim = (int)(g_effect_controller.reverb_time*l_CB2);
	cf3_lim = (int)(g_effect_controller.reverb_time*l_CB3);
	ap0_lim = (int)(g_effect_controller.reverb_time*l_AP0);
	ap1_lim = (int)(g_effect_controller.reverb_time*l_AP1);
	ap2_lim = (int)(g_effect_controller.reverb_time*l_AP2);
	
	
	float newsample = (Do_Comb0(*inSample) + Do_Comb1(*inSample) + Do_Comb2(*inSample) + Do_Comb3(*inSample))/4.0f;
	newsample = Do_Allpass0(newsample);
	newsample = Do_Allpass1(newsample);
	newsample = Do_Allpass2(newsample);
	
	*inSample = (1.0f - g_effect_controller.reverb_mix)* (*inSample) + g_effect_controller.reverb_mix * newsample;
	return 0;
}

void reverb_init(){
	g_reverb_time = 1;
	cf0_lim = (int)(g_reverb_time*l_CB0);
	cf1_lim = (int)(g_reverb_time*l_CB1);
	cf2_lim = (int)(g_reverb_time*l_CB2);
	cf3_lim = (int)(g_reverb_time*l_CB3);
	ap0_lim = (int)(g_reverb_time*l_AP0);
	ap1_lim = (int)(g_reverb_time*l_AP1);
	ap2_lim = (int)(g_reverb_time*l_AP2);
	
	cfbuf0 = (float*)mymalloc(0, sizeof(float) *l_CB0);
	cfbuf1 = (float*)mymalloc(0, sizeof(float) *l_CB1);
	cfbuf2 = (float*)mymalloc(0, sizeof(float) *l_CB2);
	cfbuf3 = (float*)mymalloc(0, sizeof(float) *l_CB3);
	apbuf0 = (float*)mymalloc(0, sizeof(float) *l_AP0);
	apbuf1 = (float*)mymalloc(0, sizeof(float) *l_AP1);
	apbuf2 = (float*)mymalloc(0, sizeof(float) *l_AP2);	

}
