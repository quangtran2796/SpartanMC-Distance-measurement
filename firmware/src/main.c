/* Includes ------------------------------------------------------------------*/
#include <peripherals.h>
#include <stdio.h> 
#include "display.h"
#include "spi_rs01.h"
#include "srf02.h"
#include <interrupt.h>
#include "interrupt.h"
#include "intr_ctrl.h"
#define INT_MODE
#define MEDIAN_FILTER
#define TIMER_MODE

void timer_init();  
void distance_display(srf02_data *data);
void pattern_display();

void main(void){
	
	distance_data.data_status = NOT_UPDATED;
	distance_data.number_digit = 2;

	
	
	spi_init();
	i2c_peri_enable(&I2C_MASTER);

	OLED_Init_25664();
	Fill_RAM_25664(0x00);
	pattern_display();
	
	#ifdef INT_MODE
	#ifdef TIMER_MODE
		timer_init();
	#endif
	interrupt_enable(); 

	/* Trigger the first ranging process */
	srf02_trigger(&I2C_MASTER, SRF02_ADDRESS, SRF02_MODE_REAL_CM);
	#endif
	

	while(1){
		#ifndef INT_MODE
			srf02_polling();
		#endif
		
		if (srf02_get_status() == UPDATED)
		{	
				/* Define MEDIAN_FILTER to use median filter, otherwise the avarage filter is used */
				#ifdef MEDIAN_FILTER
					distance_data.distance = srf02_median_filter(distance_data.distance);
				#endif

				#ifndef MEDIAN_FILTER
					distance_data.distance  = srf02_filter(distance_data.distance);
				#endif
				/* Display the data on the screen */
				pattern_display();
				distance_display(&distance_data);
				distance_data.data_status = NOT_UPDATED;	
		}
	
	}
	
}

/**
  * @brief  Enable and configure capture timer.    
  *
  * @retval None
  */
void timer_init()
{ 
	/* The prescaler value is set to 256 */
    TIMER.control = TIMER_PRE_EN | TIMER_PRE_256;

	TIMER.limit = 0xffff; 
	COMPARE.CMP_CTRL = COMPARE_EN | COMPARE_EN_INT; 

	/* CMP_DAT is set to 15234 to create a 65ms interrupt */
	COMPARE.CMP_DAT = 15234;
} 

/**
  * @brief  Display data on OLED.
  * @param  data: Pointer to the data should be shown.    
  *
  * @retval None
  */
void distance_display(srf02_data *data){
	char data_buff[4];
	
	/* Clear the redundant digit on the display */
	if(data->distance > 100){
		data->number_digit = 3;
	}
	else if( (data->number_digit == 3) && (data->distance < 100)){
		Fill_Block_25664(0, 29, 30, 0, 6);
		data->number_digit = 2;
	}
	
	/* Show distance on OLED */
	snprintf(data_buff, 4, "%d", data->distance); 
	Show_String_25664(1, data_buff,25,0);
}

/**
  * @brief  Show the default pattern on OLED.   
  *
  * @retval None
  */
void pattern_display(){
	char data_buff[20];
	snprintf(data_buff, 20, "Distance is:"); 
	Show_String_25664(1, data_buff,0,0); 
	snprintf(data_buff, 20, "centimeter"); 
	Show_String_25664(1, data_buff,32,0);  
}

/**
  * @brief I2C interrupt service routine.
  */ 
ISR(0)(){

	/* Resend corresponding command when receives NO_ACK */
	if((I2C_MASTER.stat & I2C_STA_NO_ACK) == I2C_STA_NO_ACK){
		
		if(state == READING){
			srf02_read(&I2C_MASTER, SRF02_ADDRESS,2);
		}
		else if(state == SENSOR_REGISTER_SET){
			srf02_register_set(&I2C_MASTER, SRF02_ADDRESS);
		}
		else if(state == FIRMWARE_CHECK){
			srf02_read(&I2C_MASTER, SRF02_ADDRESS,1);
		}
		else if(state == FIRMWARE_READ){
			srf02_read(&I2C_MASTER, SRF02_ADDRESS,1);
		}
		else{
			srf02_trigger(&I2C_MASTER, SRF02_ADDRESS, SRF02_MODE_REAL_CM);
		}
	}
	else{

		if(state == READING){
			
				/* Update data after reading successfully */
				srf02_data_update_timer(&I2C_MASTER);
		
				/* Start a new ranging process */
				srf02_trigger(&I2C_MASTER, SRF02_ADDRESS, SRF02_MODE_REAL_CM);
			
		}
		else if(state == SENSOR_REGISTER_SET){
			
			/* Send the second part of read command after transmitting the first part successfully */
			srf02_read(&I2C_MASTER, SRF02_ADDRESS,2);
			
		}
		else if(state == FIRMWARE_CHECK){
			srf02_read(&I2C_MASTER, SRF02_ADDRESS,1);
		}
		else if(state == FIRMWARE_READ){
			srf02_firmware_read(&I2C_MASTER, SRF02_ADDRESS);
		}
		else{
			
			#ifdef TIMER_MODE
				/* Enable Timer */
				TIMER.control |= TIMER_EN;
			#endif
			
			#ifndef TIMER_MODE
				/* Start to check firmware version  */
				srf02_firmware_register_check(&I2C_MASTER, SRF02_ADDRESS);
			#endif
		}
	}
}

/**
  * @brief Timer interrupt service routine.
  */ 
ISR(1)(){

	/* Clear interrupt bit */
	COMPARE.CMP_DAT = 15234;  

	/* Reset timer value */
	TIMER.value = 0;

	/* Disable timer */
	TIMER.control &=~ TIMER_EN; 

	/* Send the first part of read command */
	srf02_register_set(&I2C_MASTER, SRF02_ADDRESS);
} 
