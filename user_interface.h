#ifndef __USER_INTERFACE_H__
#define __USER_INTERFACE_H__
/* 
 * user_interface.h
 *
 * Created: May 2024
 * Author: Baris DINC OH2UDS/TA7W
 *
 */

/* Menu definitions (band vars array position) */
#define UI_S_TUNE			0
#define UI_S_MODE			1
#define UI_S_AGC			2
#define UI_S_PRE			3
#define UI_S_VOX			4
#define UI_S_BPF			5
#define UI_S_DFLASH   6
#define UI_S_AUDIO   7
#define UI_NMENUS			8  //number of possible menus
#define UI_NMENUS_DFLASH			7   //number of menus saved on DFLASH (only the first ones from the array of menus are saved on dflash)
                                  //(ui_NMENUS_DFLASH + 4) must be less than DFLASH DATA_BLOCK_SIZE (16)

/* Event definitions */
#define UI_E_NOEVENT		0
#define UI_E_INCREMENT		1
#define UI_E_DECREMENT		2
#define UI_E_ENTER			3
#define UI_E_ESCAPE		4
#define UI_E_LEFT			5
#define UI_E_RIGHT			6
#define UI_E_PTTON			7
#define UI_E_PTTOFF		8
#define UI_PTT_ON      9
#define UI_PTT_OFF     10
#define UI_E_ENTER_RELEASE			11
#define UI_NEVENTS			12  //number of events


/* Sub menu number of options (string sets) */
#define UI_NUM_OPT_TUNE	7  // = num pos cursor
#define UI_NUM_OPT_MODE	4
#define UI_NUM_OPT_AGC	3
#define UI_NUM_OPT_PRE	5
#define UI_NUM_OPT_VOX	4
#define UI_NUM_OPT_BPF	5
#define UI_NUM_OPT_DFLASH	2
#define UI_NUM_OPT_AUDIO 4


//"USB","LSB","AM","CW"
#define MODE_USB  0
#define MODE_LSB  1
#define MODE_AM   2
#define MODE_CW   3


// ui_o_audio "Rec from TX", "Rec from RX", "Play to TX", "Play to Speaker"
#define AUDIO_REC_TX   0
#define AUDIO_REC_RX   1
#define AUDIO_PLAY_TX   2
#define AUDIO_PLAY_SPK   3



#define BAND_VARS_SIZE   (UI_NMENUS + 4)   //menus + frequency
#define BAND_VARS_SIZE_DFLASH   (UI_NMENUS_DFLASH + 4)   //menus + frequency saved on dflash
//#define BAND_INDEX   ui_S_BPF    // = 5
extern uint8_t  band_vars[UI_NUM_OPT_BPF][BAND_VARS_SIZE];



/*
 * Some macros
 */
#ifndef MIN
#define MIN(x, y)        ((x)<(y)?(x):(y))  // Get min value
#endif
#ifndef MAX
#define MAX(x, y)        ((x)>(y)?(x):(y))  // Get max value
#endif



#define  band0_ui_freq_default     1820000L
#define  band1_ui_freq_default     3700000L
#define  band2_ui_freq_default     7050000L
#define  band3_ui_freq_default    14200000L
#define  band4_ui_freq_default    28400000L

#define b0_0 (uint8_t)(band0_ui_freq_default >> 24)
#define b0_1 (uint8_t)((band0_ui_freq_default >> 16)&0xff)
#define b0_2 (uint8_t)((band0_ui_freq_default >> 8)&0xff)
#define b0_3 (uint8_t)(band0_ui_freq_default&0xff)

#define b1_0 (uint8_t)(band1_ui_freq_default >> 24)
#define b1_1 (uint8_t)((band1_ui_freq_default >> 16)&0xff)
#define b1_2 (uint8_t)((band1_ui_freq_default >> 8)&0xff)
#define b1_3 (uint8_t)(band1_ui_freq_default&0xff)

#define b2_0 (uint8_t)(band2_ui_freq_default >> 24)
#define b2_1 (uint8_t)((band2_ui_freq_default >> 16)&0xff)
#define b2_2 (uint8_t)((band2_ui_freq_default >> 8)&0xff)
#define b2_3 (uint8_t)(band2_ui_freq_default&0xff)

#define b3_0 (uint8_t)(band3_ui_freq_default >> 24)
#define b3_1 (uint8_t)((band3_ui_freq_default >> 16)&0xff)
#define b3_2 (uint8_t)((band3_ui_freq_default >> 8)&0xff)
#define b3_3 (uint8_t)(band3_ui_freq_default&0xff)

#define b4_0 (uint8_t)(band4_ui_freq_default >> 24)
#define b4_1 (uint8_t)((band4_ui_freq_default >> 16)&0xff)
#define b4_2 (uint8_t)((band4_ui_freq_default >> 8)&0xff)
#define b4_3 (uint8_t)(band4_ui_freq_default&0xff)


//extern uint8_t  ui_sub[ui_NMENUS];							// Stored option selection per state
extern uint32_t ui_freq;  
extern uint8_t  ui_band;	
extern bool tx_enabled;
extern bool ptt_internal_active;    //PTT output = true for vox, mon and mem
extern bool ptt_external_active;
extern bool ptt_vox_active;	
extern bool ptt_mon_active;
extern bool ptt_aud_active;



#define AUDIO_BUF_MAX    160000  //160k bytes(memory used) * (1 / 16khz(sample freq)) = 10s
extern uint8_t audio_buf[AUDIO_BUF_MAX];
extern uint32_t audio_rec_pos;
extern uint32_t audio_play_pos;

#define AUDIO_STOPPED   0
#define AUDIO_START     1
#define AUDIO_RUNNING   2

extern uint16_t Aud_Rec_Tx;
extern uint16_t Aud_Rec_Rx;
extern uint16_t Aud_Play_Tx;
extern uint16_t Aud_Play_Spk;


void Setup_Band(uint8_t band);
void ui_init0(void);

void user_interface_init(void);
void user_interface_init0(void);
void user_interface_evaluate(void);

#endif
