/*
 * n3b_rx_main.c
 *
 * Created: May 2024
 * Author: Baris Dinc OH2UDS/TA7W
 * 
 * The main loop of the application.
 */

/*
 * The source code is re-written using the Arduino (Klaus) and C (Arjante) base code to fully understand and to be able to extend
 * the capabilities. 
 *  Final goal of the project is to have a VHF.UHF/SHF transceiver SDR base for IQ modulator/demodulator based transceiver designs.
 *  Initial project is decided to be a QO-100 full duplex tranbsceiver
 * 
 *  TODO:
 *  - How to prepare the build environment
 *  - Documentation describing the basic princibles of the hard ware and the firmware
 *  - Future plans
*/

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/sem.h"
#include "n3b_rx_main.h"
#include "hardware_init.h"
#include "debug_interface.h"
#include "display_ili9341.h"
#include "adf4360.h"
#include "user_interface.h"
#include "display_ili9341.h"
#include <time.h>

/* 
 * LED TIMER definition and callback routine
 */
struct repeating_timer led_timer;
bool led_callback(struct repeating_timer *t) 
{
	static bool led_state;
	
	gpio_put(PICO_DEFAULT_LED_PIN, led_state);
	led_state = (led_state?false:true);
	return true;
}


/*
 * Scheduler callback function.
 * This executes every LOOP_MS.
 */
semaphore_t loop_sem;
struct repeating_timer loop_timer;
bool loop_callback(struct repeating_timer *t)
{
	sem_release(&loop_sem);
	return(true);
}


int main()
{
	/* 
	 * Main loop rnning on Core 0
	 * Optional: increase core voltage (normally 1.1V)
	 * Optional: overclock the CPU to 250MHz  (normally 125MHz)
	 * Note that clk_peri (e.g. SPI/I2C) is derived from the SYS PLL
	 * Note that clk_adc sampling clock is derived from the 48MHz USB PLL.
	 */
	//vreg_set_voltage(VREG_VOLTAGE_1_25); sleep_ms(10);
	//set_sys_clock_khz(250000, false); sleep_ms(10);

	stdio_init_all();
	sleep_ms(1000);

	printf("Starting...\n");

	//Initialize hardware peripherals
	hardware_init_pins();
	printf("Hardware initialized.\n");

	// Set LED on/blinking
	add_repeating_timer_ms(-LED_MS, led_callback, NULL, &led_timer);
	printf("LED blinbking initialized.\n");

	//Start debug interface
	debug_interface_init();																// Monitor shell on stdio
	//printf("Debug interface initialized.\n");

	display_ili9341_init();											// TFT LCD initialization
	

	adf4360_init();																// VFO control unit
//	dsp_init();																// Signal processing unit

	user_interface_init0();																// HMI user inputs
	user_interface_init();
	
	display_ili9341_setup0();
	display_ili9341_setup();

	/* A simple round-robin scheduler */
	sem_init(&loop_sem, 1, 1) ;	
	add_repeating_timer_ms(-LOOP_MS, loop_callback, NULL, &loop_timer);
	while (1) 										
	{
//		sem_acquire_blocking(&loop_sem);									// Wait until timer callback releases sem
		user_interface_evaluate();														// Refresh HMI (and VFO, BPF, etc)
		debug_interface_evaluate();														// Check monitor input
	}

    return 0;
}



