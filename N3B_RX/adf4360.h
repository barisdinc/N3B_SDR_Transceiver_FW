#ifndef _ADF4360_H
#define _ADF4360_H


#define ADF_LE  13
#define ADF_CLK 14
#define ADF_DAT 15
// #define ADF_MUX 28


#define CRYSTAL 10000 //10MHz 10000KHz
#define PFD     10   //500KHz
#define R_COUNTER (CRYSTAL)/(PFD)
 
typedef struct
{
	uint32_t freq;		// type can hold up to 4GHz
	uint8_t  flag;		// flag != 0 when update needed
	uint8_t  phase;		// in quarter waves (0, 1, 2, 3)
  uint8_t  bsc;     // band select clock divider 00:1  01:2  10:4  11:8
  uint8_t  ldp;     // lock detect precision 00:3 cycles  01:5 cycles 
  uint8_t  abpw;    // anti backlash pulse widt 00:3.0ns  01:1.3ns  10:6.0ns  11:3.0ns
  uint8_t  ps;      // presaler  0:8/9  1:16/17  2:32/33  3:32/33
  uint8_t  pd;      // power down 
  uint8_t  cp2s;    // charge pump 2 current setting  000:0.31mA 001 0.62mA  010:0.93mA 011:1.25mA 100:1.56mA 101:1.87mA 110:2.18mA 111:2.50mA
  uint8_t  cp1s;    // charge pump 1 current setting  000:0.31mA 001 0.62mA  010:0.93mA 011:1.25mA 100:1.56mA 101:1.87mA 110:2.18mA 111:2.50mA
  uint8_t  pwr;     // output power level  00:3.5mA -13dBm  01:5.0mA -10dBm 10:7.5mA -7dBm  11:11.0mA -4dBm
  uint8_t  mtl;     // mute till lock 0:disabled 1:enabled
  uint8_t  cp;      // charge pump current setting to be used 0:cps1 1:cps2
  uint8_t  cpo;     // charge pump output 0:normal 1:3-state
  uint8_t  pdp;     // phase detector polarity 0:negative 1:positive
  uint8_t  mux;     // mux out 000:3-state out[put 001:digital lock detect(active high) 010:N divider 011:Vdd 100:R divider 101:opendrain lock detect 111:DGND
  uint8_t  cr;      // counter rest 0:normal 1:R,A,B counters held in reset
  uint8_t  cpl;     // core power level 00:5mA 01:10mA 10:15mA 11:20mA
	uint8_t  div2i;		// prescaler input divide-by 2 select 0:fundamental 1:divided by 2
  uint8_t  div2o;   // output divide-by 2 select 0:fundamental 1:divided by 2
  uint8_t  cpg;     // charge pump gain 0:use cp setting1  1:cp setting2
  uint16_t b;       // 13 bit B counter
  uint8_t  a;       // 5 bit A counter
} vfo_t;
extern vfo_t vfo[1];	// vfo[0] is for RX and vfo[1] is TX, which is on TX board


void adf4360_init(void);
// void adf4360_evaluate(uint32_t freq);
void adf4360_evaluate(double freq);

#define FREQ_RX_MAX 999990
#define FREQ_RX_MIN 500000


#define ADF4360_GETFREQ(i)		((((i)>=0)&&((i)<2))?vfo[(i)].freq:0)
#define ADF4360_INCFREQ(i, d)	if ((((i)>=0)&&((i)<2))&&((vfo[(i)].freq)<(FREQ_RX_MAX - (d)))) { vfo[(i)].freq += (d); vfo[(i)].flag = 1;}
#define ADF4360_DECFREQ(i, d)	if ((((i)>=0)&&((i)<2))&&((vfo[(i)].freq)>(FREQ_RX_MIN + (d)))) { (vfo[(i)].freq) -= (d); vfo[(i)].flag = 1;}
#define ADF4360_SETFREQ(i, f)	if ((((i)>=0)&&((i)<2))&&((f)<FREQ_RX_MAX)&&(f)>FREQ_RX_MIN) { vfo[(i)].freq = (f); vfo[(i)].flag = 1;}
#define ADF4360_SETPHASE(i, p)	if (((i)>=0)&&((i)<2)) {vfo[(i)].phase = ((uint8_t)p)&3; vfo[(i)].flag = 1;}




#endif /* _ADF4360_H */
