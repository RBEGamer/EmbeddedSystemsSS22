/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under Ultimate Liberty license
 * SLA0044, the "License"; You may not use this file except in compliance with
 * the License. You may obtain a copy of the License at:
 *                             www.st.com/SLA0044
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "stdio.h"
#include "MEMS_sensor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
TaskHandle_t task1_Control_handle;
TaskHandle_t task2_MEMS_handle;
TaskHandle_t task3_LED_handle;

QueueHandle_t xQueue_led;
QueueHandle_t xQueue_reading;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define STATUS_GAME_INIT 					0
#define STATUS_GAME_WAIT_FOR_START			1
#define STATUS_GAME_WAIT_FOR_START_BLINK 	2
#define STATUS_GAME_START_GAME				3
#define STATUS_RANDOM_TIMER_RUNNING			4
#define STATUS_RANDOM_TIMER_EXPIRED			5
#define STATUS_MEASUREMENT_RUNNING	 		6
#define STATUS_MEASUREMENT_DONE 			7
#define STATUS_INVALID_MEASUREMENT			8

#define LED_OFF 0
#define LED_3 	1
#define LED_4 	2
#define LED_5 	3
#define LED_6 	4
#define LED_7 	5
#define LED_8 	6


#define DIRECTION_INVALID 	LED_OFF
#define DIRECTION_UP 		LED_3
#define DIRECTION_DOWN 		LED_6
#define DIRECTION_LEFT 		LED_4
#define DIRECTION_RIGHT		LED_5



#define MAX_RND_VALUE 4000
#define MIN_RND_VALUE 1000

const int16_t ThresholdHigh = 8000;
const int16_t ThresholdLow = -8000;
const uint32_t ThresholdGen = 8000;
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define DWT_CTRL	(*(volatile uint32_t*) 0xE0001000)

uint8_t status = STATUS_GAME_INIT;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void task1(void* parameters);
void ITM_print(const char* msg, const uint8_t len);

extern void SEGGER_UART_init(unsigned long baud);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void start_random_timer(){
	//GET RANDOM NUMBER
	uint32_t rng = 0;
	HAL_RNG_GenerateRandomNumber(&hrng, &rng);
	//MAP 0-65536 => MIN_RND_VALUE-MAX_RND_VALUE
	rng =  (rng%3000);
	rng += 1000;


	//SET PERIOD
	//START TIMER FIRST => INIT AGAIN WITH NEW PERIOD
	HAL_TIM_Base_Start_IT(&htim6);
	htim6.Init.Period = rng;
	//htim6.Instance->CNT = 0;
	//htim6.Instance->ARR  = MAX_RND_VALUE;
	HAL_TIM_Base_Init(&htim6);
	HAL_GPIO_WritePin(TRG_GPIO_Port, TRG_Pin,1);
}

void start_fail_timer(){


	//SET PERIOD
	//START TIMER FIRST => INIT AGAIN WITH NEW PERIOD
	//HAL_TIM_Base_Start_IT(&htim7);
	//htim7.Init.Period = 2000;
//	htim7.Instance->CNT = 0;
	//htim6.Instance->ARR  = MAX_RND_VALUE;
//	HAL_TIM_Base_Init(&htim7);
	HAL_TIM_Base_Start_IT(&htim7);
}

uint32_t get_random_direction(){
	//GET RANDOM NUMBER
	uint32_t rng = 0;
	HAL_RNG_GenerateRandomNumber(&hrng, &rng);
	//MAP 0-65536 => 1-4
	rng =  (((uint16_t)rng) % 4)+1; //RND VALUE = LED INDEX SO +1
	return rng;
}


