#include "Arduino.h"
#include <Wire.h>
#include "N3B.h"
#include "hmi.h"
#include "dsp.h"
#include "adf4360.h"
// #include "hmc830.h"
#include "monitor.h"
#include "relay.h"
#include "TFT_eSPI.h"
#include "display_tft.h"

uint16_t tim_loc;     // local time 

void N3B_info(void)  //main
{
  display_tft_info();
}



void N3B_setup(void)  //main
{
 
   Wire.begin();            //i2c0 master to TX_MCU
   //Wire.setClock(200000);   // Set i2c0 clock speed (default=100k)
  
	/* Initialize units */
	// mon_init();										// Monitor shell on stdio
	adf4360_init();										// VFO control unit
  // HMC830_Init(HMC830_HMC_MODE);
  display_tft_setup();   //moved to setup0 to write into display from the beggining
	hmi_init();										// HMI user inputs
  dsp_init();                   // Signal processing unit
  tim_loc = tim_count;          // local time for main loop

// while (1)
//   {
//     //HMC830_HMC_Write_Freq(50, 1, 345.678, 2.54);
//     HMC830_HMC_Write_Freq(4, 1, 34500.678, 2.54);
//     //HMC830_HMC_Write_Freq(40, 1, 250, 2.54);
//   //HMC830_HMC_Test_REF50M_650M();

    
//     delay(1);
      
//     uint32_t id = HMC830_HMC_Read_Chip_ID();
//     uint32_t lock = HMC830_HMC_Read_Lock_Detect();
//     uint16_t refdiv = HMC830_HMC_Read_REFDIV();
//     double ndiv = HMC830_HMC_Read_NDIV();
      
//     char UserTxBuffer[64];
// //    memset(UserTxBuffer, 0, sizeof(UserTxBuffer));
//       //sprintf(UserTxBuffer,"HMC830 ID:0x%X\r\nLD:%s\r\n", id, lock == HMC830_LOCKED ? "LOCKED" : "UNLOCKED");
//     Serialx.print("HMC ID :"+String(id)+" ");
//     Serialx.println(id, HEX);
//     Serialx.println("REF DIV :"+String(refdiv));
//     Serialx.println("N DIV :"+String(ndiv));
//     if (lock == HMC830_LOCKED)
//     {
//       Serialx.println("UNLOCKED");
//     }
//     else
//     {
//       Serialx.println("LOCKED");
//     }
//     Serialx.println("----");
//     //CDC_Transmit_FS((uint8_t*)UserTxBuffer,sizeof(UserTxBuffer));
    
//     delay(1000);
//     /* USER CODE END WHILE */

//     /* USER CODE BEGIN 3 */
//   }


}




void N3B_loop(void)
{ 

  if((uint16_t)(tim_count - tim_loc) >= (uint16_t)LOOP_MS)  //run the tasks every 100ms  (LOOP_MS = 100)
  {
    hmi_evaluate();               // Refresh HMI
    adf4360_evaluate();                // Refresh VFO settings
    // mon_evaluate();               // Check monitor input
    dsp_loop();  //spend more time here for FFT and graphic
    display_tft_loop();           // Refresh display
    //it takes 50ms for the tasks (most in hmi_evaluate() to plot the waterfall)
    
    tim_loc += LOOP_MS;
  }

}
