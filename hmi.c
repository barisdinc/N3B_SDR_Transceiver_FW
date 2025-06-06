/*
 * hmi.c
 *
 * Created: Apr 2021
 * Author: Arjan te Marvelde
 * 
 * This file contains the HMI driver, processing user inputs.
 * It will also do the logic behind these, and write feedback to the display.
 *
 * The 4 auxiliary buttons have the following functions:
 * GP6 - Enter, confirm : Used to select menu items or make choices from a list
 * GP7 - Escape, cancel : Used to exit a (sub)menu or cancel the current action
 * GP8 - Left           : Used to move left, e.g. to select a digit
 * GP9 - Right			: Used to move right, e.g. to select a digit
 *
 * The rotary encoder (GP2, GP3) controls an up/down counter connected to some field. 
 * It may be that the encoder has a bushbutton as well, this can be connected to GP4.
 *     ___     ___
 * ___|   |___|   |___  A
 *   ___     ___     _
 * _|   |___|   |___|   B
 *
 * Encoder channel A triggers on falling edge. 
 * Depending on B level, count is incremented or decremented.
 * 
 * The PTT is connected to GP15 and will be active, except when VOX is used.
 *
 */
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"

#include "n3b_rx_main.h"
#include "ili9341.h"
#include "adf4360.h"
#include "hmi.h"
#include "dsp.h"

static int old_sdr_freq = 0;
static uint16_t waterfall_buffer[300][40];
static uint8_t waterfall_active_row = 0;	  
	  

/*
 * GPIO masks
 */
#define GP_MASK_IN	((1<<GP_ENC_A)|(1<<GP_ENC_B)|(1<<GP_AUX_0)|(1<<GP_AUX_1)|(1<<GP_AUX_2)|(1<<GP_AUX_3)|(1<<GP_PTT))
#define GP_MASK_PTT	(1<<GP_PTT)

/*
 * Event flags
 */
#define GPIO_IRQ_ALL		(GPIO_IRQ_LEVEL_LOW|GPIO_IRQ_LEVEL_HIGH|GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE)
#define GPIO_IRQ_EDGE_ALL	(GPIO_IRQ_EDGE_FALL|GPIO_IRQ_EDGE_RISE)

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
 
/* State definitions */
#define HMI_S_TUNE			0
#define HMI_S_MODE			1
#define HMI_S_AGC			2
#define HMI_S_PRE			3
#define HMI_S_VOX			4
#define HMI_S_BPF			5
#define HMI_NSTATES			6

/* Event definitions */
#define HMI_E_NOEVENT		0
#define HMI_E_INCREMENT		1
#define HMI_E_DECREMENT		2
#define HMI_E_ENTER			3
#define HMI_E_ESCAPE		4
#define HMI_E_LEFT			5
#define HMI_E_RIGHT			6
#define HMI_E_PTTON			7
#define HMI_E_PTTOFF		8
#define HMI_NEVENTS			9

/* Sub menu option string sets */
#define HMI_NTUNE	6
#define HMI_NMODE	4
#define HMI_NAGC	3
#define HMI_NPRE	5
#define HMI_NVOX	4
#define HMI_NBPF	5
char hmi_noption[HMI_NSTATES] = {HMI_NTUNE, HMI_NMODE, HMI_NAGC, HMI_NPRE, HMI_NVOX, HMI_NBPF};
const char* hmi_o_menu[HMI_NSTATES] = {"Tune","Mode","AGC","Pre","VOX"};		// Indexed by hmi_state
const char* hmi_o_mode[HMI_NMODE] = {"USB","LSB","AM ","CW "};					// Indexed by hmi_sub[HMI_S_MODE]
const char* hmi_o_agc [HMI_NAGC] = {"NoGC","Slow","Fast"};						// Indexed by hmi_sub[HMI_S_AGC]
const char* hmi_o_pre [HMI_NPRE] = {"-30dB","-20dB","-10dB","  0dB","+10dB"};	// Indexed by hmi_sub[HMI_S_PRE]
const char* hmi_o_vox [HMI_NVOX] = {"NoVOX","VOX-L","VOX-M","VOX-H"};			// Indexed by hmi_sub[HMI_S_VOX]
const char* hmi_o_bpf [HMI_NBPF] = {"<2.5","2-6","5-12","10-24","20-40"};		// Indexed by hmi_sub[HMI_S_BPF]

