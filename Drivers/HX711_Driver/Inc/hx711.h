/*
 * hx711.h
 *
 *  Created on: Oct 16, 2025
 *      Author: 20128
 */

#ifndef HX711_DRIVER_INC_HX711_H_
#define HX711_DRIVER_INC_HX711_H_

#include "stm32f4xx_hal.h"
#include "gpio.h"
#define no_weight_reading  368200
//#define weight_reading     411150
//#define weight             60
#define scale              (42950/60)
typedef struct {
	GPIO_TypeDef * DataPort ;
	uint16_t DataPin ;
	GPIO_TypeDef * ClockPort ;
	uint16_t ClockPin ;
}HX711_pin_Config;

int32_t HX711_read(HX711_pin_Config * HX711);
uint8_t HX711_is_ready(HX711_pin_Config * HX711);
uint16_t HX711_balance_waight(HX711_pin_Config * HX711);

#endif /* HX711_DRIVER_INC_HX711_H_ */
