/**
  ******************************************************************************
  * @file           : usbd_oscill_if.c
  * @brief          :
  ******************************************************************************
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  * 1. Redistributions of source code must retain the above copyright notice,
  * this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  * this list of conditions and the following disclaimer in the documentation
  * and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of its contributors
  * may be used to endorse or promote products derived from this software
  * without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "usbd_oscill_if.h"
#include "oscill.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_OSCILL 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_OSCILL_Private_TypesDefinitions
  * @{
  */ 
  /* USER CODE BEGIN 0 */ 
  /* USER CODE END 0 */ 
/**
  * @}
  */ 

/** @defgroup USBD_OSCILL_Private_Defines
  * @{
  */ 
  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */  
/**
  * @}
  */ 

/** @defgroup USBD_OSCILL_Private_Macros
  * @{
  */ 
  /* USER CODE BEGIN 2 */ 

  /* USER CODE END 2 */
/**
  * @}
  */ 
  
/** @defgroup USBD_OSCILL_Private_Variables
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/* Received Data over USB are stored in this buffer       */
uint8_t UserRxBufferFS[USB_FS_MAX_PACKET_SIZE];

/* Send Data over USB OSCILL are stored in this buffer       */
uint8_t UserTxBufferFS[USB_FS_MAX_PACKET_SIZE];

/* USB handler declaration */
/* Handle for USB Full Speed IP */
USBD_HandleTypeDef  *hUsbDevice_0;

extern USBD_HandleTypeDef hUsbDeviceFS;

/**
  * @}
  */ 
  
/** @defgroup USBD_OSCILL_Private_FunctionPrototypes
  * @{
  */
static int8_t OSCILL_Init_FS     (void);
static int8_t OSCILL_DeInit_FS   (void);
static int8_t OSCILL_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t OSCILL_Receive_FS  (uint8_t* pbuf, uint32_t *Len);

USBD_OSCILL_ItfTypeDef USBD_Interface_fops_FS = 
{
  OSCILL_Init_FS,
  OSCILL_DeInit_FS,
  OSCILL_Control_FS,  
  OSCILL_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  OSCILL_Init_FS
  *         Initializes the OSCILL media low layer over the FS USB IP
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t OSCILL_Init_FS(void)
{
  hUsbDevice_0 = &hUsbDeviceFS;
  /* USER CODE BEGIN 3 */ 
  /* Set Application Buffers */
  USBD_OSCILL_SetTxBuffer(hUsbDevice_0, UserTxBufferFS, 0);
  USBD_OSCILL_SetRxBuffer(hUsbDevice_0, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */ 
}

/**
  * @brief  CDC_DeInit_FS
  *         DeInitializes the OSCILL media low layer
  * @param  None
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t OSCILL_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */ 
  return (USBD_OK);
  /* USER CODE END 4 */ 
}

/**
  * @brief  OSCILL_Control_FS
  *         Manage the CDC class requests
  * @param  cmd: Command code            
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t OSCILL_Control_FS  (uint8_t cmd, uint8_t* pbuf, uint16_t length)
{ 
  /* USER CODE BEGIN 5 */
  switch (cmd)
  {
    
  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

	
/**
  * @brief  OSCILL_Receive_FS
  *         Data received over USB OUT endpoint are sent over CDC interface 
  *         through this function.
  *           
  *         @note
  *         This function will block any OUT packet reception on USB endpoint 
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on OSCILL interface (ie. using DMA controller) it will result 
  *         in receiving more data while previous ones are still not sent.
  *                 
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t OSCILL_Receive_FS (uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
	if(*Len == 5 && memcmp(Buf,"FRAME",5) == 0)
	{
		sendBuffer();
	} else if(*Len ==13 && Buf[0]=='s' && Buf[1] =='.' &&
		memcmp (&Buf[3],".range=",7) == 0)
	{ //s.[channel].range=[0..3/0..3]
		char channel = Buf[2];
		char gain = Buf[10];
		char div = Buf[12];
		setGain(channel,gain);
		setDiv(channel, div);

	} else if(*Len ==3 && memcmp (Buf,"t=",2) == 0) {
		setTiming(Buf[2]);

	} else if(*Len == 11 && memcmp (Buf,"trig.type=",10) == 0) {
		setTriggerType(Buf[10]);
	} else if(*Len == 9 && memcmp (Buf,"trig.ch=",8) == 0) {
		setTriggerChannel(Buf[8]);
	} else if(*Len > 11 && memcmp (Buf,"trig.level=",11) == 0) {
		setTriggerLevel((char*)&Buf[11],*Len - 11 );
	} else if(*Len > 10 && memcmp (Buf,"trig.time=",10) == 0) {
		setTriggerTimeShift((char*)&Buf[10],*Len - 10 );

	} else if(*Len == 11 && memcmp (Buf,"gen.shape=",10) == 0) {
		setGenShape(Buf[10]);
	} else if(*Len == 10 && memcmp (Buf,"gen.buff=",9) == 0) {
		setGenBuff(Buf[9]);
	} else if(*Len > 9 && memcmp (Buf,"gen.ampl=",9) == 0) {
		setGenAmpl((char*)&Buf[9],*Len - 9 );
	} else if(*Len > 9 && memcmp (Buf,"gen.freq=",9) == 0) {
		setGenFreq((char*)&Buf[9],*Len - 9 );

	} else if(*Len == 6 && memcmp(Buf,"CONFIG",6) == 0) {
		OSCILL_Transmit_FS(&OscillConfigDataShielded[4], * (uint32_t *)OscillConfigDataShielded);
	}
  return (USBD_OK);
  /* USER CODE END 6 */ 
}

/** 
  * @brief  OSCILL_Transmit_FS
  *         Data send over USB IN endpoint are sent over CDC interface 
  *         through this function.           
  *         @note
  *         
  *                 
  * @param  Buf: Buffer of data to be send
  * @param  Len: Number of data to be send (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t OSCILL_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */ 
  USBD_OSCILL_SetTxBuffer(hUsbDevice_0, Buf, Len);   
  result = USBD_OSCILL_TransmitPacket(hUsbDevice_0);
  /* USER CODE END 7 */ 
  return result;
}

/**
  * @brief Copy a buffer from user memory area to packet memory area (PMA)
  * @param   USBx: USB peripheral instance register address.
  * @param   pbUsrBuf: pointer to user memory area.
  * @param   wPMABufAddr: address into PMA.
  * @param   wNBytes: no. of bytes to be copied.
  * @retval None
	*
	* @!!! NOTE This is optimized version instead of standard one in Drivers/STM32F3xx_HAL_Driver/Src/stm32f3xx_hal_pcd_ex.c
  * @!!! NOTE Standard version should be removed after every CubeMX regeneration
	*
  */
void PCD_WritePMA(USB_TypeDef  *USBx, uint8_t *pbUsrBuf, uint16_t wPMABufAddr, uint16_t wNBytes)
{
  int n = (wNBytes + 1) >> 1;   // n = (wNBytes + 1) / 2 
  __IO uint32_t *pdwVal = (uint32_t *)(wPMABufAddr * 2 + (uint32_t)USBx + 0x400);
	uint16_t * pwUsrBuf = (uint16_t *) pbUsrBuf;
  for (; n != 0; n--)
  {
		*pdwVal = * pwUsrBuf;
		pwUsrBuf++;
		pdwVal++;
  }
}
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

