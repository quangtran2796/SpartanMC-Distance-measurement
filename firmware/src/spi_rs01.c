/* Includes ------------------------------------------------------------------*/
#include "i2c_driver.h"  
#include <peripherals.h>
#include  "spi_rs01.h"

/**
  * @brief  Enable SPI peripheral and select the slave.
  *
  * @retval None
  */
void spi_init(void){
	spi_peri_enable(&SPI_MASTER);
	spi_peri_select(&SPI_MASTER,1);
}

/**
  * @brief  Enable SPI peripheral.
  * @param  spi: Pointer to the selected SPI peripheral.
  *
  * @retval None
  */
void spi_peri_enable(spi_master_regs_t *spi){
	
	/* Clear the default value */
	spi->spi.spi_control &= ~SPI_MASTER_CTRL_BITCNT;
	//spi->spi.spi_control &= ~SPI_MASTER_CTRL_DIV;
	/* Set frame width to 9 */
	spi->spi.spi_control |= (9 << 8);
	
	/* Set clock divider to 2, which means that SCLK is 7.5MHz */
	spi->spi.spi_control |= (2 << 13);

	/* Enable SPI peripheral */
	spi->spi.spi_control |= SPI_MASTER_CTRL_EN;
}
 
/**
  * @brief  Deselect SPI slave.
  * @param  spi   : Pointer to the selected SPI peripheral.
  *
  * @retval None
  */
void spi_peri_deselect(spi_master_regs_t *spi){

	spi->spi.spi_control &= ~SPI_MASTER_CTRL_SLAVE;
}

/**
  * @brief  Select SPI slave.
  * @param  spi   : Pointer to the selected SPI peripheral.
  * @param  device: ID of selected slave.
  *
  * @retval None
  */
void spi_peri_select(spi_master_regs_t *spi, unsigned int device){
	
	spi->spi.spi_control |= (device << 4);
}

/**
  * @brief  Write data.
  * @param  spi   : Pointer to the selected SPI peripheral.
  * @param  data  : Data which should be written.
  *
  * @retval None
  */
void spi_peri_write(spi_master_regs_t *spi, unsigned int data){
	
	/* Check if the previous transmission is finished */
	while((spi->spi.spi_status & SPI_MASTER_STAT_FILL));
	spi->spi.spi_data_out = data;
}
