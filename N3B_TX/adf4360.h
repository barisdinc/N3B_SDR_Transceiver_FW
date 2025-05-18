#ifndef _ADF4360_H
#define _ADF4360_H


#define ADF_LE  13
#define ADF_CLK 10
#define ADF_DAT 11
// #define ADF_MUX 28

#define CRYSTAL 10000 //10MHz 10000KHz
#define PFD     100   //500KHz
#define R_COUNTER (CRYSTAL)/(PFD)


/* ADF4360 part versions */
#define ADF4360_0       0
#define ADF4360_1       1
#define ADF4360_2       2
#define ADF4360_3       3
#define ADF4360_4       4
#define ADF4360_5       5
#define ADF4360_6       6
#define ADF4360_7       7
#define ADF4360_8       8
#define ADF4360_9       9


/* ADF4360 latch control bits  */
#define ADF4360_REG_CONTROL     	  0
#define ADF4360_REG_R_COUNTER		    1
#define ADF4360_REG_N_COUNTER		    2

/* Control Latch bits */
#define ADF4360_CTRL_PRESCALE(x)	      ((0x3 & (x)) << 22)
#define ADF4360_CTRL_PWR_DWN(x)	        ((0x3 & (x)) << 20)
#define ADF4360_CTRL_CURRENT1(x)        ((0x7 & (x)) << 17)
#define ADF4360_CTRL_CURRENT2(x)        ((0x7 & (x)) << 14)
#define ADF4360_CTRL_OUT_PWR_LVL(x)     ((0x3 & (x)) << 12)
#define ADF4360_CTRL_MTLD   			      (1 << 11)
#define ADF4360_CTRL_CP_GAIN   	   	    (1 << 10)
#define ADF4360_CTRL_CP_THREE_STATE 	  (1 << 9)
#define ADF4360_CTRL_PHASE_DETECT_POL	  (1 << 8)
#define ADF4360_CTRL_MUXOUT(x)     		  ((0x7 & (x)) << 5)
#define ADF4360_CTRL_COUNTER_RESET		  (1 << 4)
#define ADF4360_CTRL_CORE_POWER(x)  	  ((0x3 & (x)) << 2)




/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/*! Initialize the device. */
unsigned char ADF4360_Init_new(char adf4360Version);

/*! Write data into a register. */
void ADF4360_Write(unsigned long data);

/*! Powers down or powers up the device. */
void ADF4360_Power(unsigned char powerMode);

/*! Sets the ADF4360 frequency. */
unsigned long long ADF4360_SetFrequency(unsigned long long frequency);


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


void adf4360_init_old(void);
void adf4360_evaluate(double freq);
void adf4360_evaluate(double freq);

#define FREQ_RX_MAX 999990
#define FREQ_RX_MIN 500000


#define ADF4360_GETFREQ(i)		((((i)>=0)&&((i)<2))?vfo[(i)].freq:0)
#define ADF4360_INCFREQ(i, d)	if ((((i)>=0)&&((i)<2))&&((vfo[(i)].freq)<(FREQ_RX_MAX - (d)))) { vfo[(i)].freq += (d); vfo[(i)].flag = 1;}
#define ADF4360_DECFREQ(i, d)	if ((((i)>=0)&&((i)<2))&&((vfo[(i)].freq)>(FREQ_RX_MIN + (d)))) { (vfo[(i)].freq) -= (d); vfo[(i)].flag = 1;}
#define ADF4360_SETFREQ(i, f)	if ((((i)>=0)&&((i)<2))&&((f)<FREQ_RX_MAX)&&(f)>FREQ_RX_MIN) { vfo[(i)].freq = (f); vfo[(i)].flag = 1;}
#define ADF4360_SETPHASE(i, p)	if (((i)>=0)&&((i)<2)) {vfo[(i)].phase = ((uint8_t)p)&3; vfo[(i)].flag = 1;}



/* ADF4360_CTRL_PRESCALE(x) options. */
#define ADF4360_PRESCALE_8_9		      0
#define ADF4360_PRESCALE_16_17		    1
#define ADF4360_PRESCALE_32_33		    2

/* ADF4360_CTRL_PWR_DWN(x) options. */  
#define ADF4360_PWR_NORMAL_OPERATION        0
#define ADF4360_PWR_ASYNCH_POWER_DOWN       1
#define ADF4360_PWR_SYNCH_POWER_DOWN        3

/* ADF4360_CTRL_OUT_PWR_LVL(x) options. */ 
#define ADF4360_OUT_POWER_3_5   		    0
#define ADF4360_OUT_POWER_5_0           1
#define ADF4360_OUT_POWER_7_5           2
#define ADF4360_OUT_POWER_11_0		      3

/* #define ADF4360_CTRL_MUXOUT(x) options. */
#define ADF4360_MUX_THREE_STATE         0
#define ADF4360_MUX_DIGITAL_LD          1
#define ADF4360_MUX_N_DIVIDER           2
#define ADF4360_MUX_DVDD                3
#define ADF4360_MUX_R_DIVIDER           4
#define ADF4360_MUX_N_LD                5
#define ADF4360_MUX_SERIAL_DATA         6
#define ADF4360_MUX_DGND                7

