#include "tremolo.h"
#include <math.h>
 
Tremolo tremolo = {1, 0.25, 0, 2.083e-5};

float update_tremolo(float rate, float depth, TremoloType type){
	
	tremolo.rate = tremolo.rate * 0.9 + 0.1 * rate;
	tremolo.depth = tremolo.depth * 0.9 + 0.1 * depth;
	tremolo.local_time += tremolo.step_time * rate;
	if (tremolo.local_time >= 6.28318){
		//tremolo.local_time -= 6.28318;
	}
	float sinout = sin(tremolo.local_time  * 6.28318);
	float cosout = sin(tremolo.local_time  * 6.28318);
	static float square = 0;
	if (cosout >= 0){
		square = 0.999 * square + 0.001 * 1.0f;
	}else{
		square = 0.999 * square + 0.001 * -1.0f;
	}
	float triangle = asin(cosout) * 0.66666f;
	float outval = 0;
	switch (type){
		case SINE:
			outval = sinout;
			break;
		case SQURE:
			outval = square;
			break;;
		case SAW:
			outval = triangle;
			break; ;
	}
	
	return (outval * depth) + (1 - depth);
}