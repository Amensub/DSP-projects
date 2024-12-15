/*
 * Copyright (c) 2018-2019 Analog Devices, Inc.  All rights reserved.
 *
 * These are the hooks for the audio processing functions.
 *
 */

#include <audio_processing/audio_effects_selector.h>
#include <math.h>

// Define your audio system parameters in this file
#include "common/audio_system_config.h"

// Support for simple multi-core data sharing
#include "common/multicore_shared_memory.h"

// Variables related to the audio framework that is currently selected (e.g. input and output buffers)
#include "audio_framework_selector.h"

// Includes all header files for effects and calls for effect selector
//#include "audio_processing/audio_effects_selector.h"

// Prototypes for this file
#include "callback_audio_processing.h"

#include "midi_setup.h"

/*
 *
 * Available Processing Power
 * --------------------------
 *
 * The two SHARC cores provide a hefty amount of audio processing power.  However, it is
 * important to ensure that any audio processing code can run and complete within one frame of audio.
 *
 * The total number of cycles available in the audio callback can be calculated as follows:
 *
 * total cycles = ( processor-clock-speed * audio-block-size ) / audio-sample-rate//
 *
 * For example, if the processor is running at 450MHz, the audio sampling rate is 48KHz and the
 * audio block size is set to 32 words, the total number of processor cycles available in each
 * callback is 300,000 cycles or 300,000/32 or 9,375 per sample of audio.
 *
 * Available Audio Buffers
 * -----------------------
 *
 * There are several sets of audio input and output buffers that correspond to the
 * various peripherals (e.g. audio codec, USB, S/PDIF, A2B).
 *
 * To send audio from USB out the DAC on the ADAU1761 one simply needs to copy data
 * from the USB buffers and copy them to the ADAU1761 buffer.
 *
 * for (i=0;i<AUDIO_BLOCK_SIZE;i++) {
 *   audiochannel_adau1761_0_left_out[i] = audiochannel_USB_0_left_in[i];
 *   audiochannel_adau1761_0_right_out[i] = audiochannel_USB_0_right_in[i];
 * }
 *
 * The framework ensures that audio is sample rate converted as needed (e.g S/PDIF)
 * and arrives where it needs to be on time using DMA.  It also manages the conversion
 * between fixed and floating point.
 *
 * Below is a list of the various input buffers and output buffers that are available.
 * Be sure that the corresponding peripheral has been enabled in audio_system_config.h
 *
 * Input Buffers
 * *************
 *
 *  Audio from the ADAU1761 ADCs
 *     audiochannel_adau1761_0_left_in[]
 *     audiochannel_adau1761_0_left_in[]
 *
 *  Audio from the S/PDIF receiver
 *     audiochannel_spdif_0_left_in[]
 *     audiochannel_spdif_0_right_in[]
 *
 *  Audio from USB (be sure to enable USB in audio_system_config.h)
 *     audiochannel_USB_0_left_in[]
 *     audiochannel_USB_0_right_in[]
 *
 *  Audio from A2B Bus
 *     audiochannel_a2b_0_left_in[]
 *     audiochannel_a2b_0_right_in[]
 *     audiochannel_a2b_1_left_in[]
 *     audiochannel_a2b_1_right_in[]
 *     audiochannel_a2b_2_left_in[]
 *     audiochannel_a2b_2_right_in[]
 *     audiochannel_a2b_3_left_in[]
 *     audiochannel_a2b_3_right_in[]
 *
 *
 *  Audio from Faust (be sure to enable Faust in audio_system_config.h and include the libraries)
 *
 *     audioChannel_faust_0_left_in[]
 *     audioChannel_faust_0_right_in[]
 *     audioChannel_faust_1_left_in[]
 *     audioChannel_faust_1_right_in[]
 *     audioChannel_faust_2_left_in[]
 *     audioChannel_faust_2_right_in[]
 *     audioChannel_faust_3_left_in[]
 *     audioChannel_faust_3_right_in[]
 *
 * Output Buffers
 * **************
 *  Audio to the ADAU1761 DACs
 *     audiochannel_adau1761_0_left_out[]
 *     audiochannel_adau1761_0_left_out[]
 *
 *  Audio to the S/PDIF transmitter
 *     audiochannel_spdif_0_left_out[]
 *     audiochannel_spdif_0_right_out[]
 *
 *  Audio to USB (be sure to enable USB in audio_system_config.h)
 *     audiochannel_USB_0_left_out[]
 *     audiochannel_USB_0_right_out[]
 *
 *  Audio to A2B Bus (be sure to enable A2B in audio_system_config.h)
 *     audiochannel_a2b_0_left_out[]
 *     audiochannel_a2b_0_right_out[]
 *     audiochannel_a2b_1_left_out[]
 *     audiochannel_a2b_1_right_out[]
 *     audiochannel_a2b_2_left_out[]
 *     audiochannel_a2b_2_right_out[]
 *     audiochannel_a2b_3_left_out[]
 *     audiochannel_a2b_3_right_out[]
 *
 *  Audio from Faust (be sure to enable Faust in audio_system_config.h)
 *
 *     audioChannel_faust_0_left_out[]
 *     audioChannel_faust_0_right_out[]
 *     audioChannel_faust_1_left_out[]
 *     audioChannel_faust_1_right_out[]
 *     audioChannel_faust_2_left_out[]
 *     audioChannel_faust_2_right_out[]
 *     audioChannel_faust_3_left_out[]
 *     audioChannel_faust_3_right_out[]
 *
 *  Note: Faust processing occurs before the audio callback so any data
 *  copied into Faust's input buffers will be available the next time
 *  the callback is called.  Similarly, Faust's output buffers contain
 *  audio that was processed before the callback.
 *
 *
 * There is also a set of buffers for sending audio to / from SHARC Core 2
 *
 *  Output to SHARC Core 2
 *     audiochannel_to_sharc_core2_0_left[]
 *     audiochannel_to_sharc_core2_0_right[]
 *     audiochannel_to_sharc_core2_1_left[]
 *     audiochannel_to_sharc_core2_1_right[]
 *     audiochannel_to_sharc_core2_2_left[]
 *     audiochannel_to_sharc_core2_2_right[]
 *     audiochannel_to_sharc_core2_3_left[]
 *     audiochannel_to_sharc_core2_3_right[]
 *
 *  Input from SHARC Core 2 (processed audio from SHARC Core 2)
 *     audiochannel_from_sharc_core2_0_left[]
 *     audiochannel_from_sharc_core2_0_right[]
 *     audiochannel_from_sharc_core2_1_left[]
 *     audiochannel_from_sharc_core2_1_right[]
 *     audiochannel_from_sharc_core2_2_left[]
 *     audiochannel_from_sharc_core2_2_right[]
 *     audiochannel_from_sharc_core2_3_left[]
 *     audiochannel_from_sharc_core2_3_right[]
 *
 * Finally, there is a set of aliased buffers that sends audio to the
 * right place.  On SHARC 1, the In[] buffers are received from the ADC
 * and the Out[] buffers are sent to either SHARC 2 (when in dual core more)
 * or to the DACs (when in single core mode).  The In[] buffers on SHARC core
 * 2 are received from SHARC core 1 and the Out[] buffers are sent to the DACs
 * (via SHARC core 1).
 *
 *     audiochannel_0_left_in[]
 *     audiochannel_0_right_in[]
 *
 *     audiochannel_1_left_out[]
 *     audiochannel_1_right_out[]
 *     audiochannel_2_left_out[]
 *     audiochannel_2_right_out[]
 *     audiochannel_3_left_out[]
 *     audiochannel_3_right_out[]
 *
 *     When the automotive board is being used, there are 16 channels of aliased
 *     buffers, not 8.  So they go up to audiochannel_7_left_in / audiochannel_7_right_in
 *     and audiochannel_7_left_out / audiochannel_7_right_out
 *
 * See the .c/.h file for the corresponding audio framework in the Audio_Frameworks
 * directory to see the buffers that are available for other frameworks (like the
 * 16 channel automotive framework).
 *
 */