// Map option to setting
int  hmi_mode[HMI_NMODE] = {MODE_USB, MODE_LSB, MODE_AM, MODE_CW};
int  hmi_agc[HMI_NAGC]   = {AGC_NONE, AGC_SLOW, AGC_FAST};
int  hmi_vox[HMI_NVOX]   = {VOX_OFF, VOX_LOW, VOX_MEDIUM, VOX_HIGH};


int  hmi_state, hmi_option;													// Current state and menu option selection
int  hmi_sub[HMI_NSTATES] = {4,0,0,3,0,2};									// Stored option selection per state
bool hmi_update;															// display needs update

uint32_t hmi_freq;															// Frequency from Tune state
uint32_t hmi_step[6] = {1000000, 100000, 10000, 1000, 100, 10};		// Frequency digit increments
#define HMI_MAXFREQ		30000000
#define HMI_MINFREQ		     100
#define HMI_MULFREQ            1											// Factor between HMI and actual frequency
																			// Set to 2 for certain types of mixer

#define PTT_DEBOUNCE	3													// Nr of cycles for debounce
int ptt_state;																// Debounce counter
bool ptt_active;															// Resulting state

/*
 * Some macros
 */
#ifndef MIN
#define MIN(x, y)        ((x)<(y)?(x):(y))  // Get min value
#endif
#ifndef MAX
#define MAX(x, y)        ((x)>(y)?(x):(y))  // Get max value
#endif


/*
 * GPIO IRQ callback routine
 * Sets the detected event and invokes the HMI state machine
 */
void hmi_callback(uint gpio, uint32_t events)
{
	uint8_t evt=HMI_E_NOEVENT;

	// Decide what the event was
	switch (gpio)
	{
	case GP_ENC_A:															// Encoder
		if (events&GPIO_IRQ_EDGE_FALL)
			evt = gpio_get(GP_ENC_B)?HMI_E_INCREMENT:HMI_E_DECREMENT;
		break;
	case GP_AUX_0:															// Enter
		if (events&GPIO_IRQ_EDGE_FALL)
			evt = HMI_E_ENTER;
		break;
	case GP_AUX_1:															// Escape
		if (events&GPIO_IRQ_EDGE_FALL)
			evt = HMI_E_ESCAPE;
		break;
	case GP_AUX_2:															// Previous
		if (events&GPIO_IRQ_EDGE_FALL)
			evt = HMI_E_LEFT;
		break;
	case GP_AUX_3:															// Next
		if (events&GPIO_IRQ_EDGE_FALL)
			evt = HMI_E_RIGHT;
		break;
	default:																// Stray...
		return;
	}
	
	/** HMI State Machine **/

	// Special case for TUNE state
	if (hmi_state == HMI_S_TUNE)
	{
		switch (evt)
		{
		case HMI_E_ENTER:													// Commit current value
			// To be defined action
			break;
		case HMI_E_ESCAPE:													// Enter submenus
			hmi_sub[hmi_state] = hmi_option;								// Store selection (i.e. digit)
			hmi_state = HMI_S_MODE;											// Should remember last one
			hmi_option = hmi_sub[hmi_state];								// Restore selection of new state
			break;
		case HMI_E_INCREMENT:
			if (hmi_freq < (HMI_MAXFREQ - hmi_step[hmi_option]))			// Boundary check
				hmi_freq += hmi_step[hmi_option];							// Increment selected digit
			break;
		case HMI_E_DECREMENT:
			if (hmi_freq > (HMI_MINFREQ + hmi_step[hmi_option]))			// Boundary check
				hmi_freq -= hmi_step[hmi_option];							// Decrement selected digit
			break;
		case HMI_E_RIGHT:
			hmi_option = (hmi_option<5)?hmi_option+1:5;						// Digit to the right
			break;
		case HMI_E_LEFT:
			hmi_option = (hmi_option>0)?hmi_option-1:0;						// Digit to the left
			break;
		}
		return;																// Early bail-out
	}
	
	// Actions for other states
	switch (evt)
	{
	case HMI_E_ENTER:
		hmi_sub[hmi_state] = hmi_option;									// Store value for selected option	
		hmi_update = true;													// Mark HMI updated: activate value
		break;
	case HMI_E_ESCAPE:
		hmi_state = HMI_S_TUNE;												// Leave submenus
		hmi_option = hmi_sub[hmi_state];									// Restore selection of new state
		break;
	case HMI_E_RIGHT:
		hmi_state = (hmi_state<HMI_NSTATES-1)?(hmi_state+1):1;				// Change submenu
		hmi_option = hmi_sub[hmi_state];									// Restore selection of new state
		break;
	case HMI_E_LEFT:
		hmi_state = (hmi_state>1)?(hmi_state-1):HMI_NSTATES-1;				// Change submenu
		hmi_option = hmi_sub[hmi_state];									// Restore selection of new state
		break;
	case HMI_E_INCREMENT:
		hmi_option = (hmi_option<hmi_noption[hmi_state]-1)?hmi_option+1:hmi_noption[hmi_state]-1;
		break;
	case HMI_E_DECREMENT:
		hmi_option = (hmi_option>0)?hmi_option-1:0;
		break;
	}	
}


