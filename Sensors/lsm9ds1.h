#pragma once
#include <string.h>
#include "stm32f10x.h"
#include "buffer.h"
#include <stdint.h>
#include "main.h"

//Sensor main bus address
#define A_G_ADDR 0xD4
#define M_ADDR 0x38

//Register addresses for the data, 8th bis are set

#define G_DATA 0x95
#define A_DATA 0xA8
#define M_DATA 0xA8

//Register addresses for the configuration, note that 8th bit is set to enable multibyte write with auto incriment
#define C_ADDR_ONE 0x0D
#define C_ADDR_TWO 0x10
#define C_ADDR_THREE 0x20
#define C_ADDR_MAG 0xA0

typedef struct{
	uint8_t dummy;	/*Used to align the variables to 32bit word boundaries*/
	int16_t temp;
	uint8_t stat;
	int16_t x;
	int16_t y;
	int16_t z;
}  __attribute__ ((aligned (1))) __attribute__((packed)) gyro_data_type; 

//The raw data buffers for the data
extern volatile gyro_data_type LSM9DS1_Gyro_Buffer;
extern volatile uint16_t LSM9DS1_Acc_Buffer[3];
extern volatile uint16_t LSM9DS1_Mag_Buffer[3];
extern volatile int16_t LSM9DS1_Mag_Offset[3];
extern volatile buff_type IMU_buff[10];//These are used for data output

extern volatile uint8_t I2C_failure;//counter

//For converting Gyro data into radians per second
#define GYRO_TO_RADIANS 0.001222
//For converting the Acc data to units of 9.81 meters per second squared (1G)
#define LSM9DS1_ACC_SCALE_FACTOR 0.000244

//+-8G,+-2000dps,238hz update with INT2 active low on new data 
#define A_G_ONE {0x01}
#define A_G_TWO {0x9B,0x0F,0x00,0x00}
//This gets sent to the third register address
#define A_G_THREE {0x98,0x00,0x74}
//Fast ODR (300Hz, called 'Medium Power')
#define M_ {0xA2,0x00,0x00,0x04,0x40}/*This uses values from LIS3MDL datasheet, assuming that table 20 is in the wrong order*/

#define I2C_JOBS_INITIALISER {\
{A_G_ADDR,I2C_Direction_Receiver,9,G_DATA,NULL}, /*Read the gyro temperature and rotation status*/\
{A_G_ADDR,I2C_Direction_Receiver,6,A_DATA,NULL}, /* Read the accel data*/\
{M_ADDR,I2C_Direction_Receiver,6,M_DATA,NULL}, /*Read the magno registers*/\
{A_G_ADDR,I2C_Direction_Transmitter,1,C_ADDR_ONE,(uint8_t [1]) A_G_ONE }, /*Setup LSM9DS1 G/A part one*/\
{A_G_ADDR,I2C_Direction_Transmitter,4,C_ADDR_TWO,(uint8_t [4]) A_G_TWO }, /*Setup ' ' part two*/\
{A_G_ADDR,I2C_Direction_Transmitter,3,C_ADDR_THREE,(uint8_t [3]) A_G_THREE }, /*Setup ' ' part three*/\
{M_ADDR,I2C_Direction_Transmitter,5,C_ADDR_MAG,(uint8_t [5]) M_ }, /*configure the magno*/\
}

//Job identifiers used to run the different I2C jobs, highest priority first
enum{LSM9DS1_GYRO=0,LSM9DS1_ACC,LSM9DS1_MAGNO,LSM9DS1_CONF1,LSM9DS1_CONF2,LSM9DS1_CONF3,LSM9DS1_CONF4};
#define I2C_NUMBER_JOBS 7
//Config all the sensors
#define CONFIG_SENSORS ((1<<LSM9DS1_CONF2)|(1<<LSM9DS1_CONF3)|(1<<LSM9DS1_CONF4))/*These are the last three of the four configuration tasks*/
#define SCHEDULE_CONFIG I2C1_Request_Job(LSM9DS1_CONF1);Jobs|=CONFIG_SENSORS/*Just adds directly - job request call starts i2c interrupts off*/

//Config for error detection and reset handling
#define LSM9DS1_FAILURE_COUNTOUT 80
#define I2C_RESET_TIMEOUT 200

//The function headers
void configure_i2c_buffers(void);
void handle_lsm9ds1(void);
uint8_t check_lsm9ds1_functionality(void);

