#include "lowpass_filter.h"
#include "math.h"
void update_para_lpf(struct lowpassfilter_s* filter, float cutfreq){
	float fr = filter->sample_rate / cutfreq;
	float ohm = tan(3.14159265 / fr);
	float c = 1 + 2 *cos(3.1415926/4.0f) * ohm + ohm*ohm;
	filter->b0 = ohm*ohm / c;
	filter->b1 = 2 * filter->b0;
	filter->b2 = filter->b0;
	filter->a0 = 1;
	filter->a1 = 2 * (ohm* ohm - 1)/c;
	filter->a2 = (1 - 2*cos(3.14159/4)*ohm +ohm * ohm) /c;
}
float calculate_lpf(struct lowpassfilter_s* filter, float input){
	float outy = filter->b0 *input + filter->b1 * filter->xk_1 + filter->b2 * filter->xk_2 - filter->a1*filter->yk_1 - filter->a2 * filter->yk_2;
	filter->yk_2 = filter->yk_1;
	filter->yk_1 = outy;
	
	filter->xk_2 = filter->xk_1;
	filter->xk_1 = input;
	return outy;
}

LowPassFilter odlpf = {0, 0, 0, 0, 48000, 0, 0, 0, 0, 0, 0, update_para_lpf, calculate_lpf};