uint32_t mems_to_direction(){
	int16_t buffer[3] = {0};
	int16_t xval, yval = 0x00;

	  /* Read Acceleration */
	read_mems(&buffer);

	  xval = buffer[0];
	  yval = buffer[2];

	  if((abs(xval))> ThresholdGen)
	  {
	    if(xval > ThresholdHigh)
	    {
	    	return DIRECTION_RIGHT;
	    }
	    else if(xval < ThresholdLow)
	    {
	    	return DIRECTION_LEFT;
	    }

	  }

	  if(abs(yval) > ThresholdGen)
	  {
	    if(yval < ThresholdLow)
	    {
	    	return DIRECTION_DOWN;
	    }
	    else if(yval > ThresholdHigh)
	    {
	    	return DIRECTION_UP;
	    }

	  }

	  return DIRECTION_INVALID;
}

void task2_MEMS_Handler(void* parameters){
	uint32_t read_direction = 0;
	uint32_t last_dir = 0;
	uint32_t trg = 0;
	while (1){
		vTaskDelay(10);
		//GET READING
		read_direction = mems_to_direction();

		if(read_direction != last_dir){
			trg = 0;
			last_dir = read_direction;
		}else{
			trg++;
		}
		//SEND IT
		//SEND MESSAGE ONLY IF 10 SAME READINGS ARE READ
		if(trg>10){
				xQueueSendToFront( xQueue_reading,&read_direction,( TickType_t ) 10 );

				trg = 0;
		}
	}
}



void send_led_set_message(uint32_t _led){
	if(uxQueueSpacesAvailable(xQueue_led) > 0){
		xQueueSendToBack( xQueue_led,&_led,( TickType_t ) 10 );
	}
}
void task1_Control_Handler(void* parameters){
	//char msg[64];
	//uint32_t value = 0;
	uint32_t event_val = 0;
	uint32_t event_val_last = 0;
	uint8_t c= 0;
	uint32_t direction = 0;
	while (1){



		if(status == STATUS_GAME_INIT){
			//STOP TIMERS
			HAL_TIM_Base_Stop_IT(&htim7);
			HAL_TIM_Base_Stop_IT(&htim6);
			//HAL_TIM_Base_Stop_IT(&htim8);
			//STOP MEASUREMENT TASK
			vTaskSuspend(task2_MEMS_handle);
			//RESET LED
			send_led_set_message(LED_OFF);

			//SET NEW STATUS
			status = STATUS_GAME_WAIT_FOR_START;
		}else if(status == STATUS_GAME_WAIT_FOR_START){
			//WAIT FOR BUTTON NOTIFY EVENT
			if(xTaskNotifyWait(0,0, &event_val, portMAX_DELAY) == pdTRUE && event_val == 1){
				send_led_set_message(LED_7);
				status = STATUS_GAME_WAIT_FOR_START_BLINK;
			}
			//THIS STATE WAS ADDED TO RESTART THE GAME AS ENTRY POINT
		}else if(status == STATUS_GAME_WAIT_FOR_START_BLINK){
			send_led_set_message(LED_7);
			//BLINK GREEN LED
			for(c = 0; c < 3; c++){
				send_led_set_message(LED_OFF);
				vTaskDelay( 500/portTICK_PERIOD_MS );
				send_led_set_message(LED_7);
				vTaskDelay( 500/portTICK_PERIOD_MS );
			}
			send_led_set_message(LED_OFF);
			//START RANDOM TIMER
			status = STATUS_GAME_START_GAME;
		}else if(status == STATUS_GAME_START_GAME){
			//SET AND START RANDOM TIMER
			send_led_set_message(LED_OFF);
			start_random_timer();
			status = STATUS_RANDOM_TIMER_RUNNING;

		}else if(status == STATUS_RANDOM_TIMER_RUNNING){
			//WAIT FOR NOTIFY
			if(xTaskNotifyWait(0,0, &event_val, portMAX_DELAY) == pdTRUE && event_val == 3){
				//STOP RANDOM TIMER
				HAL_TIM_Base_Stop_IT(&htim6);
				//SET NEW STATE
				status = STATUS_RANDOM_TIMER_EXPIRED;
			}

		}else if(status == STATUS_RANDOM_TIMER_EXPIRED){

			//SWITCH DIRECTION LED ON
			direction = get_random_direction();
			send_led_set_message(direction);
			//START NEW TIMER
			start_fail_timer();
			//START MEASUREMENT TASK
			vTaskResume(task2_MEMS_handle);
			//SET STATE
			status = STATUS_MEASUREMENT_RUNNING;


		}else if(status == STATUS_MEASUREMENT_RUNNING){
			//TIMER TIMEOUT
			event_val = 0;
			if(xTaskNotifyWait(0,0, &event_val, ( TickType_t ) 10 ) == pdTRUE && event_val == 2){
				//STOP RANDOM TIMER
				HAL_TIM_Base_Stop_IT(&htim7);
				//SET NEW STATE
				send_led_set_message(LED_8);
				vTaskDelay( 2000/portTICK_PERIOD_MS );
				//RESTART GAME
				status = STATUS_GAME_WAIT_FOR_START_BLINK;
			}
			//GET A VALID MOVEMENT RESULT
			event_val = 0;
			if(xQueueReceive( xQueue_reading,( void * ) &event_val,( TickType_t ) 10 ) == pdTRUE && event_val != DIRECTION_INVALID){

				//DISPLAY RESULT
				if(event_val == direction){
					send_led_set_message(LED_7);
				}else{
					send_led_set_message(LED_8);
				}

				//WAIT
				vTaskDelay( 2000/portTICK_PERIOD_MS );
				//RESTART GAME
				status = STATUS_GAME_WAIT_FOR_START_BLINK;
			}
		}



		vTaskDelay(5);

	}
}


