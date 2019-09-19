#include "retro_clock.h"
#include "lib/alphanumeric_display.h"

#include <stdio.h>

char numbers[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

void retro_clock_display_init()
{
    alphadisplay_init();
    alphadisplay_start();
}


void retro_clock_display_update(retro_clock_t *clock)
{
  if (clock ->clock_mode == RC_MODE_NOT_SET) {
    alphadisplay_write_ascii(0, '-');
    alphadisplay_write_ascii(1, '-');
    alphadisplay_write_ascii(2, '-');
    alphadisplay_write_ascii(3, '-');
    alphadisplay_set_blink(ALPHADISPLAY_BLINK_HALF_HZ);
  }
  if (clock->clock_mode == RC_MODE_CLOCK) {
    alphadisplay_set_blink(ALPHADISPLAY_BLINK_OFF);
    uint8_t hr0 = clock->clock_time.hours / 10;
    uint8_t hr1 = clock->clock_time.hours % 10;
    uint8_t min0 = clock->clock_time.minutes / 10;
    uint8_t min1 = clock->clock_time.minutes % 10;

    alphadisplay_write_ascii(0, numbers[hr0]);
    alphadisplay_write_ascii(1, numbers[hr1]);
    alphadisplay_write_ascii(2, numbers[min0]);
    alphadisplay_write_ascii(3, numbers[min1]);
}
if (clock->clock_mode == RC_MODE_SET_ALARM) {
  alphadisplay_set_blink(ALPHADISPLAY_BLINK_HALF_HZ);
  uint8_t hr0 = clock->alarm_time.hours / 10;
  uint8_t hr1 = clock->alarm_time.hours % 10;
  uint8_t min0 = clock->alarm_time.minutes / 10;
  uint8_t min1 = clock->alarm_time.minutes % 10;

  alphadisplay_write_ascii(0, numbers[hr0]);
  alphadisplay_write_ascii(1, numbers[hr1]);
  alphadisplay_write_ascii(2, numbers[min0]);
  alphadisplay_write_ascii(3, numbers[min1]);
}
if (clock->clock_mode == RC_MODE_IN_ALARM) {
  alphadisplay_set_blink(ALPHADISPLAY_BLINK_HALF_HZ);

  alphadisplay_write_ascii(0, 'A');
  alphadisplay_write_ascii(1, 'L');
  alphadisplay_write_ascii(2, 'R');
  alphadisplay_write_ascii(3, 'M');
}
}
