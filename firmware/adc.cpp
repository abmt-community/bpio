/*
 * adc.cpp
 *
 *  Created on: 01.10.2020
 *      Author: hva
 */

#include "mbed.h"
#include "adc.h"


volatile int32_t adc_values[6] = {0};

DMA_HandleTypeDef dma = {};
ADC_HandleTypeDef cfg = {0};

extern "C" {

void DMA1_Channel1_IRQHandler(void)
{
	HAL_DMA_IRQHandler(&dma);
}

} // extern C


void adc_init(){
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_1;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_2;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_5;
	GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	__HAL_RCC_DMA1_CLK_ENABLE();

	dma.Instance 					= DMA1_Channel1;
	dma.Init.Direction 				= DMA_PERIPH_TO_MEMORY;
	dma.Init.PeriphInc 				= DMA_PINC_DISABLE;
	dma.Init.MemInc 				= DMA_MINC_ENABLE;
	dma.Init.PeriphDataAlignment 	= DMA_PDATAALIGN_WORD;
	dma.Init.MemDataAlignment 		= DMA_MDATAALIGN_WORD;
	dma.Init.Mode 					= DMA_CIRCULAR;
	dma.Init.Priority 				= DMA_PRIORITY_VERY_HIGH;

	if (HAL_DMA_Init(&dma) != HAL_OK){
	}
	__HAL_LINKDMA(&cfg, DMA_Handle, dma);

	HAL_NVIC_SetPriority(DMA1_Channel1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Channel1_IRQn);


	__HAL_RCC_ADC1_CLK_ENABLE();

	cfg.Instance = ADC1;
	cfg.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	cfg.Init.ScanConvMode          = ADC_SCAN_ENABLE;
	cfg.Init.ContinuousConvMode    = DISABLE;
	cfg.Init.NbrOfConversion       = 6;
	cfg.Init.DiscontinuousConvMode = DISABLE;
	cfg.Init.NbrOfDiscConversion   = 0;
	cfg.Init.ExternalTrigConv 	   = ADC_SOFTWARE_START;
	cfg.DMA_Handle = &dma;

	if (HAL_ADC_Init(&cfg) != HAL_OK) {
		// error
	}


	ADC_ChannelConfTypeDef sConfig = {0};
	sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;

	sConfig.Channel = ADC_CHANNEL_0;
	sConfig.Rank = 1;

	if (HAL_ADC_ConfigChannel(&cfg, &sConfig) != HAL_OK){
		// error
	}

	sConfig.Channel = ADC_CHANNEL_1;
	sConfig.Rank = 2;
	if (HAL_ADC_ConfigChannel(&cfg, &sConfig) != HAL_OK){
	}

	sConfig.Channel = ADC_CHANNEL_2;
	sConfig.Rank = 3;
	if (HAL_ADC_ConfigChannel(&cfg, &sConfig) != HAL_OK){
		// error
	}

	sConfig.Channel = ADC_CHANNEL_3;
	sConfig.Rank = 4;
	if (HAL_ADC_ConfigChannel(&cfg, &sConfig) != HAL_OK){
		// error
	}

	sConfig.Channel = ADC_CHANNEL_4;
	sConfig.Rank = 5;
	if (HAL_ADC_ConfigChannel(&cfg, &sConfig) != HAL_OK){
		// error
	}

	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = 6;

	if (HAL_ADC_ConfigChannel(&cfg, &sConfig) != HAL_OK){
		// error
	}

	 __HAL_DMA_DISABLE_IT(&dma, DMA_IT_HT);

}

void adc_measure(){
	// 6*2 see note in DMA1_Channel1_IRQHandler
	if (HAL_ADC_Start_DMA(&cfg, (uint32_t*) adc_values, 6)!= HAL_OK){
		// error
	}
}
