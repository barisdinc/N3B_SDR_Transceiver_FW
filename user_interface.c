/*
 * user_interface.c
 *
 * Created: May 2022
 * Author: Baris DINC OH2UDS/TA7W
 * 
 * This file contains the user interface driver, processing user inputs.
 * It will also do the logic behind these, and write feedback to the TFT/LCD.
 *
 * We have 2 rotary encoders and 8 buttons (2 on rotary encoders)
 * 
 * These buttons have the following functions:
 * 
 * RX Side :
 * GP6 - Enter, confirm : Used to select menu items or make choices from a list
 * GP7 - Escape, cancel : Used to exit a (sub)menu or cancel the current action
 * GP8 - Left           : Used to move left, e.g. to select a digit
 * GP9 - Right			: Used to move right, e.g. to select a digit
 * 
 * TX Side (connected to TX MCU, and data is sent to RX MCU using I2C channel on GP16/GP17):
 * GP6 - Enter, confirm : Used to select menu items or make choices from a list
 * GP7 - Escape, cancel : Used to exit a (sub)menu or cancel the current action
 * GP8 - Left           : Used to move left, e.g. to select a digit
 * GP9 - Right			: Used to move right, e.g. to select a digit
 *
 * The rotary encoders (GP2, GP3 on RX MCU) and (GP2, GP3 on TX MCU) controls an up/down counter connected to some field. 
 *     ___     ___
 * ___|   |___|   |___  A
 *   ___     ___     _
 * _|   |___|   |___|   B
 *
 * Encoder channel A triggers on falling edge. 
 * Depending on B level, count is incremented or decremented.
 * 
 *
 */
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

#include "n3b_rx_main.h"
#include "display_ili9341.h"
#include "user_interface.h"
//#include "dsp.h"
#include "adf4360.h"

//#define UI_debug    0    //to release some serial print outs for debug

/*
 * GPIO assignments
 */
#define GP_ENC_A	2               // Encoder clock
#define GP_ENC_B	3               // Encoder direction
#define GP_AUX_0_Enter	6								// Enter, Confirm
#define GP_AUX_1_Escape	7								// Escape, Cancel
#define GP_AUX_2_Left	8								// Left move
#define GP_AUX_3_Right	9								// Right move
#define GP_MASK_IN	((1<<GP_ENC_A)|(1<<GP_ENC_B)|(1<<GP_AUX_0_Enter)|(1<<GP_AUX_1_Escape)|(1<<GP_AUX_2_Left)|(1<<GP_AUX_3_Right))
//#define GP_MASK_PTT	(1<<GP_PTT)


#define ENCODER_FALL              10    //increment/decrement freq on falling of A encoder signal
#define ENCODER_FALL_AND_RISE     22    //increment/decrement freq on falling and rising of A encoder signal
#define ENCODER_TYPE              ENCODER_FALL      //ENCODER_FALL_AND_RISE //choose to trigger the encoder step only on fall of A signal

#define ENCODER_CW_A_FALL_B_LOW   10    //encoder type clockwise step when B low at falling of A
#define ENCODER_CW_A_FALL_B_HIGH  22    //encoder type clockwise step when B high at falling of A
#define ENCODER_DIRECTION         ENCODER_CW_A_FALL_B_LOW    //ENCODER_CW_A_FALL_B_HIGH //direction related to B signal level when A signal is triggered

/*
 * Event flags
 */
//#define GPIO_IRQ_ALL		    (GPIO_IRQ_LEVEL_LOW|GPIO_IRQ_LEVEL_HIGH|GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE)
#define GPIO_IRQ_EDGE_ALL	  (GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE)

/*
 * Display layout:
 *   +----------------+
 *   |USB 14074.0 R920| --> mode=USB, freq=14074.0kHz, state=Rx,S9+20dB
 *   |      Fast -10dB| --> ..., AGC=Fast, Pre=-10dB
 *   +----------------+
 * In this HMI state only tuning is possible, 
 *   using Left/Right for digit and ENC for value, Enter to commit change.
 * Press ESC to enter the submenu states (there is only one sub menu level):
 *
 * Submenu	Values								ENC		Enter			Escape	Left	Right
 * -----------------------------------------------------------------------------------------------
 * Mode		USB, LSB, AM, CW					change	commit			exit	prev	next
 * AGC		Fast, Slow, Off						change	commit			exit	prev	next
 * Pre		+10dB, 0, -10dB, -20dB, -30dB		change	commit			exit	prev	next
 * Vox		NoVOX, Low, Medium, High			change	commit			exit	prev	next
 *
 * --will be extended--
 */

 

//char ui_o_menu[ui_NMENUS][8] = {"Tune","Mode","AGC","Pre","VOX"};	// Indexed by ui_menu  not used - menus done direct in Evaluate()
char ui_o_mode[UI_NUM_OPT_MODE][8] = {"USB","LSB","AM","CW"};			// Indexed by band_vars[ui_band][ui_S_MODE]  MODE_USB=0 MODE_LSB=1  MODE_AM=2  MODE_CW=3
char ui_o_agc [UI_NUM_OPT_AGC][8] = {"NoAGC","Slow","Fast"};					// Indexed by band_vars[ui_band][ui_S_AGC]
char ui_o_pre [UI_NUM_OPT_PRE][8] = {"-30dB","-20dB","-10dB","0dB","+10dB"};	// Indexed by band_vars[ui_band][ui_S_PRE]
char ui_o_vox [UI_NUM_OPT_VOX][8] = {"NoVOX","VOX-L","VOX-M","VOX-H"};		// Indexed by band_vars[ui_band][ui_S_VOX]
#define NoVOX_pos_menu  0   //index for NoVOX option
char ui_o_bpf [UI_NUM_OPT_BPF][8] = {"<2.5","2-6","5-12","10-24","20-40"};
char ui_o_dflash [UI_NUM_OPT_DFLASH][8] = {"Save", "Saving"};  //only save is visible  (saving is used to start the dflash write)
char ui_o_audio [UI_NUM_OPT_AUDIO][20] = {"Rec from TX", "Rec from RX", "Play to TX", "Play to Speaker"};

