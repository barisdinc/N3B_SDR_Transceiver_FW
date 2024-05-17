/*
 * debug_interface.c
 *
 * Created: May 2024
 * Author: Baris Dinc OH2UDS/TA7W
 * 
 * Main debug  output for device status monitoring and 
 * provides a CLI (Command Line Interface) for user interaction
 * Command shell on stdin/stdout.
 * Collects characters and parses commandstring.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/bootrom.h"
#include "display_ili9341.h"
//#include "dsp.h"
#include "debug_interface.h"

// Some special character ASCII codes
#define CR			13
#define LF			10
#define SP			32

#define CMD_LEN		80
#define CMD_ARGS	16

char debug_cli_cmd[CMD_LEN+1];							// Command string buffer
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
void debug_interface_init()
{
    stdio_init_all();								// Initialize Standard IO
	debug_cli_cmd[CMD_LEN] = '\0';						// Termination to be sure
	printf("\n");
	printf("======================\n");
	printf(" N3B QO-100 Terminal  \n");
	printf("                      \n");
	printf("       Baris DINC     \n");
	printf("       OH2UDS/TA7W    \n");
	printf("           2024       \n");
	printf("======================\n");
	printf("N3B> ");								// prompt
}



/*** ------------------------------------------------------------- ***/
/*** Below the definitions of the shell commands, add where needed ***/
/*** ------------------------------------------------------------- ***/

/*
 * Reboots the Pico as a USB mass storage device, ready to be programmed
 */
void debug_interface_flash(void)
{
	reset_usb_boot(1<<PICO_DEFAULT_LED_PIN,0);
}


/* 
 * Dumps the entire built-in and programmed characterset on the LCD 
 */
void debug_interface_display_test(void)
{
	printf("Checking Display...");
	display_ili9341_test();
	printf("\n");
}


/* 
 * Checks for overruns 
 */
//extern volatile uint32_t dsp_overrun;
//extern volatile uint32_t dsp_tickx;
//extern volatile int scale0;
//extern volatile int scale1;
void debug_interface_overrruns(void)
{
	//printf("DSP overruns   : %d\n", dsp_overrun);
	//printf("DSP loop load  : %lu%%\n", (100*dsp_tickx)/512);	
	//printf("FFT scale = %d, iFFT scale = %d\n", scale0, scale1);	
}


/* 
 * ADC and AGC levels 
 */
//extern volatile int32_t  rx_agc;
//extern volatile int adccnt;
void debug_interface_adc(void)
{
	// Print results
	//printf("RSSI: %5u\n", s_rssi);
	//printf("AGC : %5d\n", rx_agc);
	//printf("ADCc: %5d\n", adccnt);
}



/*
 * Command shell table, organize the command functions above
 */
#define NCMD	4
shell_t shell[NCMD]=
{
	{"flash", 5, &debug_interface_flash, "flash", "Reboots into USB bootloader mode"},
	//{"adf",  2, &debug_interface_adf,  "si <start> <nr of reg>", "Dumps Si5351 registers"},
	//{"vfo", 3, &debug_interface_vfo, "vfo <id>", "Dumps vfo[id] registers"},
	{"ilitest",  7, &debug_interface_display_test,  "ilitest (no parameters)", "ILI 9341 LCD test, dumps characterset on LCD"},
	{"or",  2, &debug_interface_overrruns,  "or (no parameters)", "Returns overrun information"},
	//{"pt",  2, &debug_interface_pt,  "pt (no parameters)", "Toggles PTT status"},
	//{"bp",  2, &debug_interface_bp,  "bp {r|w} <value>", "Read or Write BPF relays"},
	//{"rx",  2, &debug_interface_rx,  "rx {r|w} <value>", "Read or Write RX relays"},
	{"adc", 3, &debug_interface_adc, "adc (no parameters)", "Dump latest ADC readouts"}
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
void debug_interface_parse(char* s)
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
 * Debug interface evaluation process 
 * This function collects characters from stdin until CR
 * Then the command is send to a parser and executed.
 */
void debug_interface_evaluate(void)
{
	static int i = 0;
	int c = getchar_timeout_us(10L);				// NOTE: this is the only SDK way to read from stdin
	if (c==PICO_ERROR_TIMEOUT) return;				// Early bail out
	
	switch (c)
	{
	case CR:										// CR : need to parse command string
		putchar('\n');								// Echo character, assume terminal appends CR
		debug_cli_cmd[i] = '\0';							// Terminate command string		
		if (i>0)									// something to parse?
			debug_interface_parse(debug_cli_cmd);						// --> process command
		i=0;										// reset index
		printf("N3B> ");							// prompt
		break;
	case LF:
		break;										// Ignore, assume CR as terminator
	default:
		if ((c<32)||(c>=128)) break;				// Only allow alfanumeric
		putchar((char)c);							// Echo character
		debug_cli_cmd[i] = (char)c;						// store in command string
		if (i<CMD_LEN) i++;							// check range and increment
		break;
	}
}
