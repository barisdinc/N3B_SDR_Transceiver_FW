#include "pico/stdlib.h"
#include <stdio.h>

#include "pico/multicore.h"
#include "pico/time.h"

#include "receiver.h"
// #include "ui.h"
// #include "waterfall.h"
// #include "adf4360.h"

// #define UI_REFRESH_HZ (10UL)
// #define UI_REFRESH_US (1000000UL / UI_REFRESH_HZ)

// uint8_t spectrum[256];
// uint8_t dB10=10;
static rx_settings settings_to_apply;
static rx_status status;
static receiver rx(settings_to_apply, status);
// waterfall waterfall_inst;
// static ui user_interface(settings_to_apply, status, receiver, spectrum, dB10, waterfall_inst);

void core1_main()
{
    multicore_lockout_victim_init();
    for (uint8_t cnt=0;cnt<50;cnt++)
    {
      printf("Core 2 started \r\n");
      sleep_ms(100);
    }
    rx.run();
}

int main() 
{
  stdio_init_all();
  multicore_launch_core1(core1_main);
  stdio_init_all();
  for (uint8_t cnt=0;cnt<50;cnt++)
  {
    printf("Core 1 started \r\n");
    sleep_ms(100);
  }
  // create an alarm pool for USB streaming with highest priority (0), so
  // that it can pre-empt the default pool
  // receiver.set_alarm_pool(alarm_pool_create(0, 16));
  // user_interface.autorestore();
  // uint32_t last_ui_update = 0;
  // uint32_t last_cat_update = 0;

  // adf4360_init();

  while(1)
  {
    //schedule tasks
  // printf(".");

  //   if(time_us_32() - last_ui_update > UI_REFRESH_US)
  //   {
  //     last_ui_update = time_us_32();
  //     user_interface.do_ui();
  //     receiver.get_spectrum(spectrum, dB10);
  //   }

  //   else if(time_us_32() - last_cat_update > CAT_REFRESH_US)
  //   {
  //     process_cat_control(settings_to_apply, status, receiver, user_interface.get_settings());
  //   }

  //   waterfall_inst.update_spectrum(receiver, settings_to_apply, status, spectrum, dB10);

  }
}