//const uint8_t  ui_num_opt[ui_NMENUS] = { ui_NUM_OPT_TUNE, ui_NUM_OPT_MODE, ui_NUM_OPT_AGC, ui_NUM_OPT_PRE, ui_NUM_OPT_VOX, ui_NUM_OPT_BPF, ui_NUM_OPT_DFLASH, ui_NUM_OPT_AUDIO };	 // number of options for each menu


// Map option to setting
//uint8_t ui_pre[5] = {REL_ATT_30, REL_ATT_20, REL_ATT_10, REL_ATT_00, REL_PRE_10};
//uint8_t ui_bpf[5] = {REL_LPF2, REL_BPF6, REL_BPF12, REL_BPF24, REL_BPF40};

uint8_t  ui_menu;     // menu section 0=Tune/cursor 1=Mode 2=AGC 3=Pre 4=VOX 5=Band 6=Mem  (old ui_state)
uint8_t  ui_menu_opt_display;	 // current menu option showing on display (it will be copied to band vars on <enter>)  (old ui_option)
uint8_t  ui_band;     // actual band

//                              { cursor, mode, agc, pre, vox, band, mem ok }
//uint8_t  ui_sub[ui_NMENUS] = {      4,    1,   2,   3,   0,    2,      0 };							// Stored option selection per state

/*
arr[row][col]
int arr[3][5] = { {10, 0, 0, 0, 0},
                  {8, 0, 0, 0, 0},
                  {9, 0, 0, 0, 0 }};
*/
//                                                0=Tune/cursor 1=Mode 2=AGC 3=Pre 4=VOX 5=Band 6=Mem 7,8,9,10=freq
uint8_t  band_vars[UI_NUM_OPT_BPF][BAND_VARS_SIZE] =  { {4,0,2,3,0,0,0, b0_0, b0_1, b0_2, b0_3},
                                                  {4,1,2,3,0,1,0, b1_0, b1_1, b1_2, b1_3},
                                                  {4,1,2,3,0,2,0, b2_0, b2_1, b2_2, b2_3},
                                                  {4,1,2,3,0,3,0, b3_0, b3_1, b3_2, b3_3},
                                                  {4,1,2,3,0,4,0, b4_0, b4_1, b4_2, b4_3} };







uint32_t ui_freq;														// Frequency from Tune state
uint32_t ui_step[UI_NUM_OPT_TUNE] = {10000000, 1000000, 100000, 10000, 1000, 100, 50};	// Frequency digit increments (tune option = cursor position)
//#define ui_MAXFREQ		30000000
//#define ui_MINFREQ		     100
const uint32_t ui_maxfreq[UI_NUM_OPT_BPF] = {2500000, 6000000, 10489777, 24000000, 40000000};	// max freq for each band from pass band filters
const uint32_t ui_minfreq[UI_NUM_OPT_BPF] = {1000000, 2000000,  5000000, 10000000, 20000000};	  // min freq for each band from pass band filters

#ifdef PY2KLA_setup
#define ui_MULFREQ          4			// Factor between HMI and actual frequency
#else
#define ui_MULFREQ          1      // Factor between HMI and actual frequency
																		// Set to 1, 2 or 4 for certain types of mixer
#endif

															



/*
<Aud Rec TX> = Record the transmission audio
 - When menu option <Aud Rec Tx> pressed:
   Clear the audio_rec_pos
 - and then
   When put audio I/Q PWM output to transmission, store at the vet_audio

<Aud Rec RX> = Record the reception audio
 - When menu option <Aud Rec Rx> pressed:
   Clear the audio_rec_pos
 - and then
   When put audio PWM to speaker, store at the vet_audio

<Aud Play TX> = Transmit the audio in memory
 - When menu option <Aud Play Tx> pressed:
   Clear the audio_play_pos
   Set ptt_aud_active = TRUE (ptt active)
 - and then
   for each audio memory I calculate the hilbert Q = H(I)
   Put the audio memory I/Q PWM output to transmission 
 - Set ptt_aud_active = FALSE (ptt active)

<Aud Play SPK> = Play the memory audio to the speaker
 - When menu option <Aud Play SPK> pressed:
   Clear the audio_play_pos
 - and then
   Put the audio memory PWM to the speaker


*/
uint8_t audio_buf[AUDIO_BUF_MAX] = {0};  //160k bytes(memory used) * (1 / 16khz(sample freq)) = 10s
uint32_t audio_rec_pos = 0;
uint32_t audio_play_pos = 0;

#define AUDIO_TIME_AFTER  (1*(1000/LOOP_MS))      //1s * (1000/LOOP_MS) = 10
//BD #define AUDIO_TIME_MAIN   ((AUDIO_BUF_MAX/FSAMP_AUDIO)*(1000/LOOP_MS))    //160k/16k * 1000/100 = 100

bool tx_enabled = false;
bool ptt_internal_active = false;    //PTT output = true for vox, mon and mem
bool ptt_external_active = false;    //external = from mike
//these inputs will generate the tx_enabled to transmit
bool ptt_vox_active = false;	  //if vox whants to transmit
bool ptt_mon_active = false;
bool ptt_aud_active = false;


