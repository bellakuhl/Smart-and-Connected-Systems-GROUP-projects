#include <stdint.h>

#define ALPHADISPLAY_BLINK_OFF 0
#define ALPHADISPLAY_BLINK_2HZ 1
#define ALPHADISPLAY_BLINK_1HZ 2
#define ALPHADISPLAY_BLINK_HALF_HZ 3

/**
 * Initialize the alphanumeric display hooked
 * up via I2C
 */
int alphadisplay_init();


/**
 * Write an ASCII character to the the alphanumerica display
 *
 * @param digit The digit index to write to (0 - Left, 3 - Right)
 */
void alphadisplay_write_ascii(uint16_t digit, char a);


void alphadisplay_write_float(float val);
void alphadisplay_write_uint(uint32_t val);


/**
 * Sets the brightness of the alphanumeric display
 *
 * @param val Brightnexx (0-255)
 */
int alphadisplay_set_brightness_max(uint8_t val);

/*
 * Sets the blinking frequency of the display
 *
 * @param The frequency of the blinking
 */
int alphadisplay_set_blink(uint8_t freq);


/**
 * Start writing to the display
 */
void alphadisplay_start();


/**
 * Stop writing to the display
 */
void alphadisplay_stop();

