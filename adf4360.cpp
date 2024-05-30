/*
 * adf4360.c
 *
 * Created: May 2024
 * Author: Baris Dinc TA7W / OH2UDS
 */ 

#include "Arduino.h"
#include "N3B.h"
#include "dsp.h"
#include "adf4360.h"

vfo_t vfo[1];				// 0: RX vfo     1: TX vfo

int si_getreg(uint8_t *data, uint8_t reg, uint8_t len)
{
  //not applicable for ADF4360
	return(len);
}



/*
* Calculate required register values and push them out
*/
void adf4360_evaluate(void)
{
  // return;
	if (vfo[0].flag)
	{
    // prepare R register to be sent out first
    uint32_t new_sat_rx_freq = (2*(vfo[0].freq + RX_SHFT_FREQ))/1000;  //TODO: Change 2* to a declare //739000000
    Serial.print("SF:"); Serial.println(new_sat_rx_freq,DEC) ;
    // Serial.print(" F:"); Serial.print(vfo[0].freq,DEC) ;
    unsigned long R_reg = 0x000000;
    uint8_t control_bits = B01 ; //R_reg
    unsigned long r_counter = R_COUNTER ;
    if (r_counter < 1) r_counter = 1 ;
    if (r_counter > 16383) r_counter = 16383 ;
    R_reg = (vfo[0].bsc << 20);
    R_reg = R_reg | (vfo[0].ldp << 18);
    R_reg = R_reg | (vfo[0].abpw << 16);
    R_reg = R_reg | (r_counter << 2);
    R_reg = R_reg | control_bits ; 
    Serial.print(" R:"); Serial.print(R_reg,HEX) ;

    // send our R register
    digitalWrite(ADF_LE, LOW);
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (R_reg >> 16) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (R_reg >> 8) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (R_reg) );  
    digitalWrite(ADF_LE, HIGH); delay(1);
    digitalWrite(ADF_LE, LOW);
    //delay(100);

    // prepare Control register to be sent out first
    unsigned long C_reg = 0x000000;
    control_bits = B00 ;
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
    Serial.print(" C:"); Serial.print(C_reg,HEX) ;

    // // send our Control register
    digitalWrite(ADF_LE, LOW);
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (C_reg >> 16) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (C_reg >> 8) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (C_reg) );  
    digitalWrite(ADF_LE, HIGH); delay(1);
    digitalWrite(ADF_LE, LOW);
    //delay(100);

    // // prepare N register to be sent out first
    control_bits = B10 ;
    unsigned long N_reg = 0x000000;
    unsigned int steps = new_sat_rx_freq / PFD ; //vfo[0].freq / PFD;
    unsigned long b_counter =  steps / (8 * (1 << vfo[0].ps)); // 00 -> 8  01 -> 16 10 -> 32
    unsigned int  a_counter = steps - (b_counter * (8 * (1 << vfo[0].ps)));
    if (b_counter < 3) b_counter = 3 ; //TODO: A nin sinir disi degerleir icin bolme oranini degistirip aradaki frekansi yakalamak lazim
    if (b_counter > 8191) b_counter = 8191 ;
    if (a_counter < 2) a_counter = 2;
    if (a_counter > 31) a_counter = 31;
    N_reg = (vfo[0].div2i << 23);
    N_reg = N_reg | (vfo[0].div2o << 22);
    N_reg = N_reg | (vfo[0].cpg << 21);
    N_reg = N_reg | (b_counter << 8);
    N_reg = N_reg | (a_counter << 2);
    N_reg = N_reg | control_bits ;
    Serial.print(" N:"); Serial.println(N_reg,HEX);
    Serial.print(" R : ");Serial.println(r_counter, DEC);
    Serial.print(" A : ");Serial.println(a_counter, DEC);
    Serial.print(" B : ");Serial.println(b_counter, DEC);
    unsigned long FN = ((b_counter * (8 * (1 << vfo[0].ps))) + a_counter) * PFD; 
    Serial.print(" FN: ");Serial.println(FN, DEC);
    

    // // send our Control register
    digitalWrite(ADF_LE, LOW);
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (N_reg >> 16) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (N_reg >> 8) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (N_reg) );  
    digitalWrite(ADF_LE, HIGH); delay(1);
    digitalWrite(ADF_LE, LOW);
    // //delay(100);

    // // set vco updated flag
    vfo[0].flag = 0;
  }
}