uint16_t Aud_Rec_Tx = AUDIO_STOPPED;
uint16_t Aud_Rec_Rx = AUDIO_STOPPED;
uint16_t Aud_Play_Tx = AUDIO_STOPPED;
uint16_t Aud_Play_Spk = AUDIO_STOPPED;



//***********************************************************************
//
//  Audio_Rec_Play - checks if audio menu started
//                   sets the correspondent command
//                   shows a counter on display
// 
//***********************************************************************
void Audio_Rec_Play(void)
{
  static uint16_t time_main = 0;
  static uint16_t time_after = 0;


  if(Aud_Rec_Tx == AUDIO_RUNNING)
  {
    if(ptt_external_active == false)
    {
      Aud_Rec_Tx = AUDIO_STOPPED;
    }
  }
/*
printfln("Time main=" + String(time_main) +
                "   Aud_Rec_Tx=" + String(Aud_Rec_Tx) +
                "   Aud_Rec_Rx=" + String(Aud_Rec_Rx) +
                "   Aud_Play_Tx=" + String(Aud_Play_Tx) +
                "   Aud_Play_Spk=" + String(Aud_Play_Spk) +
                "   ptt_ext_act=" + (ptt_external_active ? "true" : "false"));
*/

  if(time_main > 0)     //if counting time 10s
  {
    if( (Aud_Rec_Tx == AUDIO_STOPPED) &&   //<escape> condition  stops audio
        (Aud_Rec_Rx == AUDIO_STOPPED) &&
        (Aud_Play_Tx == AUDIO_STOPPED) &&
        (Aud_Play_Spk == AUDIO_STOPPED) )
    {
      time_main = 0;
      time_after = AUDIO_TIME_AFTER;
      ptt_aud_active = false;
    }
    else
    {
      time_main--;
      if(time_main == 0)
      {
        time_after = AUDIO_TIME_AFTER;
        ptt_aud_active = false;

        Aud_Rec_Tx = AUDIO_STOPPED;
        Aud_Rec_Rx = AUDIO_STOPPED;
        Aud_Play_Tx = AUDIO_STOPPED;
        Aud_Play_Spk = AUDIO_STOPPED;
      }
      //if <enter> on any Audio menu, draw a count down window from 0 to 10s
//BD      display_ili9341_countdown(true, (AUDIO_TIME_MAIN-time_main)/10);
    }
  }
  else if(time_after > 0)     //if time after 10s, 1s more to clear the count down window
  {
    time_after--;
    if(time_after == 0)
    {
      display_ili9341_countdown(false, 0);     //close count down window
    }
    else
    {
      //display_ili9341_countdown(true, AUDIO_TIME_MAIN-time_main);  //not necessary?  already on screen
    }
  }
  else if(Aud_Rec_Tx == AUDIO_START) 
    {
      display_ili9341_countdown(true, 0);
      if(ptt_external_active == true)
      {
        audio_rec_pos = 0;
//BD        time_main = AUDIO_TIME_MAIN;
        Aud_Rec_Tx = AUDIO_RUNNING;
      }
    }
  else if(Aud_Rec_Rx == AUDIO_START)
  {
    audio_rec_pos = 0;
//BD    time_main = AUDIO_TIME_MAIN;
    Aud_Rec_Rx = AUDIO_RUNNING;
  }
  else if(Aud_Play_Tx == AUDIO_START)
  {
    audio_play_pos = 0;
//BD    time_main = AUDIO_TIME_MAIN;
    Aud_Play_Tx = AUDIO_RUNNING;
    ptt_aud_active = true;
  }
  else if(Aud_Play_Spk == AUDIO_START)
  {
    audio_play_pos = 0;
//BD    time_main = AUDIO_TIME_MAIN;
    Aud_Play_Spk = AUDIO_RUNNING;
  }
}



//***********************************************************************
//
// get info from actual band = freq -> and store it at band_vars
// when switching back to that band, it will be at the same freq and setup
// it does not save at DFLASH, just to band_vars (it will lose the changes at power off)
// 
//***********************************************************************
void Store_Last_Band(uint8_t band)
{
//  uint16_t j;
/*    
  for(j = 0; j < ui_NMENUS; j++)
    {
      band_vars[band][j] = ui_sub[j];
    }
*/

  band_vars[band][UI_NMENUS] = (uint8_t)(ui_freq >> 24);
  band_vars[band][UI_NMENUS+1] = (uint8_t)((ui_freq >> 16)&0xff);
  band_vars[band][UI_NMENUS+2] = (uint8_t)((ui_freq >> 8)&0xff);
  band_vars[band][UI_NMENUS+3] =  (uint8_t)(ui_freq&0xff);
}


