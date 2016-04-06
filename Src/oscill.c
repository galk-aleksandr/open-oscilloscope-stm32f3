#include "string.h"
#include "stdbool.h"
#include "oscill.h"
#include "usbd_oscill_if.h"
#include "arm_math.h"

uint16_t bufferADma[BUFFER_SIZE];
uint16_t bufferBDma[BUFFER_SIZE];
uint16_t bufferCDma[BUFFER_SIZE];

uint16_t bufferD[FRAME_SIZE * 3 + 1];

volatile bool ongoingFrameClear = false;

char triggerType = 'R';
char triggerChannel = 'Z';
int triggerTimeShift = 0;
int triggerLevel = 0;
volatile uint32_t triggerLevelReg2 = 0xFFFFFFFF;


ADC_HandleTypeDef * getTriggerADC(){
	switch(triggerChannel) {
		case 'B': return &hadc3;
		case 'C': return &hadc4;
	}
	return &hadc1;
}
static void setupTrigger() {
	hadc1.Instance->CFGR &= ~ADC_CFGR_AWD1EN;
	hadc3.Instance->CFGR &= ~ADC_CFGR_AWD1EN;
	hadc4.Instance->CFGR &= ~ADC_CFGR_AWD1EN;
	if(triggerChannel == 'Z') {
			bufferD[0] |= FLAG_TRIGGERED;
	} else {
		bufferD[0] &= ~FLAG_TRIGGERED;
		__HAL_TIM_SetAutoreload(&htim3, FRAME_SIZE + triggerTimeShift);
		__HAL_TIM_SetCounter(&htim3, 0);
		ADC_HandleTypeDef * hadc = getTriggerADC();
		if(triggerType == 'R') {
			hadc->Instance->TR1 = __HAL_ADC_TRX_HIGHTHRESHOLD(0xFFF) | triggerLevel;
			triggerLevelReg2 = __HAL_ADC_TRX_HIGHTHRESHOLD(triggerLevel);
		} else {
			hadc->Instance->TR1 = __HAL_ADC_TRX_HIGHTHRESHOLD(triggerLevel);
			triggerLevelReg2 = __HAL_ADC_TRX_HIGHTHRESHOLD(0xFFF) | triggerLevel;
		}
		hadc->Instance->CFGR |= ADC_CFGR_AWD1EN;
	}
}

void setTriggerType(char sign) {
	triggerType = sign;
	setupTrigger();
}

void setTriggerChannel(char sign) {
	triggerChannel = sign;
	setupTrigger();
}

int parseDecimal(char* buf, size_t len) {
	bool minus = false;
	int val = 0;
	for(int i = 0; i < len; i++) {
	  if(buf[i]  == '-') {
			minus = true; 
		} else if(buf[i]  >= '0' && buf[i]  <= '9' ) {
			val = val*10 + buf[i] -'0';
		} else break;
	}
	return minus ? -val : val;
}

void setTriggerLevel(char *value, size_t length) {
	triggerLevel = parseDecimal(value,length);
	setupTrigger();
}

void setTriggerTimeShift(char *value, size_t length) {
	triggerTimeShift = parseDecimal(value,length);
	setupTrigger();
}


void copyMem(size_t dstOffset, void * src, size_t wordCount)
{
	memcpy(&bufferD[1 + dstOffset], src, wordCount * 2);
}

