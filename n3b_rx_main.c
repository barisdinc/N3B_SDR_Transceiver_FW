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

#include <hardware/i2c.h>

#include "n3b_rx_main.h"
#include "hmi.h"
#include "dsp.h"
#include "monitor.h"
#include "adf4360.h"
#include "ili9341.h"
#include "gfx.h"
#include "Fonts/FreeMono12pt7b.h"

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



// static const uint I2C_SLAVE_ADDRESS = 0x17;
// static const uint I2C_BAUDRATE = 100000; // 100 kHz

// static void run_master() {
    // gpio_init(MASTER_I2C0_SDA);
    // gpio_set_function(MASTER_I2C0_SDA, GPIO_FUNC_I2C);
    // // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
    // gpio_pull_up(MASTER_I2C0_SDA);

    // gpio_init(MASTER_I2C0_SCL);
    // gpio_set_function(MASTER_I2C0_SCL, GPIO_FUNC_I2C);
    // gpio_pull_up(MASTER_I2C0_SCL);

    // i2c_init(i2c0, I2C_BAUDRATE);

    // for (uint8_t mem_address = 0;; mem_address = (mem_address + 32) % 256) {
    //     char msg[32];
    //     snprintf(msg, sizeof(msg), "Hello, I2C slave! - 0x%02X", mem_address);
    //     uint8_t msg_len = strlen(msg);

        // uint8_t buf[32];
    //     buf[0] = mem_address;
    //     memcpy(buf + 1, msg, msg_len);
    //     // write message at mem_address
    //     printf("Write at 0x%02X: '%s'\n", mem_address, msg);
    //     int count = i2c_write_blocking(i2c0, I2C_SLAVE_ADDRESS, buf, 1 + msg_len, false);
    //     if (count < 0) {
    //         puts("Couldn't write to slave, please check your wiring!");
    //         return;
    //     }
    //     hard_assert(count == 1 + msg_len);

    //     // seek to mem_address
    //     count = i2c_write_blocking(i2c0, I2C_SLAVE_ADDRESS, buf, 1, true);
    //     hard_assert(count == 1);
    //     // partial read
    //     uint8_t split = 5;
    //     count = i2c_read_blocking(i2c0, I2C_SLAVE_ADDRESS, buf, split, true);
    //     hard_assert(count == split);
    //     buf[count] = '\0';
    //     printf("Read  at 0x%02X: '%s'\n", mem_address, buf);
    //     hard_assert(memcmp(buf, msg, split) == 0);
    //     // read the remaining bytes, continuing from last address
    //     count = i2c_read_blocking(i2c0, I2C_SLAVE_ADDRESS, buf, msg_len - split, false);
    //     hard_assert(count == msg_len - split);
    //     buf[count] = '\0';
    //     printf("Read  at 0x%02X: '%s'\n", mem_address + split, buf);
    //     hard_assert(memcmp(buf, msg + split, msg_len - split) == 0);

    //     puts("");
    //     sleep_ms(2000);
    // }
//     static struct
// {
//     uint8_t last_requested_mem_addr;
//     uint32_t frequency;
//     bool ptt_state;
//     bool tx_btn1;
//     bool tx_btn2;
//     bool tx_btn3;
//     bool tx_btn4;
//     // uint8_t tx_dat1;
//     // uint8_t tx_dat2;
//     // uint8_t tx_dat3;
//     // uint8_t tx_dat4;
// } tx_data;
// printf("size = %d\n", sizeof(tx_data));
//     // for(int ii=0;ii<10;ii++)
//     // {
//     // int count = i2c_read_blocking(i2c0, I2C_SLAVE_ADDRESS, buf, sizeof(tx_data), false);
// 	// for (int yy = 0; yy<sizeof(tx_data); yy++)
// 	// 	{
// 	// 		printf("c=%d %d %d\n",count, yy, buf[yy]);
// 	// 	}
//     // }

// }













int main()
{
	stdio_init_all();
	sleep_ms(2000);

	// ili9341_init();
	// ili9341_set_rotation(3);
	// ili9341_fill_screen(ILI9341_BLACK);
	// LCD_setPins(4, 12, 	5, 10, 11);
	// LCD_setSPIperiph(spi1);
	// LCD_initDisplay();
	// LCD_setRotation(3);
	// LCD_WritePixel(10, 10, 5);

	// int c=0;
	// GFX_setFont(&FreeMono12pt7b);
    // GFX_createFramebuf();

// const uint8_t n3b_display_map[] = {
 
// };
	// LCD_WriteBitmap(0,0,320,240,&n3b_display_map);
	// while(1){}
	// GFX_flush();
	// GFX_clearScreen();
    // run_master();

    // while (true)
    // {
    //     GFX_clearScreen();
    //     GFX_setCursor(0, 0);
    //     GFX_printf("Hello GFX!\n%d", c++);
    //     GFX_flush();
    //     sleep_ms(500);
    // }

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

	gpio_init(GP_LNA);
	gpio_set_dir(GP_LNA, GPIO_OUT);
	gpio_put(GP_LNA, true);									// Enable LNA

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