//***********************************************************************
//
// get band info from band_vars -> and set  freq
// 
//***********************************************************************
void Setup_Band(uint8_t band)
{
//  uint16_t j;
/*    
  for(j = 0; j < UI_NMENUS; j++)
    {
    if(band_vars[band][j] < ui_num_opt[j])   //checking boudaries
      {
      ui_sub[j] = band_vars[band][j];
      }
    else
      {
      ui_sub[j] = 0;    //ui_num_opt[j]-1;
      }
    }
*/
	//ui_freq = 7050000UL;							// Initial frequency
  //get freq from DFLASH band data
  ui_freq = band_vars[band][UI_NMENUS];
  ui_freq <<= 8;        
  ui_freq += band_vars[band][UI_NMENUS+1];
  ui_freq <<= 8;        
  ui_freq += band_vars[band][UI_NMENUS+2];
  ui_freq <<= 8;        
  ui_freq += band_vars[band][UI_NMENUS+3];

  if(ui_freq > ui_maxfreq[band])  //checking boudaries
    {
      ui_freq = ui_maxfreq[band];
    }
  else if(ui_freq < ui_minfreq[band])
    {
      ui_freq = ui_minfreq[band];
    }


  printf("Setup_Band   freq = %d\n",band_vars[band][UI_NMENUS]);
  printf(" %d\n" + band_vars[band][UI_NMENUS+1]);
  printf(" %d\n" + band_vars[band][UI_NMENUS+2]);
  printf(" %d\n" + band_vars[band][UI_NMENUS+2]);
  printf("   = %d\n" + ui_freq);


  //set the new band to display and freq

	ADF4360_SETFREQ(0, ui_MULFREQ*ui_freq);			// Set freq to ui_freq (MULFREQ depends on mixer type)
	ADF4360_SETPHASE(0, 1);								// Set phase to 90deg (depends on mixer type)
	
	//ptt_state = 0;
	ptt_external_active = false;
	
//BD	dsp_setmode(band_vars[band][UI_S_MODE]);  //MODE_USB=0 MODE_LSB=1  MODE_AM=2  MODE_CW=3
//BD	dsp_setvox(band_vars[band][UI_S_VOX]);
//BD	dsp_setagc(band_vars[band][UI_S_AGC]);	

//	ui_menu = UI_S_BPF;  //changing band on band menu
//  ui_band = band;
//	ui_menu_opt_display = band_vars[ui_band][ui_menu];  //get the menu option = actual band
  
}



//***********************************************************************
//
// Save the actual band variables to DFLASH 
//    on first empty mem block = last_block+1 mem position
//
//
//***********************************************************************
void Save_Actual_Band_Dflash(void)
{
//   uint8_t   data_block[DATA_BLOCK_SIZE];
//   uint16_t  i;

//   //copy data from band vars to dflash block (not all menus are saved at dflash)
//   for (i=0; i<UI_NMENUS_DFLASH; i++)
//     {
//     data_block[i] = band_vars[ui_band][i];
//     }

//   // copy freq to dflash block and save freq to band vars (after menu data)
//   data_block[UI_NMENUS_DFLASH] = band_vars[ui_band][UI_NMENUS] = (uint8_t)(ui_freq >> 24);  //uint32_t
//   data_block[UI_NMENUS_DFLASH+1] = band_vars[ui_band][UI_NMENUS+1] = (uint8_t)((ui_freq >> 16) & 0xff); 
//   data_block[UI_NMENUS_DFLASH+2] = band_vars[ui_band][UI_NMENUS+2] = (uint8_t)((ui_freq >> 8) & 0xff); 
//   data_block[UI_NMENUS_DFLASH+3] = band_vars[ui_band][UI_NMENUS+3] = (uint8_t)(ui_freq & 0xff); 


//   // write last menu configuration to data flash memory
// //  if(Dflash_write_block(&band_vars[ui_band][0]) == true)
//   if(Dflash_write_block(data_block) == true)
//   {
// #ifdef UI_debug
//       printf("\nWrite block to DFLASH = OK\n");
//       for(int ndata = 0; ndata < UI_NMENUS; ndata++)
//         {   
//         	printf(" %s \n", band_vars[ui_band][ndata]);
//         }
//   }
//   else
//   {
//       printf("\nWrite block to DFLASH = NOT OK\n");
// #endif
//   }
}




/*
 * HMI State Machine,
 * Handle event according to current state
 * Code needs to be optimized
 */
