
/* Includes ------------------------------------------------------------------*/
#include "srf02.h"  
#include "i2c_driver.h"  
#include "display.h"   
#include <stdio.h> 
/* Srf02 register addresses --------------------------------------------------*/
#define REG_CMD             (0x00)
#define REG_HIGH            (0x02)
#define REG_LOW             (0x03)
#define REG_AUTO_HIGH       (0x04)
#define REG_AUTO_LOW        (0x05)

/* Filter parameters ---------------------------------------------------------*/
#define FILTER_SIZE			3     /* set the filter size */ 
#define	FILTER_SIZE_MAX		20    /* set the filter maximun size */

/* Global variables ----------------------------------------------------------*/
srf02_state  state = TRIGGER;
srf02_data distance_data;
int filter_buff[FILTER_SIZE_MAX];
int filter_pointer = 0, filter_start = 0;
int read_count = 0;
/**
  * @brief  Trigger a new ranging process.
  * @param  i2c_master: Pointer to the selected I2C peripheral.
  * @param  slave_addr: Address of the selected slave.
  * @param  mode      : Decide if the result is in inches, centimeters or microseconds.     
  *
  * @retval None
  */
void srf02_trigger( i2c_master_regs_t *i2c_master, char slave_addr, srf02_mode_t mode){
	
	char data_buff[2] = {REG_CMD, mode};
	i2c_peri_write(i2c_master, slave_addr, 2, data_buff,1);
	
	state = TRIGGER;
}

/**
  * @brief  Send the first part of read command, which indicate the registers should be read.
  * @param  i2c_master: Pointer to the selected I2C peripheral.
  * @param  slave_addr: Address of the selected slave.
  *
  * @retval None
  */
void srf02_register_set(i2c_master_regs_t *i2c_master, char slave_addr){
	
	char data = REG_HIGH;
	i2c_peri_write(i2c_master, slave_addr, 1, &data, 0);

	state = SENSOR_REGISTER_SET;
}

/**
  * @brief  Read data from slave.
  * @param  i2c_master: Pointer to the selected I2C peripheral.
  * @param  slave_addr: Address of the selected slave.
  *
  * @retval None
  */
void srf02_read(i2c_master_regs_t *i2c_master, char slave_addr,int byte_to_read){
	
	i2c_peri_read(i2c_master, slave_addr,byte_to_read);
	if(byte_to_read == 1)
		state = FIRMWARE_READ;
	else
		state = READING;
}

/**
  * @brief  Take and filter received data from I2C buffer.
  * @param  i2c_master: Pointer to the selected I2C peripheral.
  *
  * @retval None
  */
void srf02_data_update_timer(i2c_master_regs_t *i2c_master){
	
	distance_data.distance = (i2c_master->data[1] << 8) + i2c_master->data[2];

	if( distance_data.distance != 0){
		distance_data.data_status = UPDATED;
	}
}

/**
  * @brief  Average filter.
  * @param  new_Data: New data which should be filtered.
  *
  * @retval the filtered value or 0 when there are not enough data to filter.
  */
int srf02_filter(int new_Data){
	
	int filtered_value = 0;
	
	if(filter_pointer >= FILTER_SIZE){
		filter_pointer = 0;
		filter_start = 1;
	}
		
	filter_buff[filter_pointer++] = new_Data;
	if(filter_start == 1){
		for(int i = 0; i < FILTER_SIZE; i++)
			filtered_value += filter_buff[i];
			
	return filtered_value / FILTER_SIZE;
	}
	return 0;
}
/**
  * @brief  Median filter.
  * @param  new_Data: New data which should be filtered.
  *
  * @retval the filtered value or 0 when there are not enough data to filter.
  */
int srf02_median_filter(int new_Data){
	
	if(filter_pointer >= FILTER_SIZE){
		filter_pointer = 0;
	}
	
	filter_buff[filter_pointer++] = new_Data;
	int swap = 0;
	/* Sort the data and return the middle value */
	for( int i = 0; i < FILTER_SIZE - 1; i++){
		for(int j = i + 1; j < FILTER_SIZE; j++){
			if(filter_buff[i] < filter_buff[j]){
			swap = filter_buff[i];
			filter_buff[i] = filter_buff[j];
			filter_buff[j] = swap;
			}
		}
	}
	
	return filter_buff[(FILTER_SIZE)/2];
}

/**
  * @brief  Return the status if there is new data.
  *
  * @retval the filtered value or 0 when there are not enough data to filter.
  */
srf02_data_status srf02_get_status(){
	_SLEEP_US(0);
	return distance_data.data_status;
}

/**
  * @brief  I2C polling communication for testing purpose.
  *
  * @retval the filtered value or 0 when there are not enough data to filter.
  */
void srf02_polling(){
	do{
		srf02_trigger(&I2C_MASTER, SRF02_ADDRESS, SRF02_MODE_REAL_CM);
		while (I2C_MASTER.stat & I2C_STA_TIP);
	}
	while(I2C_MASTER.stat & I2C_STA_NO_ACK);

	do{
		srf02_register_set(&I2C_MASTER, SRF02_ADDRESS);
		while (I2C_MASTER.stat & I2C_STA_TIP);

	}
	while(I2C_MASTER.stat & I2C_STA_NO_ACK);

	do{
		srf02_read(&I2C_MASTER, SRF02_ADDRESS,2);
		while (I2C_MASTER.stat & I2C_STA_TIP);
	}
	while(I2C_MASTER.stat & I2C_STA_NO_ACK);

	srf02_data_update_timer(&I2C_MASTER);
	
}

/**
  * @brief  Get new data.
  * @param  i2c_master: Pointer to the selected I2C peripheral.
  * 		slave_addr: Address of the selected slave.
  *
  * @retval the filtered value or 0 when there are not enough data to filter.
  */
void srf02_data_update(i2c_master_regs_t *i2c_master, char slave_addr, srf02_mode_t mode){
	
	distance_data.distance = (i2c_master->data[1] << 8) + i2c_master->data[2];
	
	if(distance_data.distance == 0){
		srf02_read(i2c_master, slave_addr,2);
	}
	else{
		distance_data.data_status = UPDATED;
		srf02_trigger(i2c_master, slave_addr, mode);
	}
}
/**
  * @brief  Send the read command to read firmware version.
  * @param  i2c_master: Pointer to the selected I2C peripheral.
  * 		slave_addr: Address of the selected slave.
  *
  * @retval the filtered value or 0 when there are not enough data to filter.
  */
void srf02_firmware_register_check(i2c_master_regs_t *i2c_master, char slave_addr){
	char data = REG_CMD;
	i2c_peri_write(i2c_master, slave_addr, 1, &data, 0);

	state = FIRMWARE_CHECK;
}
/**
  * @brief  Get the firmware version. If it is not equal to 0, then read the new data 
  * @param  i2c_master: Pointer to the selected I2C peripheral.
  * 		slave_addr: Address of the selected slave.
  *
  * @retval the filtered value or 0 when there are not enough data to filter.
  */
void srf02_firmware_read(i2c_master_regs_t *i2c_master, char slave_addr){
	int data = i2c_master->data[1];
	if(data == 0){
		srf02_firmware_register_check(i2c_master, slave_addr);
		read_count =0;
	}
	else{
		
		if(read_count == 3){
			srf02_register_set(i2c_master, slave_addr);
			read_count =0;
		}
		else
			srf02_firmware_register_check(i2c_master, slave_addr);
			
		read_count++;
	}
}
