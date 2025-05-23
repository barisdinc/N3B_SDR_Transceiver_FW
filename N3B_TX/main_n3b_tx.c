/*
 *
 * The main loop of the application.
 * This initializes the units that do the actual work, and then loops in the background. 
 * Other units are:
 * - dsp.c, containing all signal processing in RX and TX branches. This part runs on the second processor core.
 * - hmi.c, contains all functions that handle user inputs
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/sem.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"

#include "main_n3b_tx.h"
#include "hmi.h"
#include "dsp.h"
#include "monitor.h"
#include "adf4360.h"



/*
 * Wrappers around i2c_write_blocking() and i2c_read_blocking()
 * The SDK functions return too soon, potentially causing overlapping calls
 */

int i2c_put_data(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop)
{
	int r = i2c_write_blocking(i2c, addr, src, len, nostop);
	sleep_us(I2C_LINGER_US);
	return(r);
}

int i2c_get_data(i2c_inst_t *i2c, uint8_t addr, uint8_t *dst, size_t len, bool nostop)
{
	int r = i2c_read_blocking(i2c, addr, dst, len, nostop);
	sleep_us(I2C_LINGER_US);
	return(r);
}


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
	 * Note that clk_peri (e.g. I2C) is derived from the SYS PLL
	 * Note that clk_adc sampling clock is derived from the 48MHz USB PLL.
	 */
	//vreg_set_voltage(VREG_VOLTAGE_1_25); sleep_ms(10);
	//set_sys_clock_khz(250000, false); sleep_ms(10);
	// sleep_ms(1000);
	stdio_init_all();
	/* 
	 * Initialize LED pin output 
	 */
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	gpio_put(PICO_DEFAULT_LED_PIN, true);									// Set LED on
	add_repeating_timer_ms(-LED_MS, led_callback, NULL, &led_timer);

	/*
	 * Initialize the PLL
	*/
	ADF4360_Init_new(ADF4360_0);
	// ADF4360_SetFrequency(2400277000);
	ADF4360_SetFrequency(2400277000);

	/*
	 * i2c0 is used for the si5351 interface
	 * i2c1 is used for the LCD and all other interfaces
	 * if the display cannot keep up, try lowering the i2c1 frequency
	 * Do not invoke i2c using functions from interrupt handlers!
	 */
	i2c_init(i2c0, 400000);													// i2c0 initialisation at 400Khz
	gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
	gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(I2C0_SDA);
	gpio_pull_up(I2C0_SCL);
	i2c_init(i2c1, 100000);													// i2c1 initialisation at 100Khz
	gpio_set_function(I2C1_SDA, GPIO_FUNC_I2C);
	gpio_set_function(I2C1_SCL, GPIO_FUNC_I2C);
	gpio_pull_up(I2C1_SDA);
	gpio_pull_up(I2C1_SCL);

	/* Initialize the SW units */
	mon_init();																// Monitor shell on stdio
	dsp_init();																// Signal processing unit
	hmi_init();																// HMI user inputs
	
	// /* A simple round-robin scheduler */
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
