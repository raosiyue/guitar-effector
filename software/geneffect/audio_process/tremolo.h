#ifndef TREMOLO_H
#define TREMOLO_H

typedef struct Tremolo_s{
	float rate; //hz 0-5
	float depth; // 0-0.5
	float local_time;
	float step_time;
}Tremolo;

typedef enum TremoloType_e{
	SINE,
	SAW,
	SQURE
}TremoloType;

float update_tremolo(float rate, float depth, TremoloType type);

#endif