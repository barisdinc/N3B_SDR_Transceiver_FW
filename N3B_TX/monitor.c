#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/bootrom.h"

#include "main_n3b_tx.h"
#include "dsp.h"
#include "monitor.h"


// Some special character ASCII codes
#define CR			13
#define LF			10
#define SP			32

#define CMD_LEN		80
#define CMD_ARGS	16


char mon_cmd[CMD_LEN+1];							// Command string buffer
char *argv[CMD_ARGS];								// Argument pointers
int nargs;											// Nr of arguments

typedef struct 
{
	char *cmdstr;									// Command string
	int   cmdlen;									// Command string length
	void (*cmd)(void);								// Command executive
	char *cmdsyn;									// Command syntax
	char *help;										// Command help text
} shell_t;




/*** Initialisation, called at startup ***/
void mon_init()
{
    stdio_init_all();								// Initialize Standard IO
	mon_cmd[CMD_LEN] = '\0';						// Termination to be sure
	printf("\n");
	printf("=============\n");
	printf(" uSDR-Pico   \n");
	printf("  PE1ATM     \n");
	printf(" 2021, Udjat \n");
	printf("=============\n");
	printf("Pico> ");								// prompt
}



/*** ------------------------------------------------------------- ***/
/*** Below the definitions of the shell commands, add where needed ***/
/*** ------------------------------------------------------------- ***/

/*
 * Reboots the Pico as a USB mass storage device, ready to be programmed
 */
void mon_flash(void)
{
	reset_usb_boot(1<<PICO_DEFAULT_LED_PIN,0);
}


/*
 * Toggles the PTT status, overriding the HW signal
 */
bool ptt = false;
void mon_pt(void)
{
	if (ptt)
	{
		ptt = false;
		printf("PTT released\n");
	}
	else
	{
		ptt = true;
		printf("PTT active\n");
	}
	tx_enabled = ptt;
}

/* 
 * Checks for overruns 
 */
extern volatile uint32_t dsp_overrun;
#if DSP_FFT == 1
extern volatile uint32_t dsp_tickx;
extern volatile int scale0;
extern volatile int scale1;
#endif
void mon_or(void)
{
	printf("DSP overruns   : %d\n", dsp_overrun);
#if DSP_FFT == 1
	printf("DSP loop load  : %lu%%\n", (100*dsp_tickx)/512);	
	printf("FFT scale = %d, iFFT scale = %d\n", scale0, scale1);	
#endif
}


/* 
 * ADC and AGC levels 
 */
extern volatile int32_t  rx_agc;
extern volatile int adccnt;
void mon_adc(void)
{
	// Print results
	printf("RSSI: %5u\n", s_rssi);
	printf("AGC : %5d\n", rx_agc);
	printf("ADCc: %5d\n", adccnt);
}



/*
 * Command shell table, organize the command functions above
 */
#define NCMD	4
shell_t shell[NCMD]=
{
	{"flash", 5, &mon_flash, "flash", "Reboots into USB bootloader mode"},
	{"or",  2, &mon_or,  "or (no parameters)", "Returns overrun information"},
	{"pt",  2, &mon_pt,  "pt (no parameters)", "Toggles PTT status"},
	{"adc", 3, &mon_adc, "adc (no parameters)", "Dump latest ADC readouts"}
};



/*** ---------------------------------------- ***/
/*** Commandstring parser and monitor process ***/
/*** ---------------------------------------- ***/

#define ISALPHANUM(c)	(((c)>' ') && ((c)<127))
#define ISWHITESP(c)	(((c)!='\0') && ((c)<=' '))
#define ISEOL(c)        ((c)=='\0')
/*
 * Command line parser
 */
void mon_parse(char* s)
{
	char *p;
	int  i;

	p = s;											// Set to start of string
	nargs = 0;
	while (ISWHITESP(*p)) p++;						// Skip leading whitespace
	while (!ISEOL(*p))								// Check remaining stringlength >0 
	{
		argv[nargs++] = p;							// Store first valid char loc after whitespace
		while (ISALPHANUM(*p)) p++;					// Skip non-whitespace
		while (ISWHITESP(*p)) p++;					// Skip separating whitespace
	}
	if (nargs==0) return;							// Nothing to do
	
	for (i=0; i<NCMD; i++)							// Lookup shell command
		if (strncmp(argv[0], shell[i].cmdstr, shell[i].cmdlen) == 0) break;
	if (i<NCMD)
		(*shell[i].cmd)();							// Execute if found
	else											// Unknown command
	{
		for (i=0; i<NCMD; i++)						// Print help if no match
			printf("%s\n   %s\n", shell[i].cmdsyn, shell[i].help);
	}
}

/*
 * Monitor process 
 * This function collects characters from stdin until CR
 * Then the command is send to a parser and executed.
 */
void mon_evaluate(void)
{
	static int i = 0;
	int c = getchar_timeout_us(10L);				// NOTE: this is the only SDK way to read from stdin
	if (c==PICO_ERROR_TIMEOUT) return;				// Early bail out
	
	switch (c)
	{
	case CR:										// CR : need to parse command string
		putchar('\n');								// Echo character, assume terminal appends CR
		mon_cmd[i] = '\0';							// Terminate command string		
		if (i>0)									// something to parse?
			mon_parse(mon_cmd);						// --> process command
		i=0;										// reset index
		printf("Pico> ");							// prompt
		break;
	case LF:
		break;										// Ignore, assume CR as terminator
	default:
		if ((c<32)||(c>=128)) break;				// Only allow alfanumeric
		putchar((char)c);							// Echo character
		mon_cmd[i] = (char)c;						// store in command string
		if (i<CMD_LEN) i++;							// check range and increment
		break;
	}
}
