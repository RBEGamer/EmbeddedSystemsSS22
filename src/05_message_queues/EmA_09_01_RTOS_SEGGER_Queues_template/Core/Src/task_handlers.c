/*
 * task_handlers.c
 *      Author: TDey
 */
#include "main.h"


#define ADC_BIN_SIZE  					410
#define CONTROL_MSG_INDEX_SHUTDOWN  	10
#define CONTROL_MSG_INDEX_WELCOME  		11


const char msg_control[12][32] = {
		"Vol 0 ",
		"Vol 1 #",
		"Vol 2 ##",
		"Vol 3 ###",
		"Vol 4 ####",
		"Vol 5 #####",
		"Vol 6 ######",
		"Vol 7 #######",
		"Vol 8 ########",
		"Vol 9 #########",
		"Shutdown Bye Bye",
		"Starting ...    Welcome!"};

static void ITM_print(const char* msg, const uint8_t len){
	for(int i = 0; i < len ; i++){
		ITM_SendChar(msg[i]);
		if (msg[i] == '\0'){
			break;
		}
	}
}


void control_task_Handler(void* parameters){


	uint32_t state = 0;
	uint32_t ulNotifiedValue = 0;
	while(1){
		HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, 0);
		//if(ulTaskNotifyTake(1,portMAX_DELAY)){
		if(xTaskNotifyWait(0,0, 0, portMAX_DELAY) == pdTRUE){
			HAL_GPIO_TogglePin(LD5_GPIO_Port, LD5_Pin);

			if(state == 0){
				state = 1;
				vTaskSuspend(process_task_handle);
				char* ptr = msg_control[CONTROL_MSG_INDEX_SHUTDOWN];
				xQueueSendToBack( xQueue_print, &ptr,( TickType_t ) 10 );
			}else{
				state = 0;
				char* ptr = msg_control[CONTROL_MSG_INDEX_WELCOME];
				xQueueSendToBack( xQueue_print, &ptr,( TickType_t ) 10 );
				vTaskResume(process_task_handle);

			}

		};



		vTaskDelay(10);
	}


}

void measure_task_Handler(void* parameters){
	uint32_t counter = 0;
	while (1){
		//counter++;
		vTaskDelay(2);

		HAL_ADC_Start(&hadc1);
		if(HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK){
			counter = HAL_ADC_GetValue(&hadc1);
			counter = (counter*CONTROL_MSG_INDEX_SHUTDOWN / 4096);



		if(uxQueueSpacesAvailable(xQueue1) > 0){
		//	HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, ENABLE);
			xQueueSendToBack( xQueue1,( void * ) &counter,( TickType_t ) 10 );
		}else{
			//HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, 0);
		}
		}

	}
}

void process_task_Handler(void* parameters){
	char msg[64];
	uint32_t value = 0;
	uint32_t last = 0;
	while (1){

		if(xQueueReceive( xQueue1,( void * ) &value,( TickType_t ) 10 ) == pdTRUE){
			//HAL_GPIO_TogglePin(LD6_GPIO_Port, LD6_Pin);
			if(last != value){
				last = value;

			char* ptr = msg_control[value];

			if(uxQueueSpacesAvailable(xQueue_print) > 0){
				xQueueSendToBack( xQueue_print,&ptr,( TickType_t ) 10 );
					}

			}
		}

		snprintf(msg,64,"Value: %lu \n", value);
		ITM_print(msg, 64);
		vTaskDelay(1);

	}
}

void print_task_Handler(void* parameters){
	uint32_t* bufferPtr;

	while (1){
		if(xQueueReceive( xQueue_print,(uint32_t*) &bufferPtr,( TickType_t ) 10 ) == pdTRUE){
			printText(&lcd_handle, bufferPtr, 5);

		}

		vTaskDelay(10);
	}

}



