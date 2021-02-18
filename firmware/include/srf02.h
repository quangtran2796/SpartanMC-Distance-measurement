#ifndef SRF02_H
#define SRF02_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "i2c_driver.h"
#include "peripherals.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * @brief SPI Ranging mode.
  */ 
typedef enum {
    SRF02_MODE_REAL_INCH    = 0x50,     /* result in inches */
    SRF02_MODE_REAL_CM      = 0x51,     /* result in centimeters */
    SRF02_MODE_REAL_MS      = 0x52,     /* result in microseconds */
    SRF02_MODE_FAKE_INCH    = 0x56,     /* result in inches (no pulse send) */
    SRF02_MODE_FAKE_CM      = 0x57,     /* result in cm (no pulse send) */
    SRF02_MODE_FAKE_MS      = 0x58      /* result in ms (no pulse send) */
} srf02_mode_t;
 
/**
  * @brief States of state machine.
  */ 
typedef enum{
    READING,
    TRIGGER,
    SENSOR_REGISTER_SET,
    FIRMWARE_CHECK,
    FIRMWARE_READ
} srf02_state;

/**
  * @brief Status of srf02 data.
  */ 
typedef enum{
	UPDATED,
	NOT_UPDATED
} srf02_data_status;

/**
  * @brief Srf02 data.
  */ 
typedef struct{
	int distance;
	int number_digit;
	srf02_data_status data_status;
} srf02_data;

extern srf02_state  state;
extern srf02_data distance_data;

#define SRF02_ADDRESS  0xe0
//#define MEDIAN_FILTER
/* Functions of SRF02 ------------------------------------------------------------------*/
void srf02_trigger( i2c_master_regs_t *i2c_master, char slave_addr, srf02_mode_t mode);
void srf02_register_set(i2c_master_regs_t *i2c_master, char slave_addr);
void srf02_read(i2c_master_regs_t *i2c_master, char slave_addr,int byte_to_read);
void srf02_data_update_timer(i2c_master_regs_t *i2c_master);
void srf02_data_update(i2c_master_regs_t *i2c_master, char slave_addr, srf02_mode_t mode);
int srf02_filter(int new_Data);
srf02_data_status srf02_get_status();
void srf02_polling();
int srf02_median_filter(int new_Data);
void srf02_firmware_register_check(i2c_master_regs_t *i2c_master, char slave_addr);
void srf02_firmware_read(i2c_master_regs_t *i2c_master, char slave_addr);

#ifdef __cplusplus
}
#endif

#endif /* SRF02_H */
