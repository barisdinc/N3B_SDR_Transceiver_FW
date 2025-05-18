/*
 * adf4360.c
 *
 * Created: May 2024
 * Author: Baris Dinc TA7W / OH2UDS
 */ 

#include <stdio.h>
#include "pico/time.h"
#include <hardware/gpio.h>
#include "adf4360.h"
#include "adf4360_4_prj.h"

#define MSBFIRST 1
#define LSBFIRST 0

vfo_t vfo[1];
unsigned char ver         = 0;
unsigned char prescaleVal = 32;
unsigned long regR        = 0;
unsigned long regCtrl     = 0;
unsigned long regN        = 0;


void shiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
      uint8_t i;
      for (i = 0; i < 8; i++)  {
            if (bitOrder == LSBFIRST)
                  gpio_put(dataPin, !!(val & (1 << i)));
            else      
                  gpio_put(dataPin, !!(val & (1 << (7 - i))));
                  
            gpio_put(clockPin, 1);
            sleep_us(100);
            gpio_put(clockPin, 0);            
            sleep_us(100);
      }
      gpio_put(dataPin,0);
}



/***************************************************************************//**
 * @brief Initialize the device.
 *
 * @param ver - AD4360 version.
 *                      Example: 0 - ADF4360-0
 *                               1 - ADF4360-1
 *                               ...
 *                               8 - ADF4360-8
 *                               9 - ADF4360-9
 *
 * @return status - Result of the initialization procedure.
 *					Example: 0x0 - SPI peripheral was not initialized.
 *				  			 0x1 - SPI peripheral is initialized.
*******************************************************************************/
unsigned char ADF4360_Init_new(char adf4360Version)
{
	unsigned char status = 0x0;

  //Initialize the SPI hardware
	gpio_init(ADF_LE);
	gpio_init(ADF_CLK);
	gpio_init(ADF_DAT);
	gpio_set_dir(ADF_LE, GPIO_OUT);
	gpio_set_dir(ADF_CLK, GPIO_OUT);
	gpio_set_dir(ADF_DAT, GPIO_OUT);

  /* Store the version of the device in use. */
  ver = adf4360Version;
  /* Initialize ADF4360 registers. */
  regR = ADF4360_R_CNT_LD_PRECISION * ADF4360_st.lockDetectPrecision | 
          ADF4360_R_CNT_ANTIBACKLASH(ADF4360_st.antiBacklash);   
  ADF4360_Write(ADF4360_REG_R_COUNTER | regR);
  regCtrl = ADF4360_CTRL_PRESCALE(ADF4360_st.preScalerMode)|
            ADF4360_CTRL_PWR_DWN(ADF4360_st.powerDownMode)|
            ADF4360_CTRL_CURRENT1(ADF4360_st.currentSetting1) |
            ADF4360_CTRL_CURRENT2(ADF4360_st.currentSetting2) |
            ADF4360_CTRL_OUT_PWR_LVL(ADF4360_st.outPowerLevel) | 
            ADF4360_CTRL_MTLD * ADF4360_st.muteTillLd | 
            ADF4360_CTRL_CP_GAIN * ADF4360_st.cpGain | 
            ADF4360_CTRL_CP_THREE_STATE * ADF4360_st.cpThreeState | 
            ADF4360_CTRL_PHASE_DETECT_POL * ADF4360_st.phaseDetectPol | 
            ADF4360_CTRL_MUXOUT(ADF4360_st.muxControl) | 
            ADF4360_CTRL_CORE_POWER(ADF4360_st.corePowerLevel);
//   printf("CONTROL REG: %lu \r\n",regCtrl);
  ADF4360_Write(ADF4360_REG_CONTROL | regCtrl);
  /* Recommended Interval Between Control Latch and N Counter Latch writes. */
  // TIME_DelayMs(10);
  sleep_us(1000);
  regN =   ADF4360_N_CNT_DIVIDE_2_SELECT * ADF4360_st.divideBy2Select |
            ADF4360_N_CNT_DIVIDE_2 * ADF4360_st.divideBy2;
  ADF4360_Write(ADF4360_REG_N_COUNTER | regN);
    
	return(status);
}


