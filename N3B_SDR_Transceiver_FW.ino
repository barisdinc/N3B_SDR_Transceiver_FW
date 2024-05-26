
/*
 * 
 * Created: May 2024
 * Author: Baris DINC OH2UDS/TA7W
 * https://github.com/barisdinc
 * 
 * Inspired from the work done by Guido, Arjante Marvel and Klaus for uSDX
 * Thanks for their contrubutin to mcu based SDX rigs
 *
*/


#include "N3B.h"
#include "hmi.h"

void setup() {

  // //RP2040 initialize the GPIOs as inputs with pulldown as default, and this is like LNA disabled 
  // gpio_pull_up(GP_LNA);             // LNA pullup  (it takes about 1s to reach this point after power up / reset)
  // gpio_set_dir(GP_LNA, GPIO_IN);    // LNA input (just to confirm) - true for out, false for in 

  // gpio_pull_up(GP_PTT);             // PTT pullup  (it takes about 1s to reach this point after power up / reset)
  // gpio_set_dir(GP_PTT, GPIO_IN);    // PTT input (just to confirm) - true for out, false for in 

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  gpio_init_mask(1<<LED_BUILTIN);  
  gpio_set_dir(LED_BUILTIN, GPIO_OUT); 

  Serialx.begin(115200);  
  while (!Serialx && millis() < 10000UL);
  Serialx.println("started");

  uint16_t tim = millis();

  //special jobs while waiting initial display print
  N3B_info();  //write something into display while waiting for the serial and DFLASH read
  hmi_init0();     //it could take some time to read all DFLASH data

  // // some delay required for Serial to open
   while((millis() - tim) < 500)   //try for 5s to connect to serial
  {
    gpio_set_mask(1<<LED_BUILTIN);
    delay(50);                       // wait
    gpio_clr_mask(1<<LED_BUILTIN);
    delay(50);                       // wait
  }  // If the serial is not open on 5s, it goes ahead and the serial print commands will be called but with no effect

  Serialx.println("\n\n***  N3B QO-100  ***");
  Serialx.println("\nSDR Satellite Terminal");
  Serialx.println("\nSerial took " + String((millis() - tim)) + "ms to start");

  N3B_setup();
}

void loop(void)
{
  N3B_loop();
}


 
