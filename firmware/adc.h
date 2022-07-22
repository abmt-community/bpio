/*
 * adc.h
 *
 *  Created on: 01.10.2020
 *      Author: hva
 */

#ifndef ADC_H_
#define ADC_H_

#include "mbed.h"
#include <cstdint>
#include "DigitalOut.h"

extern volatile int32_t adc_values[6];// only 6 are used
void adc_init();
void adc_measure();

#endif /* ADC_H_ */
