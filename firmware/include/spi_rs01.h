#ifndef __SPI_RS01_
#define __SPI_RS01_

/* Includes ------------------------------------------------------------------*/
#include "peripherals.h"   

/* Functions of SPI ----------------------------------------------------------*/
void spi_init(void);
void spi_peri_write(spi_master_regs_t *spi, unsigned int data);
void spi_peri_enable(spi_master_regs_t *spi);
void spi_peri_deselect(spi_master_regs_t *spi);
void spi_peri_select(spi_master_regs_t *spi, unsigned int device);


#endif  