/***************************************************************************//**
 * @brief Write data into a register.
 *
 * @param data - Data value to write.
 *
 * @return None.
*******************************************************************************/
void ADF4360_Write(unsigned long data)
{
  printf("ADF4360 write...\r\n");
  gpio_put(ADF_LE, 0);
  shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data & 0xFF0000) >> 16));
  shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data & 0x00FF00) >> 8) );
  shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data & 0x0000FF) >> 0) );  
  printf("> %lx <\r\n",data);
  gpio_put(ADF_LE, 1); sleep_ms(1);
  gpio_put(ADF_LE, 0);
  sleep_ms(10);

//0x340039 - 1    0011 0100 0000 0000 0011 1001
//0x000924 - 0    0000 0000 0000 1001 0010 1000
//0x002A02 - 2    0000 0000 0010 1010 0000 0010


  // printf("ADF4360 write...\r\n");
  // gpio_put(ADF_LE, 0);
  // unsigned long data1 = 0x340039; //0x001F0A;//orta
  // unsigned long data2 = 0x000924; //0x340005;//sag
  // unsigned long data3 = 0x002A02; //0x000924;


  // printf("> %lx <\r\n",data1);
  // printf("> %lx <\r\n",data2);
  // printf("> %lx <\r\n",data3);
  // shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data1 & 0xFF0000) >> 16));
  // shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data1 & 0x00FF00) >> 8) );
  // shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data1 & 0x0000FF) >> 0) );  
  //   gpio_put(ADF_LE, 1); sleep_ms(1);
  // gpio_put(ADF_LE, 0);
  // sleep_ms(10);
  // shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data2 & 0xFF0000) >> 16));
  // shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data2 & 0x00FF00) >> 8) );
  // shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data2 & 0x0000FF) >> 0) );  
  //   gpio_put(ADF_LE, 1); sleep_ms(1);
  // gpio_put(ADF_LE, 0);
  // sleep_ms(10);
  // shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data3 & 0xFF0000) >> 16));
  // shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data3 & 0x00FF00) >> 8) );
  // shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, ((data3 & 0x0000FF) >> 0) );  
  //   gpio_put(ADF_LE, 1); sleep_ms(1);
  // gpio_put(ADF_LE, 0);
  // sleep_ms(10);




// SF: 1479554 
//  F: 777000 
//  R: 300fa1 
//  C: 8fc128 
//  N: 120f4e 
//  R : 1000 
//  A : 19 
//  B : 4623 
//  FN: 1479550 

}




/***************************************************************************//**
 * @brief Powers down or powers up the device.
 *
 * @param powerMode - Power option.
 *                    Example: 0 - powers down the device;
 *                             1 - power up the device.
 *
 * @return None.
*******************************************************************************/
void ADF4360_Power(unsigned char powerMode)
{
    if(powerMode)
    {
        ADF4360_Write(ADF4360_REG_CONTROL |
                      regCtrl | 
                      ADF4360_CTRL_PWR_DWN(ADF4360_PWR_NORMAL_OPERATION));
    }
    else
    {
        ADF4360_Write(ADF4360_REG_CONTROL |
                      regCtrl |
                      ADF4360_CTRL_PWR_DWN(ADF4360_PWR_SYNCH_POWER_DOWN));
    }
}


/***************************************************************************//**
 * @brief Increases the R counter value until the maximum frequency of PFD is
 *        greater than PFD frequency.
 *
 * @param rCounter - R counter value.
 *
 * @return rCounter - modified R counter value.
*******************************************************************************/
unsigned short ADF4360_TuneRcounter(unsigned short rCounter)
{
	unsigned long frequencyPfd = 0;	// PFD frequency
	
	do
	{
		rCounter++;
		frequencyPfd = ADF4360_st.refIn / rCounter;
	}
	while(frequencyPfd > ADF4360_MAX_FREQ_PFD);
    
    return rCounter;
}


/***************************************************************************//**
 * @brief Selects a value for Band Select Clock Divider that is used to divide 
 *        the output of the R counter until a frequency below 1 MHz is obtained.
 *
 * @param frequencyPfd - Frequency value of Phase Frequency Detector.
 *
 * @return bsc - Band Select Clock value.
*******************************************************************************/
unsigned short ADF4360_GetBandDivider(unsigned long frequencyPfd)
{
	unsigned long dividedRfreq = 0;
	unsigned char bsc          = 1;
    
    /* The R counter output is used as the clock for the band select logic and 
       should not exceed 1 MHz. */
	
    dividedRfreq = frequencyPfd;
    while((dividedRfreq > 1000000) && (bsc < 8))
	{
        bsc *= 2;
		dividedRfreq = frequencyPfd / bsc;
	}
    
    return bsc;
}