void ui_handler(uint8_t event)
{
  static uint8_t ui_menu_last = UI_S_BPF;    //last menu when <escape>, used to come back to the same menu


    if ((event==UI_PTT_ON) && (ptt_internal_active == false))  //if internal is taking the ptt control, not from mike, ignores mike
    {
      ptt_external_active = true;
    }
    else if (event==UI_PTT_OFF)   
    {
      ptt_external_active = false;
    }

	/* Special case for TUNE state */
	if (ui_menu == UI_S_TUNE)  //on main tune
	{
		if (event==UI_E_ESCAPE)										// Enter submenus
		{
			//band_vars[ui_band][ui_menu] = ui_menu_opt_display;							// Store cursor position on TUNE
			ui_menu = ui_menu_last;										// go to last menu selected before TUNE
			ui_menu_opt_display = band_vars[ui_band][ui_menu];							// Restore selection of new menu
		}
		else if (event==UI_E_INCREMENT)
		{
      //if(!gpio_get(GP_AUX_1_Escape))  //in case Escape is pressed
      if(!gpio_get(GP_AUX_0_Enter))  //in case Escape is pressed
      {
        //BD if(fft_gain<(1<<FFT_GAIN_SHIFT))
        //BD   {
        //BD     fft_gain++;
        //BD   }
      }
      else
      {
			  if (ui_freq < (ui_maxfreq[band_vars[ui_band][UI_S_BPF]] - ui_step[ui_menu_opt_display]))		// Boundary check ui_MAXFREQ
			  	ui_freq += ui_step[ui_menu_opt_display];						// Increment selected digit
      }
		}
		else if (event==UI_E_DECREMENT)
		{
      //if(!gpio_get(GP_AUX_1_Escape))  //in case Escape is pressed
      if(!gpio_get(GP_AUX_0_Enter))  //in case Escape is pressed
      {
        //BD if(fft_gain>1)
        //BD   {
        //BD     fft_gain--;
        //BD   }
      }
      else
      {
        if (ui_freq > (ui_step[ui_menu_opt_display] + ui_minfreq[band_vars[ui_band][UI_S_BPF]]))		// Boundary check ui_MINFREQ
				  ui_freq -= ui_step[ui_menu_opt_display];						// Decrement selected digit
      }
		}
		if (event==UI_E_RIGHT)
      {
			ui_menu_opt_display = (ui_menu_opt_display<(UI_NUM_OPT_TUNE-1))?ui_menu_opt_display+1:(UI_NUM_OPT_TUNE-1);					// Digit to the right
      band_vars[ui_band][UI_S_TUNE] = ui_menu_opt_display;
      }      
		if (event==UI_E_LEFT)
      {      
			ui_menu_opt_display = (ui_menu_opt_display>0)?ui_menu_opt_display-1:0;					// Digit to the left
      band_vars[ui_band][UI_S_TUNE] = ui_menu_opt_display;
      }      
	}
  else  //in submenus
  {
  	/* Submenu states */
  	switch(ui_menu)
  	{
  	case UI_S_MODE:
  		if (event==UI_E_INCREMENT)
      {
  			ui_menu_opt_display = (ui_menu_opt_display<UI_NUM_OPT_MODE-1)?ui_menu_opt_display+1:UI_NUM_OPT_MODE-1;
      }
  		else if (event==UI_E_DECREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display>0)?ui_menu_opt_display-1:0;
  		break;
  	case UI_S_AGC:
  		if (event==UI_E_INCREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display<UI_NUM_OPT_AGC-1)?ui_menu_opt_display+1:UI_NUM_OPT_AGC-1;
  		else if (event==UI_E_DECREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display>0)?ui_menu_opt_display-1:0;
  		break;
  	case UI_S_PRE:
  		if (event==UI_E_INCREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display<UI_NUM_OPT_PRE-1)?ui_menu_opt_display+1:UI_NUM_OPT_PRE-1;
  		else if (event==UI_E_DECREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display>0)?ui_menu_opt_display-1:0;
  		break;
  	case UI_S_VOX:
  		if (event==UI_E_INCREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display<UI_NUM_OPT_VOX-1)?ui_menu_opt_display+1:UI_NUM_OPT_VOX-1;
  		else if (event==UI_E_DECREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display>0)?ui_menu_opt_display-1:0;
  		break;
  	case UI_S_BPF:
  		if (event==UI_E_INCREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display<UI_NUM_OPT_BPF-1)?ui_menu_opt_display+1:UI_NUM_OPT_BPF-1;
  		else if (event==UI_E_DECREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display>0)?ui_menu_opt_display-1:0;
  		break;
  	case UI_S_DFLASH: //show only 0 position = save
      /*
  		if (event==ui_E_INCREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display<ui_NUM_OPT_DFLASH-1)?ui_menu_opt_display+1:ui_NUM_OPT_DFLASH-1;
  		else if (event==ui_E_DECREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display>0)?ui_menu_opt_display-1:0;
      */
  		break;
  	case UI_S_AUDIO:
  		if (event==UI_E_INCREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display<UI_NUM_OPT_AUDIO-1)?ui_menu_opt_display+1:UI_NUM_OPT_AUDIO-1;
  		else if (event==UI_E_DECREMENT)
  			ui_menu_opt_display = (ui_menu_opt_display>0)?ui_menu_opt_display-1:0;
  		break;
  	}
  	
  	/* General actions for all submenus */
  	if (event==UI_E_ENTER)
    {
      if(ui_menu == UI_S_AUDIO)
      {
        switch(ui_menu_opt_display)
        {
        case AUDIO_REC_TX:
          Aud_Rec_Tx = AUDIO_START;
          break;
        case AUDIO_REC_RX:
          Aud_Rec_Rx = AUDIO_START;
          break;
        case AUDIO_PLAY_TX:
          Aud_Play_Tx = AUDIO_START;
          break;
        case AUDIO_PLAY_SPK:
          Aud_Play_Spk = AUDIO_START;
          break;
        }        
      }
      else if(ui_menu == UI_S_BPF)
      {
        ui_band = ui_menu_opt_display;  //band changed
      }
      else if(ui_menu == UI_S_DFLASH)
      {
        band_vars[ui_band][ui_menu] = 1;  //saving to indicate save event
      }
      else
      {
  		  band_vars[ui_band][ui_menu] = ui_menu_opt_display;				// Store selected option	
/*
        if((ui_menu == ui_S_VOX) && (ui_menu_opt_display == NoVOX_pos_menu))  //if switching to NoVOX
        {
          gpio_set_dir(GP_PTT, false);          // PTT pin input
        }
*/
      }
  	}
  	else if (event==UI_E_ESCAPE)
  	{
      if((ui_menu == UI_S_AUDIO) &&
         ((Aud_Rec_Tx != AUDIO_STOPPED) ||   //audio active
          (Aud_Rec_Rx != AUDIO_STOPPED) ||
          (Aud_Play_Tx != AUDIO_STOPPED) ||
          (Aud_Play_Spk != AUDIO_STOPPED)))
      {
        Aud_Rec_Tx = AUDIO_STOPPED;   //stops audio
        Aud_Rec_Rx = AUDIO_STOPPED;
        Aud_Play_Tx = AUDIO_STOPPED;
        Aud_Play_Spk = AUDIO_STOPPED;
      }
      else
      {
        ui_menu_last = ui_menu;
        ui_menu = UI_S_TUNE;										// Leave submenus
        ui_menu_opt_display = band_vars[ui_band][ui_menu];							// Restore selection of new state
      }
  	}
  	else if (event==UI_E_RIGHT)
  	{
  		ui_menu = (ui_menu<UI_NMENUS-1)?(ui_menu+1):1;		// Change submenu
  		ui_menu_opt_display = band_vars[ui_band][ui_menu];							// Restore selection of new state
  	}
  	else if (event==UI_E_LEFT)
  	{
  		ui_menu = (ui_menu>1)?(ui_menu-1):UI_NMENUS-1;		// Change submenu
  		ui_menu_opt_display = band_vars[ui_band][ui_menu];							// Restore selection of new state
  	}

  }
  printf("freq %lu \n", ui_freq);
}

