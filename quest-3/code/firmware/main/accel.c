#include <stdio.h>
#include <math.h>
#include "driver/i2c.h"
#include "./ADXL343.h"
#include "wearable.h"


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

// ADXL343
#define SLAVE_ADDR                         ADXL343_ADDRESS // 0x53

int steps = 0, flag = 0;
float x_vals[20];
float y_vals[20];
float z_vals[20];
float threshold;

// Function to initiate i2c -- note the MSB declaration!
static void i2c_master_init(){
  // Debug
  printf("\n>> i2c Config\n");
  int err;

  // Port configuration
  int i2c_master_port = I2C_EXAMPLE_MASTER_NUM;

  /// Define I2C configurations
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;                              // Master mode
  conf.sda_io_num = I2C_EXAMPLE_MASTER_SDA_IO;              // Default SDA pin
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
  conf.scl_io_num = I2C_EXAMPLE_MASTER_SCL_IO;              // Default SCL pin
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;                  // Internal pullup
  conf.master.clk_speed = I2C_EXAMPLE_MASTER_FREQ_HZ;       // CLK frequency
  err = i2c_param_config(i2c_master_port, &conf);           // Configure
  if (err == ESP_OK) {printf("- parameters: ok\n");}

  // Install I2C driver
  err = i2c_driver_install(i2c_master_port, conf.mode,
                     I2C_EXAMPLE_MASTER_RX_BUF_DISABLE,
                     I2C_EXAMPLE_MASTER_TX_BUF_DISABLE, 0);
  if (err == ESP_OK) {printf("- initialized: yes\n");}

  // Data in MSB mode
  i2c_set_data_mode(i2c_master_port, I2C_DATA_MODE_MSB_FIRST, I2C_DATA_MODE_MSB_FIRST);
}


// Write one byte to register
int writeRegister(uint8_t reg, uint8_t data) {
    int ret;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (SLAVE_ADDR << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data, ACK_CHECK_DIS);
    i2c_master_stop(cmd);
    ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return 0;
}

// Read register
uint8_t readRegister(uint8_t reg) {
    uint8_t data;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, reg, ACK_CHECK_DIS);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ( SLAVE_ADDR << 1 ) | I2C_MASTER_READ, ACK_CHECK_EN);
    i2c_master_read_byte(cmd, &data, ACK_CHECK_DIS);
    i2c_master_stop(cmd);
    int ret = i2c_master_cmd_begin(I2C_EXAMPLE_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return data;
}

// read 16 bits (2 bytes)
int16_t read16(uint8_t reg) {
  int16_t res = 0;
  uint8_t lsb = readRegister(reg);
  uint8_t msb = readRegister(reg + 1);
  //uint8_t msb = 0;

  res |= lsb << 0;
  res |= msb << 8;
  return res;
}


void setRange(range_t range) {
  /* Red the data format register to preserve bits */
  uint8_t format = readRegister(ADXL343_REG_DATA_FORMAT);

  /* Update the data rate */
  format &= ~0x0F;
  format |= range;

  /* Make sure that the FULL-RES bit is enabled for range scaling */
  format |= 0x08;

  /* Write the register back to the IC */
  writeRegister(ADXL343_REG_DATA_FORMAT, format);

}

range_t getRange(void) {
  /* Red the data format register to preserve bits */
  return (range_t)(readRegister(ADXL343_REG_DATA_FORMAT) & 0x03);
}

dataRate_t getDataRate(void) {
  return (dataRate_t)(readRegister(ADXL343_REG_BW_RATE) & 0x0F);
}

////////////////////////////////////////////////////////////////////////////////

// function to get acceleration
// void getAccel(float * xp, float *yp, float *zp) {
//   *xp = read16(ADXL343_REG_DATAX0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
//   *yp = read16(ADXL343_REG_DATAY0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
//   *zp = read16(ADXL343_REG_DATAZ0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
//   //printf("X: %.2f \t Y: %.2f \t Z: %.2f\n", *xp, *yp, *zp);
// }

