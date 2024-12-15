/*
 * biquad.h
 *
 *  Created on: Sep 26, 2024
 *      Author: xchen117
 */

#include <stdlib.h>

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

// Support for simple multi-core data sharing
#include "common/multicore_shared_memory.h"


class biquad{

	private :
	float x2, x1, y1, y2;

	public :
	float b0, b1, b2, a0, a1, a2;

	void reset(void){
		x2 = 0;
		x1 = 0;
		y1 = 0;
		y2 = 0;
	}

	void set(float f0, float Q){
		float w0 =2*PI*f0/AUDIO_SAMPLE_RATE;
		float alpha = sin(w0)/(2*Q);
		b0 = alpha*Q;
		b1 = 0.0;
		b2 = -alpha*Q;
		a0 = 1 + alpha;
		a1 = -2*cos(w0);
		a2 = 1 - alpha;
	}

	float doBiquad(float input){
		float output = (b0/a0)*input + (b1/a0)*x1 + (b2/a0)*x2 - (a1/a0)*y1 - (a2/a0)*y2;
		y2 = y1;
		y1 = output;
		x2= x1;
		x1 = input;

		return output;
	}

};

