

#ifndef INC_MEMS_SENSOR_H_
#define INC_MEMS_SENSOR_H_

#include "main.h"
#include "spi.h"
#define LIS3DSH_CTRL_REG1                    0x21U
#define LIS3DSH_CTRL_REG6                    0x25U
#define LIS3DSH_CTRL_REG5                    0x24U
#define LIS3DSH_CTRL_REG4                    0x20U
#define LIS3DSH_CTRL_REG3                    0x23U
#define LIS3DSH_OUT_X_L                      0x28U
#define FIFO								 0x2EU
#define status_								 0x27U

void init_mems();
void read_mems(int16_t *a);
void platform_read( uint8_t reg, uint8_t *bufp, uint16_t len);


#endif /* INC_MEMS_SENSOR_H_ */
