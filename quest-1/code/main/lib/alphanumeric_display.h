#include <stdint.h>

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


/**
 * Sets the brightness of the alphanumeric display
 *
 * @param val Brightnexx (0-255)
 */
int alphadisplay_set_brightness_max(uint8_t val);


/**
 * Start writing to the display
 */
void alphadisplay_start();


/**
 * Stop writing to the display
 */
void alphadisplay_stop();

