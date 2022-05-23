#include <assert.h>
#include "LCD_I2C_Driver.h"

void lcd_command(LCD_Handle_t* lcd_config, uint8_t cmd){
	uint8_t data_u = 0, data_l = 0;
	uint8_t data_t[4];
	data_u = (cmd&0xf0);
	data_l = ((cmd<<4)&0xf0);
	data_t[0] = data_u|0x04 | (lcd_config->bg << BG_BIT_POSITION);  //en=1, rs=0
	data_t[1] = data_u|0x00 | (lcd_config->bg << BG_BIT_POSITION);  //en=0, rs=0
	data_t[2] = data_l|0x04 | (lcd_config->bg << BG_BIT_POSITION);  //en=1, rs=0
	data_t[3] = data_l|0x00 | (lcd_config->bg << BG_BIT_POSITION); //en=0, rs=0
	// Check is RW is set,
	for (int i = 0; i< 4; i++){
		assert( (data_t[i] & 1 << 1)  == 0);
	}
	HAL_I2C_Master_Transmit(lcd_config->i2c_handle, lcd_config->Slave_address,(uint8_t *) data_t, 4, 100);
}

void printLetter(LCD_Handle_t* lcd_config, uint8_t letter){
	uint8_t data_u, data_l;
	uint8_t data_t[4];
	data_u = (letter&0xf0);
	data_l = ((letter<<4)&0xf0);
	data_t[0] = data_u|0x05| (lcd_config->bg << BG_BIT_POSITION);  //en=1, rs=1
	data_t[1] = data_u|0x01| (lcd_config->bg << BG_BIT_POSITION); //en=0, rs=1
	data_t[2] = data_l|0x05| (lcd_config->bg << BG_BIT_POSITION); //en=1, rs=1
	data_t[3] = data_l|0x01| (lcd_config->bg << BG_BIT_POSITION);  //en=0, rs=1
	// Check is RW is set,
	for (int i = 0; i< 4; i++){
		assert( (data_t[i] & 1 << 1)  == 0);
	}

	HAL_I2C_Master_Transmit(lcd_config->i2c_handle, lcd_config->Slave_address,(uint8_t *) data_t, 4, 100);
}


// Inistialiserung des Displays
void lcd_init(LCD_Handle_t* lcd_config){
	// 4 bit initialisation
	HAL_Delay(50);  // wait for >40ms
	lcd_command (lcd_config, 0x30);
	HAL_Delay(5);  // wait for >4.1ms
	lcd_command (lcd_config, 0x30);
	HAL_Delay(1);  // wait for >100us
	lcd_command (lcd_config, 0x30);
	HAL_Delay(10);
	lcd_command (lcd_config,0x20);  // 4bit mode
	HAL_Delay(10);
	HAL_Delay(100);  // wait for >40ms

	// dislay initialisation
	lcd_command(lcd_config, CMD_LCD_SET_4BIT_2LINE);
	HAL_Delay(100);
	lcd_command (lcd_config, CMD_LCD_CONTROL_ON);
	HAL_Delay(100);
	lcd_command (lcd_config, CMD_LCD_CLEAR);
	HAL_Delay(100);
	lcd_command (lcd_config, CMD_LCD_ENTRY_MODE_INC);
	HAL_Delay(100);
	lcd_command (lcd_config, CMD_LCD_CONTROL_ON);
}



void printText(LCD_Handle_t* lcd_config, char *Text, uint32_t delayvalue){
	lcd_command(lcd_config, CMD_LCD_CLEAR);
	HAL_Delay(delayvalue);
	// Print a text to the display
	char *tmp = Text;
	uint8_t counter = 0;

	while(*tmp != '\0')
	{
		printLetter(lcd_config, *tmp);
		tmp++;
		counter++;
		HAL_Delay(delayvalue);
		if(counter >= 16)
		{
			lcd_command(lcd_config, CMD_LCD_SET_SECOND_LINE);
			HAL_Delay(delayvalue);
			counter = 0;
		}
	}
}
