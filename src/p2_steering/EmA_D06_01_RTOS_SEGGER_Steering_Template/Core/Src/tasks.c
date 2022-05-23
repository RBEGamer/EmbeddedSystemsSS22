#include <stdio.h>
#include "main.h"
#include "MEMS_sensor.h"
#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t task1_handle;


void task1(void* parameters){

	while (1){
	}
}



/*// Code for readout the acceleration sensor:
 *
 // Initialization and variables
 * float ax = 0.f, ay = 0.f, az = 0.f;
 * uint8_t status = 0;
 * float scaling = 2.f/32768;
 * int16_t acceleration[3];
 * init_mems();
 *
 * // readout the acceleration sensor values
 * platform_read(status_, &status, 1);
 * if((status & 0x8) >> 3 == 1) // data ready
 * {
 * read_mems(acceleration);
 * ax = acceleration[0]*scaling;
 * ay = acceleration[1]*scaling;
 * az = acceleration[2]*scaling;
 * }
 *
 * // A threshold of +-0.7 for az and ax should be suitable, so
 *  az >  0.7 signals UP movement (orange LED, LD3 )
 *  az < -0.7 signals DOWN movement (blue LED, LD6)
 *  ax > 0.7 signals RIGHT movement (red LED, LD5)
 *  ax <-0.7 signals LEFT movement (green LED, LD4)
 */


void ITM_print(const char* msg, const uint8_t len){
	for(int i = 0; i < len ; i++){
		ITM_SendChar(msg[i]);
		if (msg[i] == '\0'){
			break;
		}
	}
}


