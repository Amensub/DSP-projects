/*
 * midi_setup.h
 *
 *  Created on: Oct 31, 2024
 *      Author: xchen117
 */
# include <math.h>

#ifndef MIDI_SETUP_H_
#define MIDI_SETUP_H_


class Keyboard
{
	public:
	int midiNote;
	float freq;
	bool playing;
	float t; // in seconds

	void reset(void)
	{
		freq = 0;
		t = 0;
		playing = false;
	}

	void setFreq(void)
	{
		freq = powf(2.0, (midiNote - 69.0)/12.0) * 440;
		t = 0;
	}

};


#endif /* MIDI_SETUP_H_ */
