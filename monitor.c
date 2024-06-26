/*
 * monitor.c
 *
 * Created: Mar 2021
 * Author: Arjan te Marvelde
 * 
 * Command shell on stdin/stdout.
 * Collects characters and parses commandstring.
 * Additional commands can easily be added.
 */ 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico.h"
#include "pico/bootrom.h"
#include <hardware/i2c.h>

#include "n3b_rx_main.h"
#include "dsp.h"
#include "monitor.h"
#include "adf4360.h"


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
	printf("====================\n");
	printf(" N3B QO-100         \n");
	printf("  & VHF/UHF/SHF     \n");
	printf("  Full Duplex SDR   \n");
	printf("    Transceiver     \n");
	printf("        2024        \n");
	printf("====================\n");
	printf("N3B_RX> ");								// prompt
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
 * Dumps a defined range of Si5351 registers 
 */
uint8_t si5351_reg[200];
void mon_si(void)
{
	int base=0, nreg=0, i;

	if (nargs>2) 
	{
		base = atoi(argv[1]);
		nreg = atoi(argv[2]);
	}
	if ((base<0)||(base+nreg>200)) return;

	for (i=0; i<200; i++) si5351_reg[i] = 0xaa;
	// si_getreg(si5351_reg, (uint8_t)base, (uint8_t)nreg);
	for (i=0; i<nreg; i++) printf("%03d : %02x \n", base+i, (int)(si5351_reg[i]));
	printf("\n");
}

/* 
 * Dumps the VFO registers 
 */
vfo_t m_vfo;
void mon_vfo(void)
{
	int i;

	if (nargs>1) 
		i = atoi(argv[1]);
	if ((i<0)||(i>1)) return;

	// si_getvfo(i, &m_vfo);														// Get local copy
	// printf("Frequency: %lu\n", m_vfo.freq);
	// printf("Phase    : %u\n", (int)(m_vfo.phase));
	// printf("Ri       : %lu\n", (int)(m_vfo.ri));
	// printf("MSi      : %lu\n", (int)(m_vfo.msi));
	// printf("MSN      : %g\n\n", m_vfo.msn);
}


/* 
 * Dumps the entire built-in and programmed characterset on the display 
 */
void mon_lt(void)
{
	printf("Check ili9341...");
	// ili_test();
	printf("\n");
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




static const uint I2C_SLAVE_ADDRESS = 0x17;
static const uint I2C_BAUDRATE = 100000; // 100 kHz

static void mon_txdata() {
    gpio_init(MASTER_I2C0_SDA);
    gpio_set_function(MASTER_I2C0_SDA, GPIO_FUNC_I2C);
    // pull-ups are already active on slave side, this is just a fail-safe in case the wiring is faulty
    gpio_pull_up(MASTER_I2C0_SDA);

    gpio_init(MASTER_I2C0_SCL);
    gpio_set_function(MASTER_I2C0_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(MASTER_I2C0_SCL);

    i2c_init(i2c0, I2C_BAUDRATE);

    // for (uint8_t mem_address = 0;; mem_address = (mem_address + 32) % 256) {
    //     char msg[32];
    //     snprintf(msg, sizeof(msg), "Hello, I2C slave! - 0x%02X", mem_address);
    //     uint8_t msg_len = strlen(msg);

        uint8_t buf[32];
    //     buf[0] = mem_address;
    //     memcpy(buf + 1, msg, msg_len);
    //     // write message at mem_address
    //     printf("Write at 0x%02X: '%s'\n", mem_address, msg);
    //     int count = i2c_write_blocking(i2c0, I2C_SLAVE_ADDRESS, buf, 1 + msg_len, false);
    //     if (count < 0) {
    //         puts("Couldn't write to slave, please check your wiring!");
    //         return;
    //     }
    //     hard_assert(count == 1 + msg_len);

    //     // seek to mem_address
    //     count = i2c_write_blocking(i2c0, I2C_SLAVE_ADDRESS, buf, 1, true);
    //     hard_assert(count == 1);
    //     // partial read
    //     uint8_t split = 5;
    //     count = i2c_read_blocking(i2c0, I2C_SLAVE_ADDRESS, buf, split, true);
    //     hard_assert(count == split);
    //     buf[count] = '\0';
    //     printf("Read  at 0x%02X: '%s'\n", mem_address, buf);
    //     hard_assert(memcmp(buf, msg, split) == 0);
    //     // read the remaining bytes, continuing from last address
    //     count = i2c_read_blocking(i2c0, I2C_SLAVE_ADDRESS, buf, msg_len - split, false);
    //     hard_assert(count == msg_len - split);
    //     buf[count] = '\0';
    //     printf("Read  at 0x%02X: '%s'\n", mem_address + split, buf);
    //     hard_assert(memcmp(buf, msg + split, msg_len - split) == 0);

    //     puts("");
    //     sleep_ms(2000);
    // }
    static struct
{
    uint8_t last_requested_mem_addr;
    uint32_t frequency;
    bool ptt_state;
    bool tx_btn1;
    bool tx_btn2;
    bool tx_btn3;
    bool tx_btn4;
    // uint8_t tx_dat1;
    // uint8_t tx_dat2;
    // uint8_t tx_dat3;
    // uint8_t tx_dat4;
} tx_data;
printf("size = %d\n", sizeof(tx_data));
    // for(int ii=0;ii<10;ii++)
    {
    int count = i2c_read_blocking(i2c0, I2C_SLAVE_ADDRESS, buf, sizeof(tx_data), false);
	for (int yy = 0; yy<sizeof(tx_data); yy++)
		{
			printf("c=%d %d %d\n",count, yy, buf[yy]);
		}
    }

}



/*
 * Command shell table, organize the command functions above
 */
#define NCMD	6
shell_t shell[NCMD]=
{
	{"flash", 5, &mon_flash, "flash", "Reboots into USB bootloader mode"},
	{"vfo", 3, &mon_vfo, "vfo <id>", "Dumps vfo[id] registers"},
	{"lt",  2, &mon_lt,  "lt (no parameters)", "Ili test, dumps characterset on display"},
	{"or",  2, &mon_or,  "or (no parameters)", "Returns overrun information"},
	{"pt",  2, &mon_pt,  "pt (no parameters)", "Toggles PTT status"},
	{"txd", 3, &mon_txdata, "txd query TX Data", "read data from TX mcu"}
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
		printf("N3B_RX> ");							// prompt
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