/*
 * Place any initialization code here for the audio processing
 */
Keyboard keys[8];
float tempAudio;
int type = 0;
float f0; // Fundamental freq
float fc; // Carrier freq
float fm; // Modulator freq
float I0 = 100; // Modulator Amplitude Constant
float A;
float I;
float Tau;
float attackSlope;
float decaySlope;
float attackTime;
float decayTime;
float sustainLevel;
float Fs = AUDIO_SAMPLE_RATE;


void processaudio_setup(void) {

	for(int i = 0; i < 8; i++)
		{
			keys[i].reset();
		}

}

/*
 * This callback is called every time we have a new audio buffer that is ready
 * for processing.  It's currently configured for in-place processing so if no
 * processing is done to the audio, it is passed through unaffected.
 *
 * See the header file for the framework you have selected in the Audio_Frameworks
 * directory for a list of the input and output buffers that are available based on
 * the framework and hardware.
 *
 * The two SHARC cores provide a hefty amount of audio processing power. However, it is important
 * to ensure that any audio processing code can run and complete within one frame of audio.
 *
 * The total number of cycles available in the audio callback can be calculated as follows:
 * total cycles = ( processor-clock-speed * audio-block-size ) / audio-sample-rate
 *
 * For example, if the processor is running at 450MHz, the audio sampling rate is 48KHz and the audio
 * block size is set to 32 words, the total number of processor cycles available in each callback
 * is 300,000 cycles or 300,000/32 or 9,375 per sample of audio
 */

