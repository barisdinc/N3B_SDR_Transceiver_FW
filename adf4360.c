/*
 * adf4360.c
 *
 * Created: May 2024
 * Author: Baris Dinc TA7W / OH2UDS
 */ 

// #include "uSDR.h"
// #include "dsp.h"
#include "adf4360.h"
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include "pico/stdlib.h"

vfo_t vfo[1];				// 0: RX vfo     1: TX vfo

int si_getreg(uint8_t *data, uint8_t reg, uint8_t len)
{
  //not applicable for ADF4360
	return(len);
}
#define MSBFIRST 1
#define LSBFIRST 0

void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
      uint8_t i;

      for (i = 0; i < 8; i++)  {
            if (bitOrder == LSBFIRST)
                  gpio_put(dataPin, !!(val & (1 << i)));
            else      
                  gpio_put(dataPin, !!(val & (1 << (7 - i))));
                  
            gpio_put(clockPin, 1);
            gpio_put(clockPin, 0);            
      }
}

/*
* Calculate required register values and push them out
*/
void adf4360_evaluate(void)
{
  return;
	if (vfo[0].flag)
	{
    // prepare R register to be sent out first
    printf(" F: %ld\n",vfo[0].freq);
    unsigned long R_reg = 0x000000;
    uint8_t control_bits = 0b01 ; //R_reg
    unsigned long r_counter = R_COUNTER ;
    if (r_counter < 1) r_counter = 1 ;
    if (r_counter > 16383) r_counter = 16383 ;
    R_reg = (vfo[0].bsc << 20);
    R_reg = R_reg | (vfo[0].ldp << 18);
    R_reg = R_reg | (vfo[0].abpw << 16);
    R_reg = R_reg | (r_counter << 2);
    R_reg = R_reg | control_bits ; 
    printf(" R: %lX\n", R_reg) ;

    // send our R register
    gpio_put(ADF_LE, 0);
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (R_reg >> 16) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (R_reg >> 8) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (R_reg) );  
    gpio_put(ADF_LE, 1); sleep_ms(1); gpio_put(ADF_LE, 0); // latch data into registers


    // prepare Control register to be sent out first
    unsigned long C_reg = 0x000000;
    control_bits = 0b00 ;
    C_reg = (vfo[0].ps << 22);
    C_reg = C_reg | (vfo[0].pd << 20);
    C_reg = C_reg | (vfo[0].cp2s << 17);
    C_reg = C_reg | (vfo[0].cp1s << 14);
    C_reg = C_reg | (vfo[0].pwr << 12);
    C_reg = C_reg | (vfo[0].mtl << 11 );
    C_reg = C_reg | (vfo[0].cpg << 10 );
    C_reg = C_reg | (vfo[0].cpo << 9 ) ;
    C_reg = C_reg | (vfo[0].pdp << 8 );
    C_reg = C_reg | (vfo[0].mux << 5 );
    C_reg = C_reg | (vfo[0].cpl << 2 );
    C_reg = C_reg | control_bits ;
    printf(" C: %lX\n", C_reg);

    // send our Control register
    gpio_put(ADF_LE, 0);
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (C_reg >> 16) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (C_reg >> 8) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (C_reg) );  
    gpio_put(ADF_LE, 1); sleep_ms(1); gpio_put(ADF_LE, 0); // latch data into registers
    //delay(100);

    // prepare N register to be sent out first
    control_bits = 0b10 ;
    unsigned long N_reg = 0x000000;
    unsigned int steps = vfo[0].freq / PFD;
    unsigned long b_counter =  steps / (8 * (1 << vfo[0].ps)); // 00 -> 8  01 -> 16 10 -> 32
    unsigned int  a_counter = steps - (b_counter * (8 * (1 << vfo[0].ps)));
    if (b_counter < 3) b_counter = 3 ;
    if (b_counter > 8191) b_counter = 8191 ;
    if (a_counter < 2) a_counter = 2;
    if (a_counter > 31) a_counter = 31;
    N_reg = (vfo[0].div2i << 23);
    N_reg = N_reg | (vfo[0].div2o << 22);
    N_reg = N_reg | (vfo[0].cpg << 21);
    N_reg = N_reg | (b_counter << 8);
    N_reg = N_reg | (a_counter << 2);
    N_reg = N_reg | control_bits ;
    printf(" N: %lX\n",N_reg);

    // send our Control register
    gpio_put(ADF_LE, 1);
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (N_reg >> 16) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (N_reg >> 8) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (N_reg) );  
    gpio_put(ADF_LE, 1); sleep_ms(1); gpio_put(ADF_LE, 0); // latch data into registers
    //delay(100);

    // set vco updated flag
    vfo[0].flag = 0;
  }
}


