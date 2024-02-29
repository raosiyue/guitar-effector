#ifndef __REVERB_H
#define __REVERB_H
#include "stdint.h"

typedef struct comb_filter_s{
	
	float gainout;
	float* delay_buffer;
	int32_t delaylen;
	int32_t current_ptr;
	int32_t delaycnt;
	void (*init_func)(struct comb_filter_s* filter, uint16_t delaylenmax, float gain);
	void (*update_para)(struct comb_filter_s* filter, uint16_t delaycnt, float gain);
	float (*calculate)(struct comb_filter_s* filter, float input);
}CombFilter;

void reverb_init_new();
float doreverb_new(float sample);

#endif