// Initialize the ADF4360 VFO registers
void adf4360_init(void)
{

	uint8_t data[16];

	// Hard initialize Synth registers: RX 739.750 MHz TX:2400.250 MHz
	// R=0x340051
	// N=0x00B83E
  // C=0x403924
	vfo[0].freq  = 777000;
	vfo[0].flag  = 1;    // we want the following to be applied
	vfo[0].phase = 1;
  vfo[0].bsc   = 3;    // band select clock divider 00:1  01:2  10:4  11:8
  vfo[0].ldp   = 0;    // lock detect precision 00(0):3 cycles  01(1):5 cycles 
  vfo[0].abpw  = 0;    // anti backlash pulse widt 00(0):3.0ns  01(1):1.3ns  10(2):6.0ns  11(3):3.0ns
  vfo[0].ps    = 2;//1;    // presaler  00(0):8/9  01(1):16/17  10(2):32/33  11(3):32/33
  vfo[0].pd    = 0;    // power down X0(0):normal 01(1):async 11(3):sync 
  vfo[0].cp2s  = 7;    // charge pump 2 current setting  000(0):0.31mA 001(1):0.62mA  010(2):0.93mA 011:1.25mA 100:1.56mA 101:1.87mA 110:2.18mA 111:2.50mA
  vfo[0].cp1s  = 7;    // charge pump 1 current setting  000(0):0.31mA 001(1):0.62mA  010(2):0.93mA 011:1.25mA 100:1.56mA 101:1.87mA 110:2.18mA 111:2.50mA
  vfo[0].pwr   = 0;    // output power level  00(0):3.5mA -13dBm  01(1):5.0mA -10dBm 10(2):7.5mA -7dBm  11(3):11.0mA -4dBm
  vfo[0].mtl   = 0;    // mute till lock 0:disabled 1:enabled
  vfo[0].cp    = 0;    // charge pump current setting to be used 0:cps1 1:cps2
  vfo[0].cpo   = 0;    // charge pump output 0:normal 1:3-state
  vfo[0].pdp   = 1;    // phase detector polarity 0:negative 1:positive
  vfo[0].mux   = 1;    // mux out 000(0):3-state output 001(1):digital lock detect(active high) 010(2):N divider 011(3):Vdd 100(4):R divider 101(5):opendrain lock detect 110(6):serial data 111(7):DGND
  vfo[0].cr    = 0;    // counter rest 0:normal 1:R,A,B counters held in reset
  vfo[0].cpl   = 2;    // core power level 00(0):5mA 01(1):10mA 10(2):15mA 11(3):20mA
	vfo[0].div2i = 0;//0;    // prescaler input divide-by 2 select 0:fundamental 1:divided by 2
  vfo[0].div2o = 0;    // output divide-by 2 select 0:fundamental 1:divided by 2
  vfo[0].cpg   = 0;    // charge pump gain 0:use cp setting1  1:cp setting2
  vfo[0].b     = 7708;  // 13 bit B counter
  vfo[0].a     = 10;    // 5 bit A counter

  //Initialize the SPI hardware
  gpio_set_dir(ADF_LE, GPIO_OUT); 
  gpio_set_dir(ADF_CLK, GPIO_OUT); 
  gpio_set_dir(ADF_DAT, GPIO_OUT); 
  //gpio_set_dir(ADF_MUX, GPIO_IN); 
  digitalWrite(ADF_LE, LOW);
  digitalWrite(ADF_CLK, LOW);
  digitalWrite(ADF_DAT, LOW);

  // push the new fequency out
  //adf4360_evaluate();

}
