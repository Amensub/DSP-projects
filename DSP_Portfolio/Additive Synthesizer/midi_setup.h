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

	void reset(void)
	{
		freq = 0;
		playing = false;
	}

	void setFreq(void)
	{
		freq = powf(2.0, (midiNote - 69.0)/12.0) * 440;
	}

};


#endif /* MIDI_SETUP_H_ */