/*
 * GPIO IRQ callback routine
 * Sets the detected event and invokes the HMI state machine
 */
void ui_callback(uint gpio, uint32_t events)
{
	uint8_t evt=UI_E_NOEVENT;
printf("GpI %d \n",gpio);
	switch (gpio)
	{
	case GP_ENC_A:									// Encoder
		if (events&GPIO_IRQ_EDGE_FALL)
    {
#if ENCODER_DIRECTION == ENCODER_CW_A_FALL_B_HIGH
			evt = gpio_get(GP_ENC_B)?ui_E_INCREMENT:ui_E_DECREMENT;
#else
      evt = gpio_get(GP_ENC_B)?UI_E_DECREMENT:UI_E_INCREMENT;
#endif
    } 
#if ENCODER_TYPE == ENCODER_FALL_AND_RISE
    else if (events&GPIO_IRQ_EDGE_RISE)
    {  
#if ENCODER_DIRECTION == ENCODER_CW_A_FALL_B_HIGH
      evt = gpio_get(GP_ENC_B)?ui_E_DECREMENT:ui_E_INCREMENT;
#else
      evt = gpio_get(GP_ENC_B)?ui_E_INCREMENT:ui_E_DECREMENT;
#endif
    }
#endif
		break;
	case GP_AUX_0_Enter:									// Enter
		if (events&GPIO_IRQ_EDGE_FALL)
    {
			evt = UI_E_ENTER;
    }
		break;
	case GP_AUX_1_Escape:									// Escape
		if (events&GPIO_IRQ_EDGE_FALL)
    {
			evt = UI_E_ESCAPE;
    }
		break;
	case GP_AUX_2_Left:									// Previous
		if (events&GPIO_IRQ_EDGE_FALL)
    {
			evt = UI_E_LEFT;
    }
		break;
	case GP_AUX_3_Right:									// Next
		if (events&GPIO_IRQ_EDGE_FALL)
    {
			evt = UI_E_RIGHT;
    }
		break;

	default:
		return;
	}
	
	ui_handler(evt);								// Invoke state machine
}



/*
 * Initialize the User interface
 * It could take some time to read all DFLASH hmi data, so make it when display is showing title
 */
void user_interface_init0(void)
{
	// Initialize LCD and set VFO
//bd  Init_ui_data(&ui_band);  //read data from DFLASH
  Setup_Band(ui_band);
  //  menu position = Tune  and  cursor position = ui_menu_opt_display
	ui_menu = UI_S_TUNE;
	ui_menu_opt_display = band_vars[ui_band][UI_S_TUNE];  // option on Tune is the cursor position
#ifdef UI_debug
  printf("TUNE %d\n",ui_band);
  printf("ui_menu  %d\n",ui_menu);
  printf("ui_menu_opt_display %d\n",ui_menu_opt_display);
#endif	
}


/*
 * Initialize the User interface
 */
void user_interface_init(void)
{
	/*
	 * Notes on using GPIO interrupts: 
	 * The callback handles interrupts for all GPIOs with IRQ enabled.
	 * Level interrupts don't seem to work properly.
	 * For debouncing, the GPIO pins should be pulled-up and connected to gnd with 100nF.
	 * PTT has separate debouncing logic
	 */

	// Init input GPIOs
	gpio_init_mask(GP_MASK_IN);   //	Initialise multiple GPIOs (enabled I/O and set func to GPIO_FUNC_SIO)
                                //  Clear the output enable (i.e. set to input). Clear any output value.

	// Enable pull-ups on input pins
	gpio_pull_up(GP_ENC_A);
	gpio_pull_up(GP_ENC_B);
	gpio_pull_up(GP_AUX_0_Enter);
	gpio_pull_up(GP_AUX_1_Escape);
	gpio_pull_up(GP_AUX_2_Left);
	gpio_pull_up(GP_AUX_3_Right);
	//gpio_pull_up(GP_MUX);
/*	
  gpio_set_dir_in_masked(GP_MASK_IN);   //don't need,  already input by  gpio_init_mask()

for(;;)
{
      gpio_put(GP_PTT, 0);      //drive PTT low (active)
      gpio_set_dir(GP_PTT, GPIO_OUT);   // PTT output
      delay(2000);

      //gpio_put(GP_PTT, 0);      //drive PTT low (active)
      gpio_set_dir(GP_PTT, GPIO_IN);   // PTT output
      delay(2000);
}
*/


// Enable interrupt on level low
	gpio_set_irq_enabled(GP_ENC_A, GPIO_IRQ_EDGE_ALL, true);
	gpio_set_irq_enabled(GP_AUX_0_Enter, GPIO_IRQ_EDGE_ALL, true);
	gpio_set_irq_enabled(GP_AUX_1_Escape, GPIO_IRQ_EDGE_ALL, true);
	gpio_set_irq_enabled(GP_AUX_2_Left, GPIO_IRQ_EDGE_ALL, true);
	gpio_set_irq_enabled(GP_AUX_3_Right, GPIO_IRQ_EDGE_ALL, true);

	// Set callback, one for all GPIO, not sure about correctness!
	gpio_set_irq_enabled_with_callback(GP_ENC_A, GPIO_IRQ_EDGE_ALL, true, ui_callback);


}





