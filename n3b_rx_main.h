#ifndef __N3B_H__
#define __N3B_H__
/* 
 * N3B.h
 *
 * Created: May 2024
 * Author: Baris DINC OH2UDS / TA7W
 *
 * This file contains the system-wide definitions and platform services.
 *
 */

// #include "hardware/i2c.h"

/* Set this to 1 when FFT engine must be used */

#define DSP_FFT					1


/* GPIO (pin) assignments */

#define GP_ENC_A				2											// Pin  4: Encoder channel A
#define GP_ENC_B				3											// Pin  5: Encoder channel B
#define GP_AUX_0				6											// Pin  9: Enter, Confirm
#define GP_AUX_1				7											// Pin 10: Escape, Cancel
#define GP_AUX_2				8											// Pin 11: Left move
#define GP_AUX_3				9											// Pin 12: Right move
#define GP_PTT					15											// Pin 20: PTT line (low is active)
#define GP_LNA                  20
#define DAC_Q					16											// Pin 26: PWM DAC Q channel
#define DAC_I					21											// Pin 27: PWM DAC I channel
#define DAC_A					22											// Pin 29: PWM DAC Audio channel
#define ADC_Q					26											// Pin 31: ADC 0
#define ADC_I					27											// Pin 32: ADC 1
#define ADC_A					28											// Pin 34: ADC 2



/* Timer values */

#define LED_MS					1000										// LED flashing, half cycle duration
#define LOOP_MS					100											// Core 0 main loop timer (see also uSDR.c)


int16_t* get_fft_buffer_address(void);
bool is_fft_completed();



#endif