void channelTrigger() {
	HAL_TIM_Base_Stop(&htim1);
	HAL_TIM_Base_Stop(&htim3);
	bufferD[0] |= FLAG_NO_DATA;
	int counter = hadc1.DMA_Handle->Instance->CNDTR;
	
	if(counter + FRAME_SIZE <= BUFFER_SIZE) {
			copyMem(             0, &bufferADma[ BUFFER_SIZE - counter - FRAME_SIZE], FRAME_SIZE);
			copyMem(    FRAME_SIZE, &bufferBDma[ BUFFER_SIZE - counter - FRAME_SIZE], FRAME_SIZE);
			copyMem(2 * FRAME_SIZE, &bufferCDma[ BUFFER_SIZE - counter - FRAME_SIZE], FRAME_SIZE);
	} else {
		  const int cnt1 = BUFFER_SIZE - counter;
		  const int cnt2 = FRAME_SIZE - cnt1;
			copyMem(             0, &bufferADma[BUFFER_SIZE - cnt2], cnt2);
			copyMem(    FRAME_SIZE, &bufferBDma[BUFFER_SIZE - cnt2], cnt2);
			copyMem(2 * FRAME_SIZE, &bufferCDma[BUFFER_SIZE - cnt2], cnt2);
		
			copyMem(		FRAME_SIZE - cnt1, bufferADma, cnt1);
			copyMem(2 *	FRAME_SIZE - cnt1, bufferBDma, cnt1);
			copyMem(3 *	FRAME_SIZE - cnt1, bufferCDma, cnt1);
	}
	bufferD[0] = FLAG_NEW | FLAG_TRIGGERED;
  if( ongoingFrameClear) {
		bufferD[0] |= FLAG_CLEAR;
	}
	HAL_TIM_Base_Start(&htim1);
	
	setupTrigger();
}

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc) {
	if(triggerLevelReg2 == 0xFFFFFFFF) {
		ongoingFrameClear = false;
		HAL_TIM_Base_Start(&htim3);
		hadc->Instance->CFGR &= ~ADC_CFGR_AWD1EN;
	} else {
		hadc->Instance->TR1 = triggerLevelReg2;
		triggerLevelReg2 = 0xFFFFFFFF;
	}
}

void initOscill() {
		{/*Fix cubeMX bug */
		hopamp4.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_VP3;
		HAL_OPAMP_Init(&hopamp4);
	}
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)bufferADma, BUFFER_SIZE);
	HAL_ADC_Start_DMA(&hadc3, (uint32_t*)bufferBDma, BUFFER_SIZE);
	HAL_ADC_Start_DMA(&hadc4, (uint32_t*)bufferCDma, BUFFER_SIZE);
	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_Base_Start(&htim2);
	HAL_OPAMP_Start(&hopamp1);
	HAL_OPAMP_Start(&hopamp3);
	HAL_OPAMP_Start(&hopamp4);

  __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
	__HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
	
	bufferD[0] = FLAG_TRIGGERED | FLAG_CLEAR;
}

static void clearKeyFrames()
{
	bufferD[0] |= FLAG_CLEAR | FLAG_NEW;
	ongoingFrameClear = true;
}
typedef struct {
	__IO uint32_t * div2ODR;
	uint32_t div2mask;
	__IO uint32_t * div10ODR;
	uint32_t div10mask;
	__IO uint32_t * div30ODR;
	uint32_t div30mask;
} VoltDivBits;

static const VoltDivBits divBits[3] = {
{
	&OSCILLOSCOPE_A2_GPIO_Port->ODR , OSCILLOSCOPE_A2_Pin, 
	&OSCILLOSCOPE_A10_GPIO_Port->ODR, OSCILLOSCOPE_A10_Pin, 
	&OSCILLOSCOPE_A30_GPIO_Port->ODR, OSCILLOSCOPE_A30_Pin
},
{
	&OSCILLOSCOPE_B2_GPIO_Port->ODR , OSCILLOSCOPE_B2_Pin, 
	&OSCILLOSCOPE_B10_GPIO_Port->ODR, OSCILLOSCOPE_B10_Pin, 
	&OSCILLOSCOPE_B30_GPIO_Port->ODR, OSCILLOSCOPE_B30_Pin
},
{
	&OSCILLOSCOPE_C2_GPIO_Port->ODR, OSCILLOSCOPE_C2_Pin, 
	&OSCILLOSCOPE_C10_GPIO_Port->ODR, OSCILLOSCOPE_C10_Pin, 
	&OSCILLOSCOPE_C30_GPIO_Port->ODR, OSCILLOSCOPE_C30_Pin
}

};

