/* Includes ------------------------------------------------------------------*/
#include "i2c_driver.h"  

/**
  * @brief  Enable I2C peripheral and I2C interrupt.
  * @param  i2c_master: Pointer to the selected I2C peripheral.
  *
  * @retval None
  */
void i2c_peri_enable(i2c_master_regs_t *i2c_master){

	/* Prescaler value is set to 299, which means that the desired_SCL is 40KHz */
	i2c_master->ctrl = I2C_CTRL_EN | 299 | I2C_CTRL_INTR_EN;
}

/**
  * @brief  Read data from selected registers.
  * @param  i2c_master  : Pointer to the selected I2C peripheral.
  * @param  slave_addr  : Address of the selected slave.
  * @param  byte_to_read: The number of bytes should be read.    
  *
  * @retval None
  */
void i2c_peri_read(i2c_master_regs_t *i2c_master, char slave_addr, char byte_to_read){

	/* Slave address must be added by 1 to indicate that this is a read command */
	i2c_master->data[0] = (slave_addr) + 1;  

	/* byte_to_read is added by 1, because the first byte is always the slave address */
	i2c_master->cmd = I2C_CMD_RD | I2C_CMD_STA | I2C_CMD_STO | ((byte_to_read + 1) << 3);
}

/**
  * @brief  write data to selected register.
  * @param  i2c_master   : Pointer to the selected I2C peripheral.
  * @param  slave_addr   : Address of the selected slave.
  * @param  data  		 : Pointer to the data array which should be written.   
  * @param  byte_to_write: The number of bytes should be written.
  * @param  stop         : When stop is equal to 1, a stop condition will be generated and vice versa.    
  *
  * @retval None
  */
void i2c_peri_write(i2c_master_regs_t *i2c_master, char slave_addr, char byte_to_write, char *data, char stop){

	i2c_master->data[0] = (slave_addr);	 

	/* Prepare data to send */
	for(int i = 1; i <= byte_to_write; i++)
		i2c_master->data[i] = data[i-1];
	
	/* byte_to_write is added by 1, because the first byte is always the slave address */
	if(stop == 1){   
		i2c_master->cmd = I2C_CMD_STA | I2C_CMD_STO | ((byte_to_write + 1) << 3);
	}
	else{
		i2c_master->cmd = I2C_CMD_STA | ((byte_to_write + 1) << 3);
	}	
}
