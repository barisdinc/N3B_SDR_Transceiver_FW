/*
 * N3B.c
 *
 * Created: May 2024
 * Author: Baris DINC OH2UDS/TA7W
 * 
 * The main loop of the application.
 * This initializes the units that do the actual work, and then loops in the background. 
 * Other units are:
 * - dsp.c, containing all signal processing in RX and TX branches. This part runs on the second processor core.
 * - adf4306.c, containing all controls for setting up the adf4301 pll vco.
 * - hmi.c, contains all functions that handle user inputs
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/sem.h"
// #include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "pico/time.h"

#include "n3b_rx_main.h"
#include "hmi.h"
#include "dsp.h"
#include "monitor.h"
#include "adf4360.h"
#include "ili9341.h"

/*
 * Wrappers around i2c_write_blocking() and i2c_read_blocking()
 * The SDK functions return too soon, potentially causing overlapping calls
 */

// int i2c_put_data(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop)
// {
// 	int r = i2c_write_blocking(i2c, addr, src, len, nostop);
// 	sleep_us(I2C_LINGER_US);
// 	return(r);
// }

// int i2c_get_data(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop)
// {
// 	int r = i2c_read_blocking(i2c, addr, dst, len, nostop);
// 	sleep_us(I2C_LINGER_US);
// 	return(r);
// }


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
	stdio_init_all();
	sleep_ms(2000);
	ili9341_init();
	ili9341_set_rotation(3);
	ili9341_fill_screen(ILI9341_BLACK);
	// ili9341_draw_string(10, 10, "Hello, RP2040!", ILI9341_YELLOW, ILI9341_BLACK,3);


	/* 
	 * Main loop rnning on Core 0
	 * Optional: increase core voltage (normally 1.1V)
	 * Optional: overclock the CPU to 250MHz  (normally 125MHz)
	 * Note that clk_peri (e.g. I2C) is derived from the SYS PLL
	 * Note that clk_adc sampling clock is derived from the 48MHz USB PLL.
	 */
	//vreg_set_voltage(VREG_VOLTAGE_1_25); sleep_ms(10);
	//set_sys_clock_khz(250000, false); sleep_ms(10);
	
	/* 
	 * Initialize LED pin output 
	 */
	printf("STARTing\n");
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	gpio_put(PICO_DEFAULT_LED_PIN, true);									// Set LED on
	add_repeating_timer_ms(-LED_MS/10, led_callback, NULL, &led_timer);


	/* Initialize the SW units */
	mon_init();																// Monitor shell on stdio
	//si_init();																// VFO control unit
	printf("init 4360 ");
	adf4360_init();
	printf("OK");
	dsp_init();																// Signal processing unit
	hmi_init();																// HMI user inputs

// while (1)
{
	// printf(".");
	// vfo[0].flag = 1;
	// adf4360_evaluate(777000);	
	// sleep_ms(2000);
}

	/* A simple round-robin scheduler */
	sem_init(&loop_sem, 1, 1) ;	
	add_repeating_timer_ms(-LOOP_MS, loop_callback, NULL, &loop_timer);
	while (1) 										
	{
		sem_acquire_blocking(&loop_sem);									// Wait until timer callback releases sem
		hmi_evaluate();														// Refresh HMI (and VFO, BPF, etc)
		mon_evaluate();														// Check monitor input
	}

    return 0;
}
