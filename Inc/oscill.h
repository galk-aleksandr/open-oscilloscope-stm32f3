#ifndef __OSCILL_H
#define __OSCILL_H

#include "stm32f3xx_hal.h"

#define FLAG_TRIGGERED	0x8000
#define FLAG_NEW 				0x4000
#define FLAG_CLEAR 			0x2000
#define FLAG_NO_DATA		0x1000

#define NO_DATA 0x8000

#define FRAME_SIZE 1535
#define BUFFER_SIZE 3800

extern uint8_t OscillConfigDataNaked[];
extern uint8_t OscillConfigDataShielded[];
extern uint16_t bufferADma[BUFFER_SIZE];
extern uint16_t bufferBDma[BUFFER_SIZE];
extern uint16_t bufferCDma[BUFFER_SIZE];

extern uint16_t bufferD[FRAME_SIZE * 3 + 1];

extern OPAMP_HandleTypeDef hopamp1;
extern OPAMP_HandleTypeDef hopamp3;
extern OPAMP_HandleTypeDef hopamp4;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc3;
extern ADC_HandleTypeDef hadc4;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc3;
extern DMA_HandleTypeDef hdma_adc4;

extern DAC_HandleTypeDef hdac;
extern DMA_HandleTypeDef hdma_dac_ch1;

extern UART_HandleTypeDef huart2;

void initOscill(void);
void sendBuffer(void);

void setGain(char opamp_letter, char g);
void setTiming(char t);
void setDiv(char channel_letter, char d);

void setTriggerType(char sign);
void setTriggerChannel(char sign);
void setTriggerLevel(char *value, size_t length);
void setTriggerTimeShift(char *value, size_t length);


void setGenShape(char shape);
void setGenBuff(char buff);
void setGenFreq(char *value, size_t length);
void setGenAmpl(char *value, size_t length);

#endif