// Initialize the ADF4360 VFO registers
void adf4360_init(void)
{
	// Hard initialize Synth registers: RX 739.750 MHz TX:2400.250 MHz
	// R=0x340051
	// N=0x00B83E
  // C=0x403924
	vfo[0].freq  = 10000000;
	vfo[0].flag  = 1;    // we want the following to be applied
	vfo[0].phase = 1;
  vfo[0].bsc   = 0;    // band select clock divider 00:1  01:2  10:4  11:8
  vfo[0].ldp   = 0;    // lock detect precision 00(0):3 cycles  01(1):5 cycles 
  vfo[0].abpw  = 3;    // anti backlash pulse widt 00(0):3.0ns  01(1):1.3ns  10(2):6.0ns  11(3):3.0ns
  vfo[0].ps    = 2;    // presaler  00(0):8/9  01(1):16/17  10(2):32/33  11(3):32/33
  vfo[0].pd    = 0;    // power down X0(0):normal 01(1):async 11(3):sync 
  vfo[0].cp2s  = 0;    // charge pump 2 current setting  000(0):0.31mA 001(1):0.62mA  010(2):0.93mA 011:1.25mA 100:1.56mA 101:1.87mA 110:2.18mA 111:2.50mA
  vfo[0].cp1s  = 0;    // charge pump 1 current setting  000(0):0.31mA 001(1):0.62mA  010(2):0.93mA 011:1.25mA 100:1.56mA 101:1.87mA 110:2.18mA 111:2.50mA
  vfo[0].pwr   = 3;    // output power level  00(0):3.5mA -13dBm  01(1):5.0mA -10dBm 10(2):7.5mA -7dBm  11(3):11.0mA -4dBm
  vfo[0].mtl   = 1;    // mute till lock 0:disabled 1:enabled
  vfo[0].cp    = 0;    // charge pump current setting to be used 0:cps1 1:cps2
  vfo[0].cpo   = 0;    // charge pump output 0:normal 1:3-state
  vfo[0].pdp   = 1;    // phase detector polarity 0:negative 1:positive
  vfo[0].mux   = 1;    // mux out 000(0):3-state output 001(1):digital lock detect(active high) 010(2):N divider 011(3):Vdd 100(4):R divider 101(5):opendrain lock detect 110(6):serial data 111(7):DGND
  vfo[0].cr    = 0;    // counter rest 0:normal 1:R,A,B counters held in reset
  vfo[0].cpl   = 2;    // core power level 00(0):5mA 01(1):10mA 10(2):15mA 11(3):20mA
	vfo[0].div2i = 0;    // prescaler input divide-by 2 select 0:fundamental 1:divided by 2
  vfo[0].div2o = 0;    // output divide-by 2 select 0:fundamental 1:divided by 2
  vfo[0].cpg   = 0;    // charge pump gain 0:use cp setting1  1:cp setting2
  vfo[0].b     = 184;  // 13 bit B counter
  vfo[0].a    = 15;    // 5 bit A counter

  //Initialize the SPI hardware
  //Output pins
	gpio_init(ADF_LE);
	gpio_init(ADF_CLK);
	gpio_init(ADF_DAT);
	gpio_set_dir(ADF_LE, GPIO_OUT);
	gpio_set_dir(ADF_CLK, GPIO_OUT);
	gpio_set_dir(ADF_DAT, GPIO_OUT);
	gpio_put(ADF_LE, 0);	
	gpio_put(ADF_CLK, 0);	
	gpio_put(ADF_DAT, 0);	

  // //input pins
  // gpio_init(ADF_MUX);
  // //gpio_pull_up(ADF_MUX);           
  // gpio_set_dir(ADF_MUX, GPIO_IN);    // MUX output from ADF4360, programmed for Digital LOCK Detect
  
  // push the new fequency out
  //adf4360_evaluate();

}