/* ADF4360_CTRL_CORE_POWER(x) options. */ 
#define ADF4360_CORE_POWER_5     		0
#define ADF4360_CORE_POWER_10    		1
#define ADF4360_CORE_POWER_15    		2
#define ADF4360_CORE_POWER_20    		3

/* N Counter Latch bits */
#define ADF4360_N_CNT_DIVIDE_2_SELECT	    (1 << 23)
#define ADF4360_N_CNT_DIVIDE_2            (1 << 22)
#define ADF4360_N_CNT_CP_GAIN             (1 << 21)
#define ADF4360_N_CNT_B_COUNTER(x)		    ((0x1FFF & (x)) << 8)
#define ADF4360_N_CNT_A_COUNTER(x)		    ((0x1F & (x)) << 2) 

/* R Counter Latch bits */
#define ADF4360_R_CNT_BAND_CLK(x)       	((0x3 & (x)) << 20)
#define ADF4360_R_CNT_TEST		    		    (1 << 19)
#define ADF4360_R_CNT_LD_PRECISION	    	(1 << 18)
#define ADF4360_R_CNT_ANTIBACKLASH(x)     ((0x3 & (x)) << 16)
#define ADF4360_R_CNT_REF_COUNTER(x)      ((0x3FFF & (x)) << 2) 

/* ADF4360_R_CNT_BAND_CLK(x) options. */ 
#define ADF4360_BAND_DIVIDER_1			0
#define ADF4360_BAND_DIVIDER_2			1
#define ADF4360_BAND_DIVIDER_4			2
#define ADF4360_BAND_DIVIDER_8			3

/* ADF4360 Specifications */
#define ADF4360_MAX_FREQ_PFD            100000 //8000000 // Hz


/*****************************************************************************/
/************************** Types Declarations *******************************/
/*****************************************************************************/
/**
 * struct ADF4360_Specifications - Stores the minimum or maximum values that 
 *                                 reflect the performance of a device version.
 *
 * @ vcoMinFreq: Minimum frequency that the VCO can output.
 * @ vcoMaxFreq: Maximum frequency that the VCO can output.
 * @ countersMaxFreq: The maximum frequency that can be applied to A and B 
 *                    counters.
 * @ maxPrescalerVal: The maximum value of the dual-modulus prescaler. Some 
 *                    versions of the device do not have a prescaler, therefore 
 *                    the maximum value of the prescaler is set to 1.
 */
struct ADF4360_Specifications
{
    unsigned long long vcoMinFreq;
    unsigned long long vcoMaxFreq;
    unsigned long      countersMaxFreq;
    unsigned char      maxPrescalerVal;
};

/**
 * struct ADF4360_InitialSettings - Stores the settings that will be written to
 *        the device when the "ADF4360_Init" function is called.
 *
 * @ refIn: Input Reference Frequency. Maximum value accepted 250000000 Hz.
 * @ powerDownMode: Provides programmable power-down modes. Range 0..3
 * @ currentSetting2: Charge Pump Currents. Range 0..7
 * @ currentSetting1: Charge Pump Currents. Range 0..7
 * @ outPowerLevel: Set the output power level of the VCO. Range 0..3
 * @ muteTillLd: Mute-till-lock detect bit:
 *                  0 - functions disabled;
 *                  1 - RF outputs are not switched until PLL is locked.
 * @ cpGain: Charge pump gain bit:
 *              0 - Current Settings 1 is used;
 *              1 - Current Settings 2 is used.
 * @ cpThreeState: Charge Pump Three-State:
 *                    0 - normal operation;
 *                    1 - Puts the charge pump into three-state mode.
 * @ muxControl: Allows the user to access various internal points on the chip.
 *               Range 0..7
 * @ corePowerLevel: Sets the power level in the VCO core. Range 0..3
 * @ divideBy2Select: Divide-by-2 select bit:
 *                     0 - the fundamental is used as the prescaler input;
 *                     1 - divide-by-2 output is selected as the prescaler input
 * @ divideBy2: Divide-by-2 bit: 
 *                 0 - normal operation occurs;
 *                 1 - the output divide-by-2 functions is chosen.
 * @ lockDetectPrecision: Lock detect precision bit. Sets the number of 
 *                        reference cycles with less than 15 ns phase error for 
 *                        entering the locked state:
 *                           0 -  three cycles are taken;
 *                           1 -  five cycles are taken.
 * @ antiBacklash: Sets the antibacklash pulse width. Range 0..3.
 */
struct ADF4360_InitialSettings
{
    unsigned long  refIn;
    
    /* Control Latch */
    unsigned char preScalerMode;
    unsigned char powerDownMode;
    unsigned char currentSetting2;
    unsigned char currentSetting1;
    unsigned char outPowerLevel;
    unsigned char muteTillLd;
    unsigned char cpGain;
    unsigned char cpThreeState;
    unsigned char phaseDetectPol;
    unsigned char muxControl;
    unsigned char corePowerLevel;
    
    /* N Counter Latch */
    unsigned char divideBy2Select; // Not available for ADF4360-8 and ADF4360-9
    unsigned char divideBy2;       // Not available for ADF4360-8 and ADF4360-9
    
    /* R Counter Latch */
    unsigned char lockDetectPrecision;
    unsigned char antiBacklash;
};


#endif /* _ADF4360_H */