/***************************************************************************//**
 * @brief Sets the ADF4360 frequency.
 *
 * @param frequency - The desired frequency value.
 *
 * @return calculatedFrequency - The actual frequency value that was set.
*******************************************************************************/
unsigned long long ADF4360_SetFrequency(unsigned long long frequency)
{
    unsigned long long vcoFrequency        = 0; // VCO frequency
    unsigned long      frequencyPfd        = 0;	// PFD frequency
    unsigned long      freqRatio           = 0; // VCOfreq / PFDfreq
    unsigned long long calculatedFrequency = 0; // Actual VCO frequency
    unsigned short	   rCounterValue 	   = 0; // Value for R counter
    unsigned short     a                   = 0; // Value for A counter
    unsigned short     b                   = 0; // Value for B counter
    unsigned char      band                = 0; // Band Select Clock Value
    unsigned char      bandBits            = 0; // Band Select Clock Bits
    
    /* Force "frequency" parameter to fit in the Output frequency range. */
    if(frequency <= ADF4360_part[ver].vcoMaxFreq)
    {
        if(frequency >= ADF4360_part[ver].vcoMinFreq)
        {
            vcoFrequency = frequency;
        }
        else
        {
            vcoFrequency = ADF4360_part[ver].vcoMinFreq;
        }
    }
    else
    {
        vcoFrequency = ADF4360_part[ver].vcoMaxFreq;
    }
    /* If ADF4360-8 or ADF4360-9 are used. */
    if(ver > ADF4360_7)
    {
        /* Dual-modulus prescaler does not exist. */
        prescaleVal = 1;
        /* A counter does not exist or has a different purpose. */
        a = 0;
        /* Get the actual PFD frequency. */
        rCounterValue = ADF4360_TuneRcounter(rCounterValue);
        frequencyPfd = ADF4360_st.refIn / rCounterValue;
        /* Find Counter B value using VCO frequency and PFD frequency. */
        b = (unsigned long)((float)vcoFrequency / frequencyPfd + 0.5);
    }
    else // If ADF4360-0, ADF4360-1, ... , ADF4360-7 are used. 
    {
        /* Adjust the dual-modulus prescaler value so that counters A and B will 
        be supplied by a clock that has a frequency below "countersMaxFreq". */
        // printf("vco %llu vvc/pres %llu max %lu \r\n",vcoFrequency ,(vcoFrequency / prescaleVal),ADF4360_part[ver].countersMaxFreq);
        while(((vcoFrequency / prescaleVal) > 
              ADF4360_part[ver].countersMaxFreq) && 
              (prescaleVal <= ADF4360_part[ver].maxPrescalerVal))
        {
            prescaleVal *= 2;
            // printf("found prescalval %d\r\n",prescaleVal);
        }
        do
        {
            /* Get the actual PFD frequency. */
            rCounterValue = ADF4360_TuneRcounter(rCounterValue);
            frequencyPfd = ADF4360_st.refIn / rCounterValue;
            /* Find the values for Counter A and Counter B using VCO frequency 
            and PFD frequency. */
            freqRatio = (unsigned long)(((float)vcoFrequency / frequencyPfd) + 0.5);
            b = freqRatio / prescaleVal;
            a = freqRatio % prescaleVal;
            // printf("VCO freq %llu \r\n",vcoFrequency);
            // printf("freqRatio %lu\r\n",freqRatio);
            // printf("rcounter %d freqPFD %lu *a,b= %d,%d\r\n", rCounterValue, frequencyPfd,a,b);
            // printf("prescale %d\r\n",prescaleVal);
        }while((a > b) && (b < 3)); // B must be greater or equal to A
    }
    /* Find the actual VCO frequency. */
    calculatedFrequency = ((b * prescaleVal) + a) * frequencyPfd;
    /* Select the value of the bits for the Band Select Clock Divider. */
    band = ADF4360_GetBandDivider(frequencyPfd);
    /* Relationshio between band value and band bits. */
    band = 8; 
    bandBits = (unsigned char)(((band - 1) * 0.42) + 0.8);
    /* Load the saved values into ADF4118 registers using Counter Reset
    Method. */
    // printf("R : %02x Band Select : %02x  \r\n", rCounterValue, bandBits);
    // printf("RReg : %lu \r\n",(ADF4360_REG_R_COUNTER | regR | ADF4360_R_CNT_BAND_CLK(bandBits) | ADF4360_R_CNT_REF_COUNTER(rCounterValue)));
    ADF4360_Write(ADF4360_REG_R_COUNTER |          // Select R Counter Register
                  regR |                           // Write the fixed settings
                  ADF4360_R_CNT_BAND_CLK(bandBits) |
                  ADF4360_R_CNT_REF_COUNTER(rCounterValue));
    ADF4360_Write(ADF4360_REG_CONTROL |             // Select Control Register
                  regCtrl |                         // Write the fixed settings
                  ADF4360_CTRL_PRESCALE(prescaleVal / 16));
    /* Recommended Interval Between Control Latch and N Counter Latch writes. */
    //TIME_DelayMs(10);
    sleep_us(1000);
    ADF4360_Write(ADF4360_REG_N_COUNTER |           // Select N Counter Register
                  regN |                            // Write the fixed settings
                  ADF4360_N_CNT_B_COUNTER(b) | 
                  ADF4360_N_CNT_A_COUNTER(a));
    
    return calculatedFrequency;
}