#define SPECTRUM_WIDTH 300
#define SPECTRUM_HEIGHT 40



/*
 * Draw Spectrum
 * TODO: add logaritmic scale
 *       if valus is same don't draw
 * 		 delete in length of old level
 * 		 replace 300 with a define
*/


void hmi_draw_spectrum(void)
{
	// int16_t* fft_buffer = get_fft_buffer_address();

	if (waterfall_active_row == SPECTRUM_HEIGHT) waterfall_active_row = 0; //go to next buffer for net call
	uint16_t* wf_act_p = &waterfall_buffer[waterfall_active_row][0]; //target memory is the first element of active waterfall buffer

int imm = 0;
	for (int spectrum_pos = 0; spectrum_pos<SPECTRUM_WIDTH; spectrum_pos++)
	{
	//printf("%d\n",*fft_buffer++ >> 8);
	// ili9341_draw_pixel(aa,150, *fft_buffer++);
	//spectrum
//BD	ili9341_draw_line(spectrum_pos+10,100,spectrum_pos+10,160, ILI9341_BLACK);//*fft_buffer++);
	// GFX_drawFastVLine(spectrum_pos+10, 100, 100, ILI9341_BLACK);
	// uint16_t signal_strength = abs(*fft_buffer++ + *fft_buffer++ + *fft_buffer++);
	// signal_strength = signal_strength>>10;

//BD	ili9341_draw_line(spectrum_pos+10,160,spectrum_pos+10,160-signal_strength, ILI9341_GREEN);//*fft_buffer++);
	// GFX_drawFastVLine(spectrum_pos+10, 160, signal_strength, ILI9341_GREEN);
	//waterfall
	// uint16_t waterfall_color = signal_strength << 11 | signal_strength << 5 | signal_strength;

	
	// *wf_act_p = signal_strength; // waterfall_active_row * 300 + imm;//
	imm++;
	wf_act_p++;
	
//BD	ili9341_draw_pixel(spectrum_pos+10,168+waterfall_active_row, signal_strength<<10);//*fft_buffer++);
	// GFX_drawPixel(spectrum_pos+10,168+waterfall_active_row, signal_strength<<10);
	//printf("[%d] %d %x\n", waterfall_active_row, signal_strength, waterfall_color);
	}	

	waterfall_active_row++;
//BD	ILI9341_setAddrWindow(10, 190, 310, 219);
	// ILI9341_copyFrameBufferToDisplay(&waterfall_buffer[0][0], 300, 40); //redraw the  waterfall

}


/*
 * Draw bandwidth
 * TODO: add bandwidth parameter
 *       add mod type
 * 		 add Cnter frequency
*/

void hmi_draw_bandwidth(void)
{
	  int sdr_freq =  ((vfo[0].freq/100) % 30) ;
	//   ili9341_draw_line(sdr_freq*10,220,sdr_freq*10,200, ILI9341_RED);//*fft_buffer++);
//BD	  ili9341_draw_line(old_sdr_freq,167,old_sdr_freq+5,162, ILI9341_BLACK);
//BD	  ili9341_draw_line(old_sdr_freq+5,162,old_sdr_freq+50,162, ILI9341_BLACK);
//BD	  ili9341_draw_line(old_sdr_freq+50,162,old_sdr_freq+55,167, ILI9341_BLACK);

//BD	  ili9341_draw_line(sdr_freq,167,sdr_freq+5,162, ILI9341_CYAN);
//BD	  ili9341_draw_line(sdr_freq+5,162,sdr_freq+50,162, ILI9341_CYAN);
//BD	  ili9341_draw_line(sdr_freq+50,162,sdr_freq+55,167, ILI9341_CYAN);

	//   printf("%d-------------%d------------\n", sdr_freq, old_sdr_freq);
	  old_sdr_freq = sdr_freq;

}



