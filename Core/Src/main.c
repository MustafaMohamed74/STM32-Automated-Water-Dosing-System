/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hx711.h"
#include "stdio.h"
#include "stdint.h"
#include "inttypes.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
extern UART_HandleTypeDef huart1;
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define water_ratio        2

#define pulses_per_liter   350
#define ml_per_liter       1000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int __io_putchar(int ch);
void start_dosing(void);
void stop_dosing(void);
void top_up_dosing(void);

uint16_t calculate_desired_water_amount(uint16_t ingredient_grams);
uint16_t map_ml_to_pulses(uint16_t desired_ml);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint16_t pulseCount ;
HX711_pin_Config my_hx711 ;
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
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */
	HAL_GPIO_WritePin(solenoid_valve_GPIO_Port, solenoid_valve_Pin, GPIO_PIN_RESET);

	my_hx711.ClockPort = HX711_CLK_GPIO_Port;
	my_hx711.ClockPin = HX711_CLK_Pin ;
	my_hx711.DataPort = HX711_DATA_GPIO_Port ;
	my_hx711.DataPin = HX711_DATA_Pin ;

//	int32_t reading = 0 ;
	uint16_t grams = 0 ;
	uint16_t water_ml = 0 ;
	uint16_t desired_pulses = 0 ;
	uint8_t flag = 1 ;

	uint8_t last_stop_state = 1 ;
	uint8_t current_stop_state = 0 ;

	uint8_t last_start_state = 1 ;
	uint8_t	current_start_state = 0;

	uint8_t last_topup_state = 1 ;
	uint8_t	current_topup_state = 0;

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */
//		printf("65\n");
		current_stop_state  =  HAL_GPIO_ReadPin(Stop_button_GPIO_Port, Stop_button_Pin);
		current_start_state = HAL_GPIO_ReadPin(Start_button_GPIO_Port, Start_button_Pin) ;
		current_topup_state = HAL_GPIO_ReadPin(Top_up_button_GPIO_Port, Top_up_button_Pin) ;
		/* USER CODE BEGIN 3 */
		if (current_stop_state == 0 && last_stop_state == 1 ){
			stop_dosing();
		}
		else if (current_start_state == 0 && last_start_state == 1) {
			pulseCount = 0 ;
			grams = HX711_balance_waight(&my_hx711);
//		    printf("grams = %" PRId16 "\n", grams);
			water_ml = calculate_desired_water_amount(grams);
//		     printf("water_ml = %" PRId16 "\n", water_ml);

			desired_pulses = map_ml_to_pulses(water_ml);
//			printf("desired_pulses = %" PRId16 "\n", desired_pulses);
			start_dosing();
		}
		else if((current_topup_state == 0 && last_topup_state == 1)&& (flag == 1 )){
			if(HAL_GPIO_ReadPin(Top_up_button_GPIO_Port, Top_up_button_Pin) == GPIO_PIN_RESET){
			top_up_dosing();
			flag = 0;
			}
		}
		else if ((current_topup_state == 1 && last_topup_state == 1)&& flag == 0){
			if ((HAL_GPIO_ReadPin(Top_up_button_GPIO_Port, Top_up_button_Pin) == GPIO_PIN_SET)&& flag == 0){
			stop_dosing();
			flag = 1 ;
			}
		}

		if (pulseCount > desired_pulses){
			stop_dosing();
			pulseCount = 0 ;
		}

		last_stop_state = current_stop_state;
		last_start_state = current_start_state;
		last_topup_state = current_topup_state;


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
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */
int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == water_flow_sensor_Pin)
	{
		pulseCount ++ ;
		printf("%" PRId16 "\n", pulseCount);
	}
}

void start_dosing(void){
	HAL_GPIO_WritePin(solenoid_valve_GPIO_Port, solenoid_valve_Pin, GPIO_PIN_SET);
	printf("Start\n");
}
void stop_dosing(void){
	HAL_Delay(500);
	HAL_GPIO_WritePin(solenoid_valve_GPIO_Port, solenoid_valve_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);

	printf("Stop\n");
}
void top_up_dosing(void){
	HAL_GPIO_WritePin(solenoid_valve_GPIO_Port, solenoid_valve_Pin, GPIO_PIN_SET);
//	HAL_Delay(1000);
	printf("Srsob mya\n");
}

uint16_t calculate_desired_water_amount(uint16_t ingredient_grams){
	return (ingredient_grams * water_ratio);
}

uint16_t map_ml_to_pulses(uint16_t desired_ml)
{

    uint16_t pulses = (uint16_t)((desired_ml * pulses_per_liter) / ml_per_liter);
    return pulses;
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
