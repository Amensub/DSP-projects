/*
 * filters.h
 *
 *  Created on: Dec 5, 2024
 *      Author: xchen117
 */
#include <stdlib.h>
#include "common/multicore_shared_memory.h"
#include "common/audio_system_config.h"

#ifndef FILTERS_H_
#define FILTERS_H_

class combfilter
{
	private:
	float outdelay[5000];

	public:
	float g;
	int length;
	int idx;
	void reset()
	{
		idx = 0;
		for(int i = 0; i < 5000; i++)
		{
			outdelay[i] = 0;
		}
	}

	float docomb(float input)
	{
		float output = input + g * outdelay[idx];
		outdelay[idx] = output;
		idx = (idx+1) % length;
		return output;
	}

};

class allpassfilter
{
	private:
	float outdelay[2252];
	float indelay[2252];

	public:
	float g;
	int length;
	int idx;
	void reset()
	{
		idx = 0;
		for(int i = 0; i < 2252; i++)
		{
			outdelay[i] = 0;
		}
	}

	float doallpass(float input)
	{
		float output = -g * input + indelay[idx] + g * outdelay[idx];
		outdelay[idx] = output;
		indelay[idx] = input;
		idx = (idx+1) % length;
		return output;
	}

};

#endif /* FILTERS_H_ */
