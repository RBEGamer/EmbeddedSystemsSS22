/*
 * LCD_I2C_Driver.h
 */

#ifndef INC_LCD_I2C_DRIVER_H_
#define INC_LCD_I2C_DRIVER_H_

#include "stm32f4xx_hal.h"

#define CMD_LCD_CLEAR					0b00000001
#define CMD_LCD_CONTROL_ON 				0b00001111	// Display on, cursor on and blinking
#define CMD_LCD_CONTROL_OFF 			0b00001000	// Display of, cursor off and blinking
#define CMD_LCD_SET_4BIT_2LINE 			0b00101000
#define CMD_LCD_SET_8BIT_2LINE 			0b00111000
#define CMD_LCD_RETURN_HOME				0b00000010
#define CMD_LCD_SET_SECOND_LINE			0b11000000
#define CMD_LCD_ENTRY_MODE_INC			0b00000110
#define CMD_LCD_ENTRY_MODE_SHIFT		0b00000111
#define CMD_LCD_SHIFT_CURSOR_LEFT		0b00010000
#define CMD_LCD_SHIFT_CURSOR_RIGHT		0b00010100

#define BG_BIT_POSITION					3

/*
 * Handle struct für LCD
 */
typedef struct
{
	I2C_HandleTypeDef *i2c_handle;
	uint16_t Slave_address;
	uint8_t bg; // Background switch
}LCD_Handle_t;


// Sendet das Kommando in cmd angegebene Kommando an das Display (instruction Mode)
// und führt es aus.
void lcd_command(LCD_Handle_t* lcd_config, uint8_t cmd);

// Inistialiserung des Displays
void lcd_init(LCD_Handle_t* lcd_config);

void printLetter(LCD_Handle_t* lcd_config, uint8_t letter);

void printText(LCD_Handle_t* lcd_config, char *Text, uint32_t delayvalue);


#endif /* INC_LCD_I2C_DRIVER_H_ */