/*
 * Redraw the display, representing current state
 * This function is called regularly from the main loop.
 */
void user_interface_evaluate(void)   //hmi loop
{
	char s[32];
//BD  int16_t rec_level;
  
  static uint8_t  band_vars_old[UI_NMENUS] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };          // Stored last option selection
  static uint32_t ui_freq_old = 0xff;
  static uint8_t ui_band_old = 0xff;
  static bool tx_enable_old = true;
  static uint8_t ui_menu_old = 0xff;
  static uint8_t ui_menu_opt_display_old = 0xff;
//BD  static int16_t agc_gain_old = 1;
//BD  static int16_t fft_gain_old = 0;

#ifdef UI_debug
  uint16_t ndata;
      for(ndata = 1; ndata < UI_NMENUS-2; ndata++)
        {   
        if(band_vars[ui_band][ndata] != band_vars_old[ndata])
           break;
        }
      if(ndata < UI_NMENUS-2)
        {
        printf("evaluate Band %d   band vars changed   \n", ui_band);
        for(ndata = 0; ndata < UI_NMENUS; ndata++)
          {   
          printf("%d \n ",band_vars_old[ndata]);
          }
        printf("  ->  ");
        for(ndata = 0; ndata < UI_NMENUS; ndata++)
          {   
          printf("%d \n",band_vars[ui_band][ndata]);
          }
        printf("\n");
        }      
