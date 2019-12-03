/*
  This module was adapted from Emily Lam's example code:

  https://github.com/BU-EC444/code-examples/tree/master/i2c-display

  and Adafruit's LED Backpack:

  https://github.com/adafruit/Adafruit_LED_Backpack
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"

#include "alphanumeric_display.h"

// 14-Segment Display
#define SLAVE_ADDR                         0x70 // alphanumeric address
#define OSC                                0x21 // oscillator cmd
#define HT16K33_BLINK_DISPLAYON            0x01 // Display on cmd
#define HT16K33_BLINK_OFF                  0    // Blink off cmd
#define HT16K33_BLINK_CMD                  0x80 // Blink cmd
#define HT16K33_CMD_BRIGHTNESS             0xE0 // Brightness cmd

// Master I2C
#define I2C_EXAMPLE_MASTER_SCL_IO          22   // gpio number for i2c clk
#define I2C_EXAMPLE_MASTER_SDA_IO          23   // gpio number for i2c data
#define I2C_EXAMPLE_MASTER_NUM             I2C_NUM_0  // i2c port
#define I2C_EXAMPLE_MASTER_TX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_RX_BUF_DISABLE  0    // i2c master no buffer needed
#define I2C_EXAMPLE_MASTER_FREQ_HZ         100000     // i2c master clock freq
#define WRITE_BIT                          I2C_MASTER_WRITE // i2c master write
#define READ_BIT                           I2C_MASTER_READ  // i2c master read
#define ACK_CHECK_EN                       true // i2c master will check ack
#define ACK_CHECK_DIS                      false// i2c master will not check ack
#define ACK_VAL                            0x00 // i2c ack value
#define NACK_VAL                           0xFF // i2c nack value

#define ALPHA_DISPLAYBUFFER_SIZE (8)

TaskHandle_t *write_display_task = NULL;
static uint16_t ALPHA_DISPLAYBUFFER_G[ALPHA_DISPLAYBUFFER_SIZE];

static const uint16_t alphafonttable[] =  {
	0b0000000000000001,
	0b0000000000000010,
	0b0000000000000100,
	0b0000000000001000,
	0b0000000000010000,
	0b0000000000100000,
	0b0000000001000000,
	0b0000000010000000,
	0b0000000100000000,
	0b0000001000000000,
	0b0000010000000000,
	0b0000100000000000,
	0b0001000000000000,
	0b0010000000000000,
	0b0100000000000000,
	0b1000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0000000000000000,
	0b0001001011001001,
	0b0001010111000000,
	0b0001001011111001,
	0b0000000011100011,
	0b0000010100110000,
	0b0001001011001000,
	0b0011101000000000,
	0b0001011100000000,
	0b0000000000000000, //
	0b0000000000000110, // !
	0b0000001000100000, // "
	0b0001001011001110, // #
	0b0001001011101101, // $
	0b0000110000100100, // %
	0b0010001101011101, // &
	0b0000010000000000, // '
	0b0010010000000000, // (
	0b0000100100000000, // )
	0b0011111111000000, // *
	0b0001001011000000, // +
	0b0000100000000000, // ,
	0b0000000011000000, // -
	0b0100000000000000, // .
	0b0000110000000000, // /
	0b0000110000111111, // 0
	0b0000000000000110, // 1
	0b0000000011011011, // 2
	0b0000000010001111, // 3
	0b0000000011100110, // 4
	0b0010000001101001, // 5
	0b0000000011111101, // 6
	0b0000000000000111, // 7
	0b0000000011111111, // 8
	0b0000000011101111, // 9
	0b0001001000000000, // :
	0b0000101000000000, // ;
	0b0010010000000000, // <
	0b0000000011001000, // =
	0b0000100100000000, // >
	0b0001000010000011, // ?
	0b0000001010111011, // @
	0b0000000011110111, // A
	0b0001001010001111, // B
	0b0000000000111001, // C
	0b0001001000001111, // D
	0b0000000011111001, // E
	0b0000000001110001, // F
	0b0000000010111101, // G
	0b0000000011110110, // H
	0b0001001000000000, // I
	0b0000000000011110, // J
	0b0010010001110000, // K
	0b0000000000111000, // L
	0b0000010100110110, // M
	0b0010000100110110, // N
	0b0000000000111111, // O
	0b0000000011110011, // P
	0b0010000000111111, // Q
	0b0010000011110011, // R
	0b0000000011101101, // S
	0b0001001000000001, // T
	0b0000000000111110, // U
	0b0000110000110000, // V
	0b0010100000110110, // W
	0b0010110100000000, // X
	0b0001010100000000, // Y
	0b0000110000001001, // Z
	0b0000000000111001, // [
	0b0010000100000000, //
	0b0000000000001111, // ]
	0b0000110000000011, // ^
	0b0000000000001000, // _
	0b0000000100000000, // `
	0b0001000001011000, // a
	0b0010000001111000, // b
	0b0000000011011000, // c
	0b0000100010001110, // d
	0b0000100001011000, // e
	0b0000000001110001, // f
	0b0000010010001110, // g
	0b0001000001110000, // h
	0b0001000000000000, // i
	0b0000000000001110, // j
	0b0011011000000000, // k
	0b0000000000110000, // l
	0b0001000011010100, // m
	0b0001000001010000, // n
	0b0000000011011100, // o
	0b0000000101110000, // p
	0b0000010010000110, // q
	0b0000000001010000, // r
	0b0010000010001000, // s
	0b0000000001111000, // t
	0b0000000000011100, // u
	0b0010000000000100, // v
	0b0010100000010100, // w
	0b0010100011000000, // x
	0b0010000000001100, // y
	0b0000100001001000, // z
	0b0000100101001001, // {
	0b0001001000000000, // |
	0b0010010010001001, // }
	0b0000010100100000, // ~
	0b0011111111111111,
};

int alphadisplay_test_connection(uint8_t devAddr, int32_t timeout);
// Utility function to scan for i2c device
static void i2c_scanner()
{
    int32_t scanTimeout = 1000;
    printf("\n>> I2C scanning ..."  "\n");
    uint8_t count = 0;

    for (uint8_t i = 1; i < 127; i++) {
        // printf("0x%X%s",i,"\n");
        if (alphadisplay_test_connection(i, scanTimeout) == ESP_OK) {
            printf( "- Device found at address: 0x%X%s", i, "\n");
            count++;
        }
    }

    if (count == 0) {

        printf("- No I2C devices found!" "\n");
	}

    printf("\n");
}


static void task_write_display_buffers()
{
    // Continually writes the same command
    i2c_cmd_handle_t cmd4 = i2c_cmd_link_create();
    i2c_master_start(cmd4);
    i2c_master_write_byte(cmd4, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd4, (uint8_t)0x00, ACK_CHECK_EN);

    for (uint8_t i=0; i< ALPHA_DISPLAYBUFFER_SIZE; i++)
    {
        i2c_master_write_byte(cmd4, ALPHA_DISPLAYBUFFER_G[i] & 0xFF, ACK_CHECK_EN);
        i2c_master_write_byte(cmd4, ALPHA_DISPLAYBUFFER_G[i] >> 8, ACK_CHECK_EN);
    }

    i2c_master_stop(cmd4);
    i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd4, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd4);
}


static int alphadisplay_i2c_config_default(int i2c_master_port, i2c_config_t *config)
{
    config->mode = I2C_MODE_MASTER;                              // Master mode
    config->sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;              // Default SDA pin
    config->sda_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
    config->scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;              // Default SCL pin
    config->scl_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
    config->master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;       // CLK frequency

	return i2c_param_config(i2c_master_port, config);           // Configure
}


// Function to initiate i2c -- note the MSB declaration!
static void alphadisplay_i2c_init()
{
    // Debug
    printf("\n>> i2c Config\n");
    int err;

    // Port configuration
    int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;

    /// Define I2C configurations
    i2c_config_t conf;
	alphadisplay_i2c_config_default(i2c_master_port, &conf);

    // Install I2C driver
    err = i2c_driver_install(i2c_master_port, conf.mode,
                       I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                       I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
    // i2c_set_data_mode(i2c_master_port,I2C_DATA_MODE_LSB_FIRST,I2C_DATA_MODE_LSB_FIRST);
    if (err == ESP_OK) {printf("- initialized: yes\n\n");}

    // Dat in MSB mode
    i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
    i2c_scanner();
}


void alphadisplay_write_ascii(uint16_t digit, char a)
{
    if (digit > 3) {
        return;
    }

	uint16_t font = alphafonttable[(uint8_t)a];
	ALPHA_DISPLAYBUFFER_G[digit] = font;
    task_write_display_buffers();
}

void alphadisplay_write_float(float value)
{
    uint16_t dot = alphafonttable['.'];
    char str[32];

    if (value > 10000) {
        ALPHA_DISPLAYBUFFER_G[0] = 'O';
        ALPHA_DISPLAYBUFFER_G[1] = 'F';
        ALPHA_DISPLAYBUFFER_G[2] = 'L';
        ALPHA_DISPLAYBUFFER_G[2] = 'W';
    }
    else if (value > 1000) {
        sprintf(str, "%.0f", value);
        ALPHA_DISPLAYBUFFER_G[0] = alphafonttable[(uint8_t)str[0]];
        ALPHA_DISPLAYBUFFER_G[1] = alphafonttable[(uint8_t)str[1]];
        ALPHA_DISPLAYBUFFER_G[2] = alphafonttable[(uint8_t)str[2]];
        ALPHA_DISPLAYBUFFER_G[3] = alphafonttable[(uint8_t)str[3]];
    }
    else if (value > 100)  {
        sprintf(str, "%.1f", value);
        ALPHA_DISPLAYBUFFER_G[0] = alphafonttable[(uint8_t)str[0]];
        ALPHA_DISPLAYBUFFER_G[1] = alphafonttable[(uint8_t)str[1]];
        ALPHA_DISPLAYBUFFER_G[2] = alphafonttable[(uint8_t)str[2]] | dot;
        ALPHA_DISPLAYBUFFER_G[3] = alphafonttable[(uint8_t)str[4]];
    }
    else if (value > 10) {
        sprintf(str, "%.2f", value);
        ALPHA_DISPLAYBUFFER_G[0] = alphafonttable[(uint8_t)str[0]];
        ALPHA_DISPLAYBUFFER_G[1] = alphafonttable[(uint8_t)str[1]] | dot;
        ALPHA_DISPLAYBUFFER_G[2] = alphafonttable[(uint8_t)str[3]];
        ALPHA_DISPLAYBUFFER_G[3] = alphafonttable[(uint8_t)str[4]];
    }
    else if (value > 0) {
        sprintf(str, "%.3f", value);
        ALPHA_DISPLAYBUFFER_G[0] = alphafonttable[(uint8_t)str[0]] | dot;
        ALPHA_DISPLAYBUFFER_G[1] = alphafonttable[(uint8_t)str[2]];
        ALPHA_DISPLAYBUFFER_G[2] = alphafonttable[(uint8_t)str[3]];
        ALPHA_DISPLAYBUFFER_G[3] = alphafonttable[(uint8_t)str[4]];
    }
    else {
        ALPHA_DISPLAYBUFFER_G[0] = alphafonttable[(uint8_t)'0'];
        ALPHA_DISPLAYBUFFER_G[1] = alphafonttable[(uint8_t)'0'];
        ALPHA_DISPLAYBUFFER_G[2] = alphafonttable[(uint8_t)'0'];
        ALPHA_DISPLAYBUFFER_G[3] = alphafonttable[(uint8_t)'0'];
    }

    task_write_display_buffers();
}

void alphadisplay_write_uint(uint32_t value)
{
    if (value > 9999) {
        ALPHA_DISPLAYBUFFER_G[0] = 'O';
        ALPHA_DISPLAYBUFFER_G[1] = 'F';
        ALPHA_DISPLAYBUFFER_G[2] = 'L';
        ALPHA_DISPLAYBUFFER_G[2] = 'W';
    }
    else {
        char str[5];
        sprintf(str, "%d", value);
        ALPHA_DISPLAYBUFFER_G[0] = alphafonttable[(uint8_t)str[0]];
        ALPHA_DISPLAYBUFFER_G[1] = alphafonttable[(uint8_t)str[1]];
        ALPHA_DISPLAYBUFFER_G[2] = alphafonttable[(uint8_t)str[2]];
        ALPHA_DISPLAYBUFFER_G[3] = alphafonttable[(uint8_t)str[3]];
    }

    task_write_display_buffers();
}


int alphadisplay_test_connection(uint8_t devAddr, int32_t timeout)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_stop(cmd);
    int err = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return err;
}



// Turn on oscillator for alpha display
int alphadisplay_oscillator()
{
	int ret;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, OSC, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	vTaskDelay(200 / portTICK_RATE_MS);
	return ret;
}


// Set blink rate to off
int alphadisplay_no_blink()
{
	  int ret;
	  i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
	  i2c_master_start(cmd2);
	  i2c_master_write_byte(cmd2, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
	  i2c_master_write_byte(cmd2, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | (HT16K33_BLINK_OFF << 1), ACK_CHECK_EN);
	  i2c_master_stop(cmd2);
	  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd2, 1000 / portTICK_RATE_MS);
	  i2c_cmd_link_delete(cmd2);
	  vTaskDelay(200 / portTICK_RATE_MS);
	  return ret;
}

// Set Blink
int alphadisplay_set_blink(uint8_t freq)
{
	  int ret;

      uint8_t blink_bits = 0; // No blink
      if (freq == ALPHADISPLAY_BLINK_1HZ) {
          blink_bits = 2;
      }
      else if (freq == ALPHADISPLAY_BLINK_2HZ) {
          blink_bits = 1;
      }
      else if (freq == ALPHADISPLAY_BLINK_HALF_HZ) {
          blink_bits = 3;
      }

	  i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
	  i2c_master_start(cmd2);
	  i2c_master_write_byte(cmd2, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
	  i2c_master_write_byte(cmd2, HT16K33_BLINK_CMD | HT16K33_BLINK_DISPLAYON | blink_bits, ACK_CHECK_EN);
	  i2c_master_stop(cmd2);
	  ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd2, 1000 / portTICK_RATE_MS);
	  i2c_cmd_link_delete(cmd2);
	  vTaskDelay(200 / portTICK_RATE_MS);
	  return ret;
}

// Set Brightness
int alphadisplay_set_brightness_max(uint8_t val)
{
	int ret;
	i2c_cmd_handle_t cmd3 = i2c_cmd_link_create();
	i2c_master_start(cmd3);
	i2c_master_write_byte(cmd3, ( SLAVE_ADDR << 1 ) | WRITE_BIT, ACK_CHECK_EN);
	i2c_master_write_byte(cmd3, HT16K33_CMD_BRIGHTNESS | val, ACK_CHECK_EN);
	i2c_master_stop(cmd3);
	ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd3, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd3);
	vTaskDelay(200 / portTICK_RATE_MS);
	return ret;
}


int alphadisplay_init()
{
    int ret = ESP_OK;
    alphadisplay_i2c_init();

    // Turn on alpha oscillator
    ret = alphadisplay_oscillator();
    if(ret != ESP_OK) {return ret;}

    // Set display blink off
    ret = alphadisplay_no_blink();
    if(ret != ESP_OK) {return ret;}

    ret = alphadisplay_set_brightness_max(0xF);
    return ret;
}


void alphadisplay_start()
{
    if (write_display_task != NULL) {
        // already running...
        return;
    }
    /*
    This was a bad idea.
    xTaskCreate(
        task_write_display_buffers,
        "task_write_display_buffers",
        4096,
        NULL,
        5,
        write_display_task);
    */
}

void alphadisplay_stop()
{
    if (write_display_task == NULL) {
        // already stopped...
        return;
    }
    //
    //vTaskDelete(write_display_task);
}