/*
 * Redraw the  display, representing current state
 * This function is invoked regularly from the main loop.
 */
void hmi_evaluate(void)
{
	char s[32];
	
	// Print top line of display
	// GFX_setCursor(10,75);
	if (tx_enabled)
		// GFX_printf("10489,%6.2f %c %-2d", (double)hmi_freq/1000.0, 0x07, 0);
		sprintf(s, "10489,%6.2f %c %-2d", (double)hmi_freq/1000.0, 0x07, 0);
	else
		// GFX_printf("10489,%6.2f %cS%-2d", (double)hmi_freq/1000.0, 0x06, get_sval());
		sprintf(s, "10489,%6.2f %cS%-2d", (double)hmi_freq/1000.0, 0x06, get_sval());

//BD	ili9341_draw_string(10,40,s, ILI9341_YELLOW, ILI9341_BLACK,3);	

	// GFX_setCursor(10,100);
	// GFX_printf("%s", hmi_o_mode[hmi_sub[HMI_S_MODE]]);
	sprintf(s, "%s", hmi_o_mode[hmi_sub[HMI_S_MODE]]);
//BD	ili9341_draw_string(10,70,s, ILI9341_LIGHTGREY, ILI9341_BLACK,3);	

	if (is_fft_completed())
	{
		hmi_draw_spectrum();
		hmi_draw_bandwidth();
	}

	// Print bottom line of display, depending on state
	// GFX_setCursor(10,30);
	switch (hmi_state)
	{
	case HMI_S_TUNE:
		// GFX_printf("%s %s %s", hmi_o_vox[hmi_sub[HMI_S_VOX]], hmi_o_agc[hmi_sub[HMI_S_AGC]], hmi_o_pre[hmi_sub[HMI_S_PRE]]);
		sprintf(s, "%s %s %s", hmi_o_vox[hmi_sub[HMI_S_VOX]], hmi_o_agc[hmi_sub[HMI_S_AGC]], hmi_o_pre[hmi_sub[HMI_S_PRE]]);
//BD		ili9341_draw_string(10,10,s, ILI9341_YELLOW, ILI9341_BLACK,3);	
//BD		ili9341_draw_rect(105+(hmi_option>3?hmi_option*16+20:hmi_option*16), 40, 20, 20,  ILI9341_RED);
		break;
	case HMI_S_MODE:
		// GFX_printf("Set Mode: %s        ", hmi_o_mode[hmi_option]);
		sprintf(s, "Set Mode: %s        ", hmi_o_mode[hmi_option]);
//BD		ili9341_draw_string(10,10,s, ILI9341_YELLOW, ILI9341_BLACK,3);	
//BD		ili9341_draw_rect(90, 10, 20, 20, ILI9341_RED);
		break;
	case HMI_S_AGC:
		// GFX_printf("Set AGC: %s        ", hmi_o_agc[hmi_option]);
		sprintf(s, "Set AGC: %s        ", hmi_o_agc[hmi_option]);
//BD		ili9341_draw_string(10,10,s, ILI9341_YELLOW, ILI9341_BLACK,3);	
//BD		ili9341_draw_rect(80, 10, 20, 20, ILI9341_RED);
		break;
	case HMI_S_PRE:
		// GFX_printf("Set Pre: %s        ", hmi_o_pre[hmi_option]);
		sprintf(s, "Set Pre: %s        ", hmi_o_pre[hmi_option]);
//BD		ili9341_draw_string(10,10,s, ILI9341_YELLOW, ILI9341_BLACK,3);	
//BD		ili9341_draw_rect(80, 10, 20, 20, ILI9341_RED);
		break;
	case HMI_S_VOX:
		// GFX_printf(s, "Set VOX: %s        ", hmi_o_vox[hmi_option]);
		sprintf(s, "Set VOX: %s        ", hmi_o_vox[hmi_option]);
//BD		ili9341_draw_string(10,10,s, ILI9341_YELLOW, ILI9341_BLACK,3);	
//BD		ili9341_draw_rect(80, 10, 20, 20, ILI9341_RED);
		break;
	case HMI_S_BPF:
		// GFX_printf(s, "Band: %d %s        ", hmi_option, hmi_o_bpf[hmi_option]);
		sprintf(s, "Band: %d %s        ", hmi_option, hmi_o_bpf[hmi_option]);
//BD		ili9341_draw_string(10,10,s, ILI9341_YELLOW, ILI9341_BLACK,3);	
//BD		ili9341_draw_rect(80, 10, 20, 20, ILI9341_RED);
	default:
		break;
	}
	
	/* PTT debouncing */
	if (gpio_get(GP_PTT))													// Get PTT level
	{
		if (ptt_state<PTT_DEBOUNCE)											// Increment debounce counter when high
			ptt_state++;
	}
	else 
	{
		if (ptt_state>0)													// Decrement debounce counter when low
			ptt_state--;
	}
	if (ptt_state == PTT_DEBOUNCE)											// Reset PTT when debounced level high
		ptt_active = false;
	if (ptt_state == 0)														// Set PTT when debounced level low
		ptt_active = true;


	/* Set parameters corresponding to latest entered option value */
	
	// See if VFO needs update
	adf4360_evaluate(HMI_MULFREQ*(hmi_freq-FC_OFFSET));

	

	// Update peripherals according to menu setting
	// For frequency si5351 is set directly, HMI top line follows
	if (hmi_update)
	{
		dsp_setmode(hmi_sub[HMI_S_MODE]);
		dsp_setvox(hmi_sub[HMI_S_VOX]);
		dsp_setagc(hmi_sub[HMI_S_AGC]);	
		hmi_update = false;
	}
	// GFX_flush();
}

