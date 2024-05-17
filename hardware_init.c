/*
 * hardware_init.c
 *
 * Created: May 2024
 * Author: Baris Dinc OH2UDS/TA7W
 * 
 * The hardware initialization will take place in this file.
 * This initializes the units that do the actual work. 
 */

#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware_init.h"

void hardware_init_pins()
{
    //RP2040 initialize the GPIOs as inputs with pulldown as default, and this is like LNA disabled 
    //it needs a strong pullup = 1K to 3v3 on pin GPIO20 (pin 26) to force high level during initialization
    gpio_init(GPIO_LNA);
    gpio_pull_up(GPIO_LNA);             // LNA pullup  (it takes about 1s to reach this point after power up / reset)
    gpio_set_dir(GPIO_LNA, GPIO_IN);    // LNA input (just to confirm) - true for out, false for in 

	/* 
	 * Initialize LED pin output 
	 */
	gpio_init(PICO_DEFAULT_LED_PIN);
	gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
	gpio_put(PICO_DEFAULT_LED_PIN, true);	

	/*
	 * i2c0 is used for the si5351 interface
	 * i2c1 is used for the LCD and all other interfaces
	 * if the display cannot keep up, try lowering the i2c1 frequency
	 * Do not invoke i2c using functions from interrupt handlers!
	 */
	// i2c_init(i2c0, 400000);													// i2c0 initialisation at 400Khz
	// gpio_set_function(I2C0_SDA, GPIO_FUNC_I2C);
	// gpio_set_function(I2C0_SCL, GPIO_FUNC_I2C);
	// gpio_pull_up(I2C0_SDA);
	// gpio_pull_up(I2C0_SCL);
	// i2c_init(i2c1, 100000);													// i2c1 initialisation at 100Khz
	// gpio_set_function(I2C1_SDA, GPIO_FUNC_I2C);
	// gpio_set_function(I2C1_SCL, GPIO_FUNC_I2C);
	// gpio_pull_up(I2C1_SDA);
	// gpio_pull_up(I2C1_SCL);





}