/*
 * hx711.c
 *
 *  Created on: Oct 16, 2025
 *      Author: 20128
 */


#include "hx711.h"
/* My HX711 CONNECTION TO MY STM32 YOU HAVE TO CHANGE IT to YOURS*/
//HX711_pin_Config my_hx711 = {
//    .DataPort = HX711_DATA_PIN_GPIO_Port,
//    .DataPin = HX711_DATA_PIN_Pin,
//    .ClockPort = HX711_CLK_PIN_GPIO_Port,
//    .ClockPin = HX711_CLK_PIN_Pin
//};

int32_t HX711_read(HX711_pin_Config * HX711){
	int32_t value = 0;
	/* wait for data be ready*/
	while(!HX711_is_ready(HX711));
	/* read 24 bit converted data */
	for(int i = 0 ; i <24 ; i++){
		/*send pulse to get a bit from 24 */
		HAL_GPIO_WritePin(HX711->ClockPort, HX711->ClockPin, GPIO_PIN_SET);
		value = value << 1 ;
		if(HAL_GPIO_ReadPin(HX711->DataPort, HX711->DataPin) == GPIO_PIN_SET) {
			/* if its 1 change it to 1 at the value if zero leave it */
			value++ ;
		}
		HAL_GPIO_WritePin(HX711->ClockPort, HX711->ClockPin, GPIO_PIN_RESET);
	}

	/*send 1 pulse to choose the 128 gain */
	HAL_GPIO_WritePin(HX711->ClockPort, HX711->ClockPin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(HX711->ClockPort, HX711->ClockPin, GPIO_PIN_RESET);

	// Convert to signed 24-bit
	if (value & 0x800000){
		value |= 0xFF000000;
	}


	return value ;
}

uint8_t HX711_is_ready(HX711_pin_Config * HX711){
	return (HAL_GPIO_ReadPin(HX711->DataPort, HX711->DataPin) == GPIO_PIN_RESET ) ;
}

uint16_t HX711_balance_waight(HX711_pin_Config * HX711){
	int32_t reading = HX711_read(HX711);

	return (reading - no_weight_reading) / scale;
}