// function to print roll and pitch
/*double calcRP(float x, float y, float z){
  double roll = atan2(y , z) * 57.3;
  double pitch = atan2((- x) , sqrt(y * y + z * z)) * 57.3;
  double tilt = acos(z / sqrt(x*x + y*y + z*z)) * 57.3;
  return tilt;
  //printf("roll: %.2f \t pitch: %.2f \t tilt: %.2f\n", roll, pitch, tilt);
}
*/
// Task to continuously poll acceleration and calculate roll and pitch

/*Function to find minimum of x and y*/
int min(int x, int y) 
{ 
return y ^ ((x ^ y) & -(x < y)); 
} 
  
/*Function to find maximum of x and y*/
int max(int x, int y) 
{ 
return x ^ ((x ^ y) & -(x < y));  
} 


// Function to count steps
static void count_steps() {
  /*
  This function uses the z axis acceleration data (vertical, up/down data)
  to find when the user takes a step because this is where you find the most
  consistent variation per step.
  */
  double maxs[] = {-100, -100, -100};
  double mins[] = {100, 100, 100};
  double avgs[] = {0, 0, 0}; 
  for (int i=0;i<20;i++){
    //x_vals[i] = read16(ADXL343_REG_DATAX0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
    //y_vals[i] = read16(ADXL343_REG_DATAY0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
    z_vals[i] = read16(ADXL343_REG_DATAZ0) * ADXL343_MG2G_MULTIPLIER * SENSORS_GRAVITY_STANDARD;
    //maxs[0] = max(maxs[0], x_vals[i]);
    //maxs[1] = max(maxs[1], y_vals[i]);
    maxs[2] = max(maxs[2], z_vals[i]);
    //mins[0] = min(mins[0], x_vals[i]);
    //mins[1] = min(mins[1], y_vals[i]);
    mins[2] = min(mins[2], z_vals[i]);
    if (z_vals[i] > threshold && flag==0){
      steps=steps+1;
      flag=1;
      //printf("a step!\n");
    }
    if (z_vals[i] < threshold && flag==1){
      flag=0;
    }
    vTaskDelay(50 / portTICK_RATE_MS);
  }
  //avgs[0] = (maxs[0]+mins[0])/2;
  //avgs[1] = (maxs[1]+mins[1])/2;
  avgs[2] = (maxs[2]+mins[2])/2;
  threshold = avgs[2];


}


// Calibrate pedometer in first 5 seconds of walking
static void calibrate_ped() {
  printf("Calibrating, please walk around.\n");
  for (int i=0;i<5;i++){
    count_steps();
  }
  steps=0, flag=0;
  printf("Finished calibration, beginning step counter.\n");
}


static void step_counter() {
  printf("\n>> Polling ADAXL343\n");
  calibrate_ped();
  while (1) {
    //float xVal, yVal, zVal;
    //getAccel(&xVal, &yVal, &zVal);
    count_steps();
    //printf("Threshold: %.2f\t Steps: %d\n", threshold, steps);

    //calcRP(xVal, yVal, zVal);
    vTaskDelay(50 / portTICK_RATE_MS);
  }
}

void accel_init(){
  i2c_master_init();
  //i2c_scanner();


  // Disable interrupts
  writeRegister(ADXL343_REG_INT_ENABLE, 0);

  // Set range
  setRange(ADXL343_RANGE_16_G);
  

  // Enable measurements
  writeRegister(ADXL343_REG_POWER_CTL, 0x08);
  xTaskCreate(step_counter,"step_counter", 4096, NULL, 5, NULL);
}


int accel_step_count(){
  return steps;
}

/*double accel_read_tilt(){
	float xVal, yVal, zVal;
    getAccel(&xVal, &yVal, &zVal);
    tilt = calcRP(xVal, yVal, zVal);
    vTaskDelay(1000 / portTICK_RATE_MS);
    return tilt;
}*/

/*void app_main() {

  // Routine
  i2c_master_init();
  i2c_scanner();


  // Disable interrupts
  writeRegister(ADXL343_REG_INT_ENABLE, 0);

  // Set range
  setRange(ADXL343_RANGE_16_G);
  

  // Enable measurements
  writeRegister(ADXL343_REG_POWER_CTL, 0x08);

  // Create task to poll ADXL343
  xTaskCreate(test_adxl343,"test_adxl343", 4096, NULL, 5, NULL);
}*/


