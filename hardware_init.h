#ifndef __HARDWARE_INIT_H__
#define __HARDWARE_INIT_H__
/* 
 * hardware_init.h
 *
 * Created: May 2024
 * Author: Baris Dinc OH2UDS/TA7W
 *
 * This file contains the system-wide hae=rdware definitions.
 *
 */


/* GPIO (pin) assignments */

#define RXUART_TX               0                                           // Pin  1: Pin for direct serial input/output TX
#define RXUART_RX               1                                           // Pin  2: Pin for direct serial input/output RX
#define GPIO_ENC_A				2											// Pin  4: Encoder channel A
#define GPIO_ENC_B				3											// Pin  5: Encoder channel B
#define ILI_DC                  4                                           // Pin  6: ILI 9341 display DC pin
#define ILI_RST                 5                                           // Pin  7: ILI 9341 display RESET pin
#define GPIO_BTN_0				6											// Pin  9: Enter, Confirm
#define GPIO_BTN_1				7											// Pin 10: Escape, Cancel
#define GPIO_BTN_2				8											// Pin 11: Left move
#define GPIO_BTN_3				9											// Pin 12: Right move
#define ILI_SCK                 10                                          // Pin 14: ILI 9341 display CLOCK pin
#define ILI_MOSI                11                                          // Pin 15: ILI 9341 display MOSI/DATA pin
#define ILI_CS                  12                                          // Pin 16: ILI 9341 display Chip Select pin
#define ADF4360_RX_LE           13                                          // Pin 17: ADF4360 PLL/VCO Latch Enable pin
#define ADF4360_RX_CLK          14                                          // Pin 19: ADF4360 PLL/VCO Clock pin
#define ADF4360_RX_DATA         15                                          // Pin 20: ADF4360 PLL/VCO Data pin
#define MCUTX_I2C0_SDA		    16											// Pin 21: I2C channel 0 - data connected to TX MCU via I2C bus
#define MCUTX_I2C0_SCL			17											// Pin 22: I2C channel 0 - clock  connected to TX MCU via I2C bus
//#define I2C1_SDA				18											// Pin 24: I2C channel 1 - data
//#define I2C1_SCL				19											// Pin 25: I2C channel 1 - clock
//#define DAC_Q					20											// Pin 26: PWM DAC Q channel
//#define DAC_I					21											// Pin 27: PWM DAC I channel
#define GPIO_LNA				20											// Pin 20: PTT line (low is active)
#define DAC_A					22											// Pin 29: PWM DAC Audio channel (RX Audio Out)
#define ADC_Q					26											// Pin 31: ADC 0
#define ADC_I					27											// Pin 32: ADC 1
#define ADC_A					28											// Pin 34: ADC 2

void hardware_init_pins();


#endif //HARDWARE_INIT_H