void setDiv(char channel_letter, char d) {
	int chIdx = channel_letter - 'a';
	if(chIdx >=0 && chIdx < 4) {
		switch(d) {
			case '0':
				* divBits[chIdx].div2ODR  |= divBits[chIdx].div2mask;
				* divBits[chIdx].div10ODR |= divBits[chIdx].div10mask;
				* divBits[chIdx].div30ODR |= divBits[chIdx].div30mask;
				break;
			case '1':
				* divBits[chIdx].div2ODR  &= ~divBits[chIdx].div2mask;;
				* divBits[chIdx].div10ODR |= divBits[chIdx].div10mask;
				* divBits[chIdx].div30ODR |= divBits[chIdx].div30mask;
				break;
			case '2':
				* divBits[chIdx].div2ODR  |= divBits[chIdx].div2mask;
				* divBits[chIdx].div10ODR &= ~divBits[chIdx].div10mask;
				* divBits[chIdx].div30ODR |= divBits[chIdx].div30mask;
				break;
			case '3':
				* divBits[chIdx].div2ODR  |= divBits[chIdx].div2mask;
				* divBits[chIdx].div10ODR |= divBits[chIdx].div10mask;
				* divBits[chIdx].div30ODR &= ~divBits[chIdx].div30mask;
				break;
		}
	}
	clearKeyFrames();
}

void setGain(char opamp_letter, char g) {
	OPAMP_HandleTypeDef * opamp;
	switch(opamp_letter) {
		case	'a': opamp = &hopamp1; break;
		case	'b': opamp = &hopamp3; break;
		case	'c': opamp = &hopamp4; break;
		default: return;
	}
	opamp->Init.Mode = OPAMP_PGA_MODE;
	switch(g) {
			case '0': 
					opamp->Init.PgaGain = OPAMP_PGA_GAIN_2;
				break;
			case '1': 
					opamp->Init.PgaGain = OPAMP_PGA_GAIN_4;
				break;
			case '2':
					opamp->Init.PgaGain = OPAMP_PGA_GAIN_8; 
				break;
			case '3': 
					opamp->Init.PgaGain = OPAMP_PGA_GAIN_16; 
				break;
			default: 
					opamp->Init.Mode = OPAMP_FOLLOWER_MODE;
				break;
	}
	HAL_OPAMP_Init(opamp);
	HAL_OPAMP_Start(opamp);
	clearKeyFrames();
}

typedef struct {
	uint16_t prescaler;
	uint16_t arr;
	uint16_t samplingTime;
} TimeCoeff;

static const size_t timing_count = 18;
static const TimeCoeff timing[timing_count] =
{
{999, 7200, ADC_SAMPLETIME_601CYCLES_5},  // A => 10Hz
{99, 36000, ADC_SAMPLETIME_601CYCLES_5},  // B => 20Hz
{99, 14400, ADC_SAMPLETIME_601CYCLES_5},  // C => 50Hz
{99, 7200, ADC_SAMPLETIME_601CYCLES_5},   // D => 100Hz
{99, 3600, ADC_SAMPLETIME_601CYCLES_5},   // E => 200Hz
{99, 1440, ADC_SAMPLETIME_601CYCLES_5},   // F => 500Hz
{99, 720, ADC_SAMPLETIME_601CYCLES_5},    // G => 1kHz
{0, 36000, ADC_SAMPLETIME_601CYCLES_5},   // H => 2kHz
{0, 14400, ADC_SAMPLETIME_601CYCLES_5},   // I => 5kHz
{0, 7200, ADC_SAMPLETIME_601CYCLES_5},    // J => 10kHz
{0, 3600, ADC_SAMPLETIME_601CYCLES_5},    // K => 20kHz
{0, 1440, ADC_SAMPLETIME_601CYCLES_5},    // L => 50kHz
{0, 720, ADC_SAMPLETIME_601CYCLES_5},     // M => 100kHz
{0, 360, ADC_SAMPLETIME_181CYCLES_5},     // N => 200kHz
{0, 144, ADC_SAMPLETIME_19CYCLES_5},      // O => 500kHz
{0, 72, ADC_SAMPLETIME_19CYCLES_5},       // P => 1MHz
{0, 36, ADC_SAMPLETIME_19CYCLES_5},       // Q => 2MHz
{0, 14, ADC_SAMPLETIME_1CYCLE_5}          // R => 5MHz
};	

