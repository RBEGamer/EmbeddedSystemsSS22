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
#include "i2c.h"
#include "rng.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_RND_VALUE 40000
#define MIN_RND_VALUE  5000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
uint8_t status = 0;
uint32_t reaction_time_in_ms = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_RNG_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_I2C2_Init();
  MX_I2C3_Init();
  /* USER CODE BEGIN 2 */
  // Settings for LCD-display via I2C
  LCD_Handle_t lcd_handle;
  lcd_handle.Slave_address = 0x4E;
  lcd_handle.i2c_handle = &hi2c1;
  lcd_handle.bg = 1;
  lcd_init(&lcd_handle);

  // Print welcome message
  char* text = "Willkommen in Embedded Software!\0";
  uint32_t delayValue = 100;
  printText(&lcd_handle, text, delayValue);
  reaction_time_in_ms = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//  status = STATUS_RANDOM_TIMER_READY;
	while (1)
	{
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

		switch(status){

		case STATUS_START_NEW_GAME:
			printText(&lcd_handle, "Starting Game!", delayValue);
			startNewGame();
			break;

		case STATUS_RANDOM_TIMER_READY:
			printText(&lcd_handle, "Get ready! ... ", delayValue);
			start_random_timer();
			break;
		case STATUS_RANDOM_TIMER_RUNNING:
			// Nothing to do :-)
			break;

		case STATUS_RANDOM_TIMER_EXPIRED:
			//STOP TIMER 6
			HAL_TIM_Base_Stop_IT(&htim6);
			//PRINT MSG
			printText(&lcd_handle, "Go!", delayValue);
			status = STATUS_MEASURE_REACTION_TIME;
			//SET LED
			HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);
			//START MEASUREMENT TIMER
			HAL_TIM_Base_Start_IT(&htim7);

			break;

		case STATUS_MEASUREMENT_DONE:
			HAL_TIM_Base_Stop_IT(&htim7);
			//reaction_time_in_ms = htim7.Instance->CNT / 10;
			//Switch off all LEDs
			switch_all_leds(DISABLE, 0);
			// Wait until showing the result.
			HAL_Delay(500);
			char textBuffer[32] = {0};
			//SWITCH LEDS OFF
			switch_all_leds(DISABLE, 0);
			//SWITCH DEPENDING SPEED
			if(reaction_time_in_ms < 500){
				snprintf (textBuffer, 32, "Time: %4u ms.  Well done!", (uint16_t) reaction_time_in_ms);
				HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, GPIO_PIN_SET);
			}else if(reaction_time_in_ms >= 500 && reaction_time_in_ms <= 1000){
				snprintf (textBuffer, 32, "Time: %4u ms.  Okay!", (uint16_t) reaction_time_in_ms);
				HAL_GPIO_WritePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin, GPIO_PIN_SET);
			}else{
				snprintf (textBuffer, 32, "Time: %4u ms.  Are you asleep?", (uint16_t) reaction_time_in_ms);
				HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, GPIO_PIN_SET);
			}

			printText(&lcd_handle, textBuffer, delayValue);

			HAL_Delay(500);
			//STAQRT NEW GAME
			status =STATUS_START_NEW_GAME;
			break;

		case STATUS_INVALID_MEASUREMENT:


			printText(&lcd_handle, "Invalid Measurement!", delayValue);

			for(int i = 0; i < 4; i++){
				switch_all_leds(ENABLE, 0);
				HAL_Delay(500);
				switch_all_leds(DISABLE, 0);
				HAL_Delay(500);
			}
			HAL_Delay(3000);
			status = STATUS_START_NEW_GAME;
			break;
		}
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
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
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV16;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void start_random_timer(){


	//GET RANDOM NUMBER
	uint32_t rng = 0;
	HAL_RNG_GenerateRandomNumber(&hrng, &rng);
	//MAP 0-65536 => MIN_RND_VALUE-MAX_RND_VALUE
	rng = ((uint16_t)rng) * (MAX_RND_VALUE - MIN_RND_VALUE) / 65536 + MIN_RND_VALUE;

	//SET PERIOD
	//START TIMER FIRST => INIT AGAIN WITH NEW PERIOD
	HAL_TIM_Base_Start_IT(&htim6);
	htim6.Init.Period = rng;
	//htim6.Instance->CNT = 0;
	//htim6.Instance->ARR  = MAX_RND_VALUE;
	HAL_TIM_Base_Init(&htim6);

	//SET STATE
	HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin,GPIO_PIN_SET);
	status = STATUS_RANDOM_TIMER_RUNNING;


}




void switch_all_leds(uint8_t ENorDI, uint32_t delay_value){

	if(delay_value <= 0){
		HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, ENorDI);
		HAL_GPIO_WritePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin, ENorDI);
		HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, ENorDI);
		HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, ENorDI);
	}else{

		//SWITCH SEQUENCE
		HAL_GPIO_WritePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin, ENorDI);
		HAL_Delay(500);
		HAL_GPIO_WritePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin, ENorDI);
		HAL_Delay(500);
		HAL_GPIO_WritePin(RED_LED_GPIO_Port, RED_LED_Pin, ENorDI);
		HAL_Delay(500);
		HAL_GPIO_WritePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin, ENorDI);
		HAL_Delay(500);
	}
}

void led_new_game(){
	//Switch on LEDs (LD4,LD3,LD5,LD6 subsequently with a delay of 500 ms)
	switch_all_leds(ENABLE, 500);
	// Switch all LEDs off without delay
	switch_all_leds(DISABLE,0);
	//Switch blue LED on to indicate that the game ist starting.
}

void startNewGame(){
	led_new_game();
	status = STATUS_RANDOM_TIMER_READY;
	reaction_time_in_ms = 0;
}
/* USER CODE END 4 */

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