/*
* Calculate required register values and push them out
*/
void adf4360_evaluate(double freq)
{
  if (vfo[0].freq == freq) return;
  vfo[0].freq = freq; //TODO: bunu duzelt.. 
  vfo[0].flag = 1;

	if (vfo[0].flag)
	{
    // prepare R register to be sent out first
    // printf("--------------------------------\n");
    #define RX_IF_FREQ    9750000000
    #define LNB_DRIFT     0 //60000
    #define RX_SHFT_FREQ  10489000000 - RX_IF_FREQ + LNB_DRIFT

    uint32_t new_sat_rx_freq = (2*(vfo[0].freq + RX_SHFT_FREQ))/1000;  //TODO: Change 2* to a declare //739000000
    // printf("SF: %ld \n",new_sat_rx_freq);
    // printf(" F: %ld \n",vfo[0].freq) ;
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
    // printf(" R: %lx \n",R_reg);

  	// R_reg=0x340051;
    // send our R register
    gpio_put(ADF_LE, 0);
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (R_reg >> 16) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (R_reg >> 8) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (R_reg) );  
    gpio_put(ADF_LE, 1); sleep_ms(1);
    gpio_put(ADF_LE, 0);
    sleep_ms(10);

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
    // printf(" C: %lx \n",C_reg);

    // C_reg=0x403924;
    // // send our Control register
    gpio_put(ADF_LE, 0);
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (C_reg >> 16) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (C_reg >> 8) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (C_reg) );  
    gpio_put(ADF_LE, 1); sleep_ms(1);
    gpio_put(ADF_LE, 0);
    sleep_ms(10);

    // // prepare N register to be sent out first
    control_bits = 0b10 ;
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
    // printf(" N: %lx \n",N_reg);

    // printf(" R : %ld \n",r_counter);
    // printf(" A : %d \n",a_counter);
    // printf(" B : %ld \n",b_counter);
    // unsigned long FN = ((b_counter * (8 * (1 << vfo[0].ps))) + a_counter) * PFD; 
    // printf(" FN: %ld \n", FN);
    
	  // N_reg=0x00B83E;
    // // send our Control register
    gpio_put(ADF_LE, 0);
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (N_reg >> 16) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (N_reg >> 8) );
    shiftOut(ADF_DAT, ADF_CLK, MSBFIRST, (N_reg) );  
    gpio_put(ADF_LE, 1); sleep_ms(1);
    gpio_put(ADF_LE, 0);
    sleep_ms(10);

    // // set vco updated flag
    vfo[0].flag = 0;
  }
}


// Initialize the ADF4360 VFO registers
void adf4360_init_old(void)
{

	//uint8_t data[16];

	// Hard initialize Synth registers: RX 739.750 MHz TX:2400.250 MHz
	// R=0x340051
	// N=0x00B83E
  // C=0x403924
	vfo[0].freq  = 0;
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
	gpio_init(ADF_LE);
	gpio_init(ADF_CLK);
	gpio_init(ADF_DAT);
	gpio_set_dir(ADF_LE, GPIO_OUT);
	gpio_set_dir(ADF_CLK, GPIO_OUT);
	gpio_set_dir(ADF_DAT, GPIO_OUT);

  // push the new fequency out
  adf4360_evaluate(777000);

}