void hmi_drawBackgroundBitmap()
{
	
}


/*
 * Initialize the User interface
 */
void hmi_init(void)
{
	/*
	 * Notes on using GPIO interrupts: 
	 * The callback handles interrupts for all GPIOs with IRQ enabled.
	 * Level interrupts don't seem to work properly.
	 * For debouncing, the GPIO pins should be pulled-up and connected to gnd with 100nF.
	 * PTT has separate debouncing logic
	 */
	 
	// Init input GPIOs
	gpio_init_mask(GP_MASK_IN);
	
	// Enable pull-ups
	gpio_pull_up(GP_ENC_A);
	gpio_pull_up(GP_ENC_B);
	gpio_pull_up(GP_AUX_0);
	gpio_pull_up(GP_AUX_1);
	gpio_pull_up(GP_AUX_2);
	gpio_pull_up(GP_AUX_3);
	gpio_pull_up(GP_PTT);
	gpio_set_oeover(GP_PTT, GPIO_OVERRIDE_HIGH);							// Enable output on PTT GPIO; bidirectional
	
	// Enable interrupt on level low
	gpio_set_irq_enabled(GP_ENC_A, GPIO_IRQ_EDGE_ALL, true);
	gpio_set_irq_enabled(GP_AUX_0, GPIO_IRQ_EDGE_ALL, true);
	gpio_set_irq_enabled(GP_AUX_1, GPIO_IRQ_EDGE_ALL, true);
	gpio_set_irq_enabled(GP_AUX_2, GPIO_IRQ_EDGE_ALL, true);
	gpio_set_irq_enabled(GP_AUX_3, GPIO_IRQ_EDGE_ALL, true);
	gpio_set_irq_enabled(GP_PTT, GPIO_IRQ_EDGE_ALL, false);

	// Set callback, one for all GPIO, not sure about correctness!
	gpio_set_irq_enabled_with_callback(GP_ENC_A, GPIO_IRQ_EDGE_ALL, true, hmi_callback);
		
	// Initialize display and set VFO
	hmi_state = HMI_S_TUNE;
	hmi_option = 4;															// Active kHz digit
	hmi_freq = 823000UL;													// Initial frequency

	adf4360_evaluate(HMI_MULFREQ*(hmi_freq-FC_OFFSET));						// Set freq to 7074 kHz (depends on mixer type)
	
	ptt_state  = PTT_DEBOUNCE;
	ptt_active = false;
	
	dsp_setmode(hmi_sub[HMI_S_MODE]);
	dsp_setvox(hmi_sub[HMI_S_VOX]);
	dsp_setagc(hmi_sub[HMI_S_AGC]);	
	hmi_update = false;
}