#endif

  // if band_var changed (after <enter>), set parameters accordingly

  if(ui_freq_old != ui_freq)
  {
    ADF4360_SETFREQ(0, ui_MULFREQ*ui_freq);
    //freq  (from encoder)
    sprintf(s, "%5.3f", (double)ui_freq/1000.0);
    tft_writexy_plus(3, kYellow, kBlack, 2,0,2,20,(uint8_t *)s);
    //cursor (writing the freq erase the cursor)
//    tft_cursor_plus(3, kYellow, 2+(ui_menu_opt_display>4?6:ui_menu_opt_display), 0, 2, 20);
//BD    tft_cursor_plus(3, kYellow, 2+(band_vars[ui_band][ui_S_TUNE]>4?6:band_vars[ui_band][ui_S_TUNE]), 0, 2, 20);
    //BD display_fft_graf_top();  //scale freqs
    ui_freq_old = ui_freq;
  }
  if(band_vars_old[UI_S_MODE] != band_vars[ui_band][UI_S_MODE])    //mode (SSB AM CW)
  {
//BD    dsp_setmode(band_vars[ui_band][UI_S_MODE]);  //MODE_USB=0 MODE_LSB=1  MODE_AM=2  MODE_CW=3
    sprintf(s, "%s  ", ui_o_mode[band_vars[ui_band][UI_S_MODE]]);
    tft_writexy_(2, kGreen, kBlack, 0,1,(uint8_t *)s);
//BD    display_fft_graf_top();  //scale freqs, mode changes the triangle
    band_vars_old[UI_S_MODE] = band_vars[ui_band][UI_S_MODE];
  }
  if(band_vars_old[UI_S_VOX] != band_vars[ui_band][UI_S_VOX])
  {
//BD    dsp_setvox(band_vars[ui_band][UI_S_VOX]);
    band_vars_old[UI_S_VOX] = band_vars[ui_band][UI_S_VOX];
  }
  if(band_vars_old[UI_S_AGC] != band_vars[ui_band][UI_S_AGC])
  {
//BD    dsp_setagc(band_vars[ui_band][UI_S_AGC]); 
    band_vars_old[UI_S_AGC] = band_vars[ui_band][UI_S_AGC];
  }
  if(ui_band_old != ui_band)
  {
    if(ui_band_old < UI_S_BPF)  //if not the first time;
      {
      Store_Last_Band(ui_band_old);  // store data from old band (save freq to have it when back to this band)
      }
    //relay_setband(ui_band);  // = ui_band  
    sleep_ms(1);                  // I2C doesn't work without...
    Setup_Band(ui_band);  // = ui_band  get the new band data 
    ui_band_old = ui_band;  // = ui_band  

    sprintf(s, "B%d", band_vars[ui_band][UI_S_BPF]);  // ui_menu_opt_display = band_vars[ui_band][ui_S_BPF]
    tft_writexy_plus(3, kYellow, kBlack, 0,0,2,20,(uint8_t *)s);
    tft_writexy_plus(2, kBlue, kBlack, 0,0,3,18,(uint8_t *)s);

  }
  if(band_vars_old[UI_S_PRE] != band_vars[ui_band][UI_S_PRE])
  {  
//    relay_setattn(ui_pre[band_vars[ui_band][UI_S_PRE]]);
    band_vars_old[UI_S_PRE] = band_vars[ui_band][UI_S_PRE];
  }
  if(band_vars[ui_band][UI_S_DFLASH] == 1)  //mem save + enter = saving
  {
    //sprintf(s, "%s       ", ui_o_dflash[ui_menu_opt_display]);
    //tft_writexy_(1, kRed, kBlack,8,0,(uint8_t *)s);  
#ifdef UI_debug
    printf("Mem save   band %d\n",ui_band);
#endif
    band_vars[ui_band][UI_S_DFLASH] = 0;  //back to 0
    Save_Actual_Band_Dflash();

    sprintf(s, "%s       ", ui_o_dflash[ui_menu_opt_display]);
    tft_writexy_(1, kMagenta, kBlack,8,0,(uint8_t *)s);  
  }




  //T or R  (using letters instead of arrow used on original project)
  if(tx_enable_old != tx_enabled)
  {
    if(tx_enabled == true)
    {
      sprintf(s, "T   ");
      tft_writexy_(2, kRed, kBlack, 0,2,(uint8_t *)s);
    }
    else
    {
      sprintf(s, "R");
      tft_writexy_(2, kGreen, kBlack, 0,2,(uint8_t *)s);

      sprintf(s, "x");
      tft_writexy_plus(1, kGreen, kBlack, 4, 9, 3, 5, (uint8_t *)s);
    }
    //BD agc_gain_old = agc_gain+1;

    tx_enable_old = tx_enabled;
  }

  
   
  //Smeter rec level
  if(tx_enabled == false)
  {
//BD     if(agc_gain_old != agc_gain)
//BD     {
//BD       rec_level = AGC_GAIN_MAX - agc_gain;
//BD       sprintf(s, "%d", rec_level);
//BD      tft_writexy_(2, kGreen, kBlack, 1,2,(uint8_t *)s);
//BD       agc_gain_old = agc_gain;
//BD     }
    
//BD     if(fft_gain_old != fft_gain)
//BD     {
//BD       sprintf(s, "%d  ",fft_gain);
//BD       s[3]=0;
//BD      tft_writexy_plus(1, kGreen, kBlack, 5, 9, 3, 5, (uint8_t *)s);   
//BD       fft_gain_old = fft_gain;
//BD     }       
  }


  // if menu changed, print new value

  if((ui_menu_old != ui_menu) || (ui_menu_opt_display_old != ui_menu_opt_display))
  {
#ifdef UI_debug
  printf("evaluate Band %d \n",ui_band);
  printf("ui_menu %d -> %d  \n",ui_menu_old, ui_menu);
  printf("ui_menu_opt_display %d -> %d \n", ui_menu_opt_display_old, ui_menu_opt_display);
#endif

  	//menu 
  	switch (ui_menu)
  	{
  	case UI_S_TUNE:
      //BD sprintf(s, "%s   %s   %s        ", ui_o_vox[band_vars[ui_band][UI_S_VOX]], ui_o_agc[band_vars[ui_band][UI_S_AGC]], ui_o_pre[band_vars[ui_band][UI_S_PRE]]);
	  sprintf(s, "HEBE %s  ", ui_o_pre[band_vars[ui_band][UI_S_PRE]]);
      tft_writexy_(1, kBlue, kBlack,0,0,(uint8_t *)s);  
      //cursor
//BD      tft_cursor_plus(3, kYellow, 2+(ui_menu_opt_display>4?6:ui_menu_opt_display), 0, 2, 20);    
  		break;
  	case UI_S_MODE:
  		sprintf(s, "Set Mode: %s        ", ui_o_mode[ui_menu_opt_display]);
      tft_writexy_(1, kMagenta, kBlack,0,0,(uint8_t *)s);  
  		break;
  	case UI_S_AGC:
  		sprintf(s, "Set AGC: %s        ", ui_o_agc[ui_menu_opt_display]);
      tft_writexy_(1, kMagenta, kBlack,0,0,(uint8_t *)s);  
  		break;
  	case UI_S_PRE:
  		sprintf(s, "Set Pre: %s        ", ui_o_pre[ui_menu_opt_display]);
      tft_writexy_(1, kMagenta, kBlack,0,0,(uint8_t *)s);  
  		break;
  	case UI_S_VOX:
  		sprintf(s, "Set VOX: %s        ", ui_o_vox[ui_menu_opt_display]);
      tft_writexy_(1, kMagenta, kBlack,0,0,(uint8_t *)s);  
  		break;
  	case UI_S_BPF:
  		sprintf(s, "Band: B%d %sMHz    ", ui_menu_opt_display, ui_o_bpf[ui_menu_opt_display]);
      tft_writexy_(1, kMagenta, kBlack,0,0,(uint8_t *)s);  
  		break;
  	case UI_S_DFLASH:
  		sprintf(s, "Memory: %s         ", ui_o_dflash[ui_menu_opt_display]);
      tft_writexy_(1, kMagenta, kBlack,0,0,(uint8_t *)s);  
  		break;
  	case UI_S_AUDIO:
//BD  		sprintf(s, "Audio: %s          ", ui_o_audio[ui_menu_opt_display]);
      tft_writexy_(1, kMagenta, kBlack,0,0,(uint8_t *)s);  
  		break;
  	default:
  		break;
  	}
   
    ui_menu_old = ui_menu;
    ui_menu_opt_display_old = ui_menu_opt_display;

  } 




  if (tx_enabled == false)  //waterfall only during RX
  {
    //BD if (fft_display_graf_new == 1)    //design a new graphic only when a new line is ready from FFT
    //BD {
    //BD   //plot waterfall graphic     
    //BD   display_fft_graf();  // warefall 110ms

    //BD   fft_display_graf_new = 0;  
    //BD   fft_samples_ready = 2;  //ready to start new sample collect
    //BD }
  }


 
//BD   if (aud_samples_state == AUD_STATE_SAMP_RDY) //design a new graphic only when data is ready
//BD   {
//BD     //plot audio graphic     
//BD     display_aud_graf();

//BD     aud_samples_state = AUD_STATE_SAMP_IN;  
//BD   }



  Audio_Rec_Play();  //check for audio rec play function
}


