#ifndef LOWPASS_FILTER
#define LOWPASS_FILTER

typedef struct lowpassfilter_s{
	float xk_1;
	float xk_2;
	float yk_1;
	float yk_2;
	float sample_rate;
	
	float a0;
	float a1;
	float a2;
	float b0;
	float b1;
	float b2;
	
	void (*update_para)(struct lowpassfilter_s* filter, float cutfreq);
	float (*calculate)(struct lowpassfilter_s* filter, float input);
}LowPassFilter;

extern LowPassFilter odlpf;

#endif