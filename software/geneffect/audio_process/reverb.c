#include "effect_chain.h"
//#include "malloc.h"
#define l_CB0 901*3
#define l_CB1 778*3
#define l_CB2 1011*3
#define l_CB3 1123*3
#define l_AP0 125*3
#define l_AP1 42*3
#define l_AP2 12*3

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
float cf0_g = 0.805f, cf1_g=0.827f, cf2_g=0.783f, cf3_g=0.764f;
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
	cf0_lim = (int)(g_reverb_time*l_CB0);
	cf1_lim = (int)(g_reverb_time*l_CB1);
	cf2_lim = (int)(g_reverb_time*l_CB2);
	cf3_lim = (int)(g_reverb_time*l_CB3);
	ap0_lim = (int)(g_reverb_time*l_AP0);
	ap1_lim = (int)(g_reverb_time*l_AP1);
	ap2_lim = (int)(g_reverb_time*l_AP2);
	
	cfbuf0 = (float*)mymalloc(1, sizeof(float) *l_CB0);
	cfbuf1 = (float*)mymalloc(1, sizeof(float) *l_CB1);
	cfbuf2 = (float*)mymalloc(1, sizeof(float) *l_CB2);
	cfbuf3 = (float*)mymalloc(1, sizeof(float) *l_CB3);
	apbuf0 = (float*)mymalloc(1, sizeof(float) *l_AP0);
	apbuf1 = (float*)mymalloc(1, sizeof(float) *l_AP1);
	apbuf2 = (float*)mymalloc(1, sizeof(float) *l_AP2);	

}
