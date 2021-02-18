#ifndef __I2C_DRIVER_
#define __I2C_DRIVER_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "peripherals.h"
#ifdef __cplusplus
extern "C" {
#endif

/* the I2c functions ---------------------------------------------------------*/
void i2c_peri_enable(i2c_master_regs_t *i2c_master);
void i2c_peri_read(i2c_master_regs_t *i2c_master, char slave_addr, char byte_to_read);
void i2c_peri_write(i2c_master_regs_t *i2c_master, char slave_addr, char byte_to_write, char *data, char stop); // 1 -> stop bit

#ifdef __cplusplus
}
#endif
#endif
