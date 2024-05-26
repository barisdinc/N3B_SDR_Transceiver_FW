#ifndef __N3B_H__
#define __N3B_H__

#ifdef __cplusplus
extern "C" {
#endif

//choose the serial to be used
//#define Serialx   Serial1    //USB virtual serial  /dev/ttyACM0
#define Serialx   Serial   //UART0  /dev/ttyUSB0

#define LOOP_MS    100  //100 miliseconds

#define RX_IF_FREQ  9750000

void N3B_info(void);
void N3B_setup(void);
void N3B_loop(void);




#ifdef __cplusplus
}
#endif
#endif
