#include "MEMS_sensor.h"

//write acceleration sensor
void platform_write(uint8_t reg, const uint8_t *bufp,uint16_t len)
{

	HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, &reg, 1, 1000);
	HAL_SPI_Transmit(&hspi1, (uint8_t*) bufp, len, 1000);
	HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_SET);

}

//read acceleration sensor
void platform_read( uint8_t reg, uint8_t *bufp, uint16_t len)
{

	reg |= 0x80;

	/* Read multiple command */
	HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1,&reg, 1, 1000);
	HAL_SPI_Receive(&hspi1, bufp, len, 1000);
	HAL_GPIO_WritePin(CS_I2C_SPI_GPIO_Port, CS_I2C_SPI_Pin, GPIO_PIN_SET);

}


void init_mems()
{

	const uint8_t reg1=0x08;//X,Y,Z enable ODR=100 Hz update rate
	const uint8_t reg3=0x50;//DRY active high on INIT1 pin
	const uint8_t reg4=0x67;//X,Y,Z enable ODR=100 Hz update rate
	const uint8_t reg5=0x40;//Anti-aliasing filter bandwidth 200Hz, +-2G,SPI 4 Wire
	const uint8_t reg6=0x51;//FIFO_EN ->1,Enable FIFO empty indication on Int1->1
	//HAL_Delay(5);
	platform_write(LIS3DSH_CTRL_REG4, &reg4,1);
	platform_write(LIS3DSH_CTRL_REG5, &reg5,1);
	HAL_Delay(10);

}

void read_mems(int16_t *a)
{

	uint8_t buffer[6];
	uint8_t x=0;
	//float beschleunigungswert = beschleunigungswert = 2.0/(65536-1);

	platform_read(LIS3DSH_OUT_X_L, buffer, 6);

	for(int n=0;n<3;n++)
	{
		a[x]=((int16_t)((buffer[n+1] << 8) | buffer[n]));
		x++;
	}

}