void setTiming(char t){
	if(t < 'A') t = 0; else  t -= 'A';
	if(t >= timing_count) t = timing_count - 1;
	htim1.Instance->PSC = timing[t].prescaler;
	htim1.Instance->ARR = timing[t].arr;
	htim1.Instance->CNT = 0;
	clearKeyFrames();
	//TODO ADC SAMPLING TIME
}

#define GEN_DMA_LENGTH 720
static uint16_t genDmaBuf[GEN_DMA_LENGTH];
static char genShape='N';
static char genBuff;
static int genFreq;
static int genAmpl;


void genFillConst(uint16_t ampl, uint16_t * buffer, size_t len)
{
	for(size_t i = 0; i < len; i++) {
		buffer[i] = ampl;
	}
}

void genFillTriangle() {
	for(size_t i = 0; i <= GEN_DMA_LENGTH / 2; i++) {
		genDmaBuf[i] = genDmaBuf[GEN_DMA_LENGTH - i] = (genAmpl * i) / (GEN_DMA_LENGTH / 2);
	}
}

void genFillSine() {
	for(size_t i = 0; i < GEN_DMA_LENGTH ; i++) {
		int32_t s = arm_sin_q15(0x7FFFL * i / GEN_DMA_LENGTH);
		genDmaBuf[i] = s * genAmpl / 2 / 0x8000L  + genAmpl / 2;
	}
}

void genFillSawtooth() {
	for(size_t i = 0; i < GEN_DMA_LENGTH ; i++) {
		genDmaBuf[i] = (genAmpl * i) / GEN_DMA_LENGTH;
	}
}

static void setupGen() {
	HAL_DAC_Stop(&hdac, DAC_CHANNEL_1);
	switch(genShape) {
		case '-'://const voltage
			genFillConst(genAmpl, genDmaBuf, GEN_DMA_LENGTH);
			break;
		case 'M': //Meander
			genFillConst(0, genDmaBuf, GEN_DMA_LENGTH / 2);
			genFillConst(genAmpl, &genDmaBuf[GEN_DMA_LENGTH / 2], GEN_DMA_LENGTH / 2);
			break;
		case 'T': //Triangle
			genFillTriangle();
			break;
		case 'J'://Sawtooth
			genFillSawtooth();
			break;
		case 'S':
			genFillSine();
			break;
		default: return;//case 'N'
	}
	htim2.Instance->ARR = 100000 / genFreq;
	htim2.Instance->CNT = 0;
	if(genBuff == 't' || genBuff == 'T')
	{
		hdac.Instance->CR &= ~DAC_CR_BOFF1;
	} else {
		hdac.Instance->CR |= DAC_CR_BOFF1;
	}
	HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)genDmaBuf, GEN_DMA_LENGTH, DAC_ALIGN_12B_R);
}

void setGenShape(char shape) {
	genShape = shape;
	setupGen();
}

void setGenBuff(char buff) {
	genBuff = buff;
	setupGen();
}

void setGenFreq(char *value, size_t length) {
	genFreq = parseDecimal(value,length);
	setupGen();
}
void setGenAmpl(char *value, size_t length) {
	genAmpl = parseDecimal(value,length);
	setupGen();
}

void sendBuffer() {
	if(!(bufferD[0] & FLAG_NEW)) {
		//TODO copy non-key buffer
	}
	if(bufferD[0] & (FLAG_CLEAR|FLAG_NO_DATA)) {
			OSCILL_Transmit_FS((uint8_t*)bufferD, 2); 
	} else {
			OSCILL_Transmit_FS((uint8_t*)bufferD, 2 * (1 + 3 * FRAME_SIZE)); 
	}
	//TODO busy state
	bufferD[0] |= FLAG_NO_DATA;
	setupTrigger();
}