void set_leds_off(){
	HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 0);
	HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, 0);
	HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, 0);
	HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, 0);
	HAL_GPIO_WritePin(LD7_GPIO_Port, LD7_Pin, 0);
	HAL_GPIO_WritePin(LD8_GPIO_Port, LD8_Pin, 1);
}

void task3_LED_Handler(void* parameters){
	uint32_t led_value = 0;
	uint32_t last = 99;
	while (1){

		if(xQueueReceive( xQueue_led,( void * ) &led_value,( TickType_t ) 10 ) == pdTRUE && led_value != last){
			last = led_value;
			//SWITCH LEDS OFF
			set_leds_off();
			//SET TO NEW VALUE
			switch(led_value){
			case LED_3:{ HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, 1);break;}
			case LED_4:{ HAL_GPIO_WritePin(LD4_GPIO_Port, LD4_Pin, 1);break;}
			case LED_5:{ HAL_GPIO_WritePin(LD5_GPIO_Port, LD5_Pin, 1);break;}
			case LED_6:{ HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, 1);break;}
			case LED_7:{ HAL_GPIO_WritePin(LD7_GPIO_Port, LD7_Pin, 1);break;}
			case LED_8:{ HAL_GPIO_WritePin(LD8_GPIO_Port, LD8_Pin, 0);break;}
			default: set_leds_off();break;
			}
		}
		vTaskDelay(10);
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	BaseType_t status;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
  xQueue_led = xQueueCreate( 1, sizeof( uint32_t ) );
  xQueue_reading = xQueueCreate( 1, sizeof( uint32_t ) );
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_RNG_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
	DWT_CTRL |= (1 << 0);

	SEGGER_SYSVIEW_Conf();
	SEGGER_SYSVIEW_Start();

	//INIT MEMS
	init_mems();
	set_leds_off();
	//START TASKS
	status = xTaskCreate(task2_MEMS_Handler,"task2_MEMS_Handler",200, "task2_MEMS_Handler", 1, &task2_MEMS_handle);
	configASSERT(status==pdPASS);

	status = xTaskCreate(task1_Control_Handler,"task1_Control_Handler",200, "task1_Control_Handler", 2, &task1_Control_handle);
	configASSERT(status==pdPASS);

	status = xTaskCreate(task3_LED_Handler,"task3_LED_handle",200, "task3_LED_handle", 3, &task3_LED_handle);
	configASSERT(status==pdPASS);

	vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM8 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM8) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