// When debugging audio algorithms, helpful to comment out this pragma for more linear single stepping.
#pragma optimize_for_speed
void processaudio_callback(void) {

//	if (true) {
//
//		// Copy incoming audio buffers to the effects input buffers
//		copy_buffer(audiochannel_0_left_in, audio_effects_left_in,
//				AUDIO_BLOCK_SIZE);
//		copy_buffer(audiochannel_0_right_in, audio_effects_right_in,
//				AUDIO_BLOCK_SIZE);
//
//		// Process audio effects
//		audio_effects_process_audio_core1();
//
//		// Copy processed audio back to input buffers
//		copy_buffer(audio_effects_left_out, audiochannel_0_left_in,
//				AUDIO_BLOCK_SIZE);
//		copy_buffer(audio_effects_right_out, audiochannel_0_right_in,
//				AUDIO_BLOCK_SIZE);
//
//	}

	// Otherwise, perform our C-based block processing here!
	for (int i = 0; i < AUDIO_BLOCK_SIZE; i++) {

		tempAudio = 0;

		if(type == 0) // Harmonic Bells
		{
			for (int j = 0; j < 8; j++)
			{
				f0 = keys[j].freq;
				fc = f0;
				fm = 2 * f0;
				I0 = multicore_data->audioproj_fin_pot_hadc0 * 9 + 1;
				Tau = multicore_data->audioproj_fin_pot_hadc1 * 5.9 + 0.1;
				A = exp(-keys[j].t/ Tau);
				I = I0 * A;
				tempAudio += A * sin(2*PI*fc*keys[j].t + I * sin(2*PI*fm*keys[j].t));

				keys[j].t += 1.0 / Fs;
			}
		}

		if(type == 1) // Inharmonic Bells
		{
			for (int j = 0; j < 8; j++)
			{
				f0 = keys[j].freq;
				fc = 5*f0;
				fm = 7 * f0;
				I0 = multicore_data->audioproj_fin_pot_hadc0 * 9 + 1;
				Tau = multicore_data->audioproj_fin_pot_hadc1 * 5.9 + 0.1;
				A = exp(-keys[j].t / Tau);
				I = I0 * A;
				tempAudio += A * sin(2*PI*fc*keys[j].t + I * sin(2*PI*fm*keys[j].t));

				keys[j].t+= 1.0 / Fs;
			}
		}

		if(type == 2) // Brass
		{
			for (int j = 0; j < 8; j++)
			{
				f0 = keys[j].freq;
				fc = f0;
				fm = f0;
				attackTime = multicore_data->audioproj_fin_pot_hadc0 * 0.45 + 0.05;
				decayTime = multicore_data->audioproj_fin_pot_hadc1 * 0.45 + 0.05;
				sustainLevel = multicore_data->audioproj_fin_pot_hadc2;
				attackSlope = 1 / (Fs * attackTime);
				decaySlope = - (1 - sustainLevel) / (Fs * decayTime);

				if(keys[j].t <= attackTime)
				{
					A = attackSlope * (keys[j].t * Fs);
				}

				else if(keys[j].t <= (attackTime + decayTime))
				{
					A = 1 + decaySlope * (keys[j].t * Fs - attackTime * Fs);
				}
				else
				{
					A = sustainLevel;
				}

				I = 5 * A;

				tempAudio += A * sin(2 * PI * fc * keys[j].t + I * sin(2 * PI * fm * keys[j].t));

				keys[j].t+= 1.0 / Fs;
			}
		}

		if(type == 3) // Basson
		{
			for (int j = 0; j < 8; j++)
			{
				f0 = keys[j].freq;
				attackTime = multicore_data->audioproj_fin_pot_hadc0 * 0.45 + 0.05;
				decayTime = multicore_data->audioproj_fin_pot_hadc1 * 0.45 + 0.05;
				sustainLevel = multicore_data->audioproj_fin_pot_hadc2;
				fc = 5 * f0;
				fm = f0;

				if(keys[j].t <= attackTime)
				{
					A = exp(0.683 * (keys[j].t) / attackTime) - 1;
				}
				else
				{
					A = sustainLevel;
				}
				I = 1.5 * A;
				tempAudio += A * sin(2 * PI * fc * keys[j].t + I * sin(2 * PI * fm * keys[j].t));

				keys[j].t+= 1.0 / Fs;
			}
		}

		if(type == 4) // Clarinet
		{
			for (int j = 0; j < 8; j++)
			{
				f0 = keys[j].freq;
				attackTime = multicore_data->audioproj_fin_pot_hadc0 * 0.45 + 0.05;
				decayTime = multicore_data->audioproj_fin_pot_hadc1 * 0.45 + 0.05;
				sustainLevel = multicore_data->audioproj_fin_pot_hadc2;
				fc = 3 * f0;
				fm = 2 * f0;

				if(keys[j].t <= attackTime)
				{
					A = exp(0.683 * (keys[j].t) / attackTime) - 1;
				}
				else
				{
					A = sustainLevel;
				}
				I = 4 - 2 * A;
				tempAudio += A * sin(2 * PI * fc * keys[j].t + I * sin(2 * PI * fm * keys[j].t));
				keys[j].t+= 1.0 / Fs;
			}
		}

		tempAudio /= 8;


		audiochannel_0_left_out[i] = tempAudio;
		audiochannel_0_right_out[i] = tempAudio;



		/* Below are some additional examples of how to receive audio from the various input buffers

		 // Example: Pass audio just from 1/8" (or 1/4" on Audio Project Fin) inputs to outputs
		 audioChannel_0_left_out[i] = audioChannel_0_left_in[i];
		 audioChannel_0_right_out[i] = AudioChannel_0_right_in[i];

		 // Example: mix audio in from 1/8" jacks and A2B input
		 audiochannel_0_left_out[i] = audiochannel_0_left_in[i] + audiochannel_a2b_0_left_in[i];
		 audiochannel_0_right_out[i] = audiochannel_0_right_in[i] + audiochannel_a2b_0_right_in[i];

		 // Example: receive audio from S/PDIF inputs and analog inputs
		 audiochannel_0_left_out[i] = audiochannel_0_left_in[i] + audiochannel_spdif_0_left_in[i];
		 audiochannel_0_right_out[i] = audiochannel_0_right_in[i] + audiochannel_spdif_0_right_in[i];

		 */

		/* You can also write directly to the various output buffers to explicitly route
		 * audio to different peripherals (ADAU1761, S/PDIF, A2B, etc.).  If you're using both
		 * cores to process audio (configured in common/audio_system_config.h), write your
		 * processed audio data to the audiochannel_N_left_out/audiochannel_N_right_out buffers
		 * and direct the output to the second core.  The function below, processAudio_OutputRouting(),
		 * is then used to route audio returning from the second core to various peripherals.
		 *
		 * However, if you're only using a single core in the audio processing path, you can redirect audio to
		 * specific peripherals by writing to the corresponding output buffers as shown in the
		 * examples below.  When using just one core for processing, audio written to the
		 * audiochannel_0_left_out/audiochannel_0_right_out buffers will get sent to the ADAU1761.

		 // Example: Send audio in from ADAU1761 to the A2B bus (be sure to enable A2B in audio_system_config.h)
		 audiochannel_a2b_0_left_out[i] = audiochannel_0_left_in[i];
		 audiochannel_a2b_0_right_out[i] = audiochannel_0_right_in[i];

		 // Example: Send audio from ADAU1761 to the SPDIF transmitter
		 audiochannel_spdif_0_left_out[i]  = audiochannel_adau1761_0_left_in[i];
		 audiochannel_spdif_0_right_out[i] = audiochannel_adau1761_0_right_in[i];

		 // Example: Send first stereo pair from A2B bus to ADAU1761 audio out
		 audiochannel_0_left_out[i] = audiochannel_a2b_0_left_in[i];
		 audiochannel_0_right_out[i] = audiochannel_a2b_0_right_in[i];
		 */
   		// If we're using just one core and A2B is enabled, copy the output buffer to A2B bus as well
#if (!USE_BOTH_CORES_TO_PROCESS_AUDIO) && (ENABLE_A2B)
			audiochannel_a2b_0_left_out[i] = audiochannel_0_left_out[i];
			audiochannel_a2b_0_right_out[i] = audiochannel_0_right_out[i];
#endif

		// If we're using Faust, copy audio into the flow
#if (USE_FAUST_ALGORITHM_CORE1)

		// Copy 8 channel audio from Faust to output buffers
		audiochannel_0_left_out[i] = audioChannel_faust_0_left_out[i];
		audiochannel_0_right_out[i] = audioChannel_faust_0_right_out[i];
		audiochannel_1_left_out[i] = audioChannel_faust_1_left_out[i];
		audiochannel_1_right_out[i] = audioChannel_faust_1_right_out[i];
		audiochannel_2_left_out[i] = audioChannel_faust_2_left_out[i];
		audiochannel_2_right_out[i] = audioChannel_faust_2_right_out[i];
		audiochannel_3_left_out[i] = audioChannel_faust_3_left_out[i];
		audiochannel_3_right_out[i] = audioChannel_faust_3_right_out[i];

		// Route audio to Faust for next block
		audioChannel_faust_0_left_in[i] = audiochannel_0_left_in[i] + audiochannel_spdif_0_left_in[i];
		audioChannel_faust_0_right_in[i] = audiochannel_0_right_in[i] + audiochannel_spdif_0_right_in[i];

#endif
	}

}

#if (USE_BOTH_CORES_TO_PROCESS_AUDIO)

/*
 * When using a dual core configuration, SHARC Core 1 is responsible for routing the
 * processed audio from SHARC Core 2 to the various output buffers for the
 * devices connected to the SC589.  For example, in a dual core framework, SHARC Core 1
 * may pass 8 channels of audio to Core 2, and then receive 8 channels of processed audio
 * back from Core 2.  It is this routine where we route these channels to the ADAU1761,
 * the A2B bus, SPDIF, etc.
 */
#pragma optimize_for_speed
void processaudio_output_routing(void) {

	static float t = 0;

	for (int i = 0; i < AUDIO_BLOCK_SIZE; i++) {

		// If automotive board is attached, send all 16 channels from core 2 to the DACs
#if defined(AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN) && AUDIO_FRAMEWORK_16CH_SAM_AND_AUTOMOTIVE_FIN

		// Copy 16 channels from Core 2 to the DACs on the automotive board
		audiochannel_automotive_0_left_out[i] = audiochannel_from_sharc_core2_0_left[i];
		audiochannel_automotive_0_right_out[i] = audiochannel_from_sharc_core2_0_right[i];
		audiochannel_automotive_1_left_out[i] = audiochannel_from_sharc_core2_1_left[i];
		audiochannel_automotive_1_right_out[i] = audiochannel_from_sharc_core2_1_right[i];
		audiochannel_automotive_2_left_out[i] = audiochannel_from_sharc_core2_2_left[i];
		audiochannel_automotive_2_right_out[i] = audiochannel_from_sharc_core2_2_right[i];
		audiochannel_automotive_3_left_out[i] = audiochannel_from_sharc_core2_3_left[i];
		audiochannel_automotive_3_right_out[i] = audiochannel_from_sharc_core2_3_right[i];
		audiochannel_automotive_4_left_out[i] = audiochannel_from_sharc_core2_4_left[i];
		audiochannel_automotive_4_right_out[i] = audiochannel_from_sharc_core2_4_right[i];
		audiochannel_automotive_5_left_out[i] = audiochannel_from_sharc_core2_5_left[i];
		audiochannel_automotive_5_right_out[i] = audiochannel_from_sharc_core2_5_right[i];
		audiochannel_automotive_6_left_out[i] = audiochannel_from_sharc_core2_6_left[i];
		audiochannel_automotive_6_right_out[i] = audiochannel_from_sharc_core2_6_right[i];
		audiochannel_automotive_7_left_out[i] = audiochannel_from_sharc_core2_7_left[i];
		audiochannel_automotive_7_right_out[i] = audiochannel_from_sharc_core2_7_right[i];

#else

		// If A2B enabled, route audio down the A2B bus
#if (ENABLE_A2B)

		// Send all 8 channels from core 2 down the A2B bus
		audiochannel_a2b_0_left_out[i] = audiochannel_from_sharc_core2_0_left[i];
		audiochannel_a2b_0_right_out[i] = audiochannel_from_sharc_core2_0_right[i];
		audiochannel_a2b_1_left_out[i] = audiochannel_from_sharc_core2_1_left[i];
		audiochannel_a2b_1_right_out[i] = audiochannel_from_sharc_core2_1_right[i];
		audiochannel_a2b_2_left_out[i] = audiochannel_from_sharc_core2_2_left[i];
		audiochannel_a2b_2_right_out[i] = audiochannel_from_sharc_core2_2_right[i];
		audiochannel_a2b_3_left_out[i] = audiochannel_from_sharc_core2_3_left[i];
		audiochannel_a2b_3_right_out[i] = audiochannel_from_sharc_core2_3_right[i];

#endif

		// Send Audio from SHARC Core 2 out to the DACs (1/8" audio out connector)
		audiochannel_adau1761_0_left_out[i] =
				audiochannel_from_sharc_core2_0_left[i];
		audiochannel_adau1761_0_right_out[i] =
				audiochannel_from_sharc_core2_0_right[i];

		// Send audio from SHARC Core 2 to the SPDIF transmitter as well
		audiochannel_spdif_0_left_out[i] =
				audiochannel_from_sharc_core2_0_left[i];
		audiochannel_spdif_0_right_out[i] =
				audiochannel_from_sharc_core2_0_right[i];
#endif
	}
}
#endif

/*
 * This loop function is like a thread with a low priority.  This is good place to process
 * large FFTs in the background without interrupting the audio processing callback.
 */
void processaudio_background_loop(void) {

	if(multicore_data->audioproj_fin_sw_1_core1_pressed == true)
	{
		multicore_data->audioproj_fin_sw_1_core1_pressed = false;
		type = (type+1)%5;
	}

//	if(multicore_data->audioproj_fin_sw_2_core1_pressed == true)
//	{
//		multicore_data->audioproj_fin_sw_2_core1_pressed = false;
//		for(int i = 0; i < 8; i++)
//		{
//			keys[i].reset();
//		}
//	}
}

/*
 * This function is called if the code in the audio processing callback takes too long
 * to complete (essentially exceeding the available computational resources of this core).
 */
void processaudio_mips_overflow(void) {
}
