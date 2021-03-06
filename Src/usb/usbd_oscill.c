/**
  ******************************************************************************
  * @file    usbd_cdc.c
  * @author  MCD Application Team
  * @version V2.2.0
  * @date    13-June-2014
  * @brief   This file provides the high layer firmware functions to manage the 
  *          following functionalities of the USB OSCILL Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as OSCILL Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *           
  *  @verbatim
  *      
  *          ===================================================================      
  *                                OSCILL Class Driver Description
  *          =================================================================== 
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus 
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as OSCILL device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  * 
  *           These aspects may be enriched or modified for a specific user application.
  *          
  *            This driver doesn't implement the following aspects of the specification 
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
  *      
  *  @endverbatim
  *                                  
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "usbd_oscill.h"
#include "usbd_oscill_if.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_OSCILL 
  * @brief usbd core module
  * @{
  */ 

/** @defgroup USBD_OSCILL_Private_TypesDefinitions
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_OSCILL_Private_Defines
  * @{
  */ 
/**
  * @}
  */ 


/** @defgroup USBD_OSCILL_Private_Macros
  * @{
  */ 

/**
  * @}
  */ 


/** @defgroup USBD_OSCILL_Private_FunctionPrototypes
  * @{
  */


static uint8_t  USBD_OSCILL_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_OSCILL_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_OSCILL_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  USBD_OSCILL_DataIn (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);

static uint8_t  USBD_OSCILL_DataOut (USBD_HandleTypeDef *pdev, 
                                 uint8_t epnum);

static uint8_t  USBD_OSCILL_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  *USBD_OSCILL_GetFSCfgDesc (uint16_t *length);

/**
  * @}
  */ 

/** @defgroup USBD_OSCILL_Private_Variables
  * @{
  */ 


/* OSCILL interface class callbacks structure */
USBD_ClassTypeDef  USBD_OSCILL = 
{
  USBD_OSCILL_Init,
  USBD_OSCILL_DeInit,
  USBD_OSCILL_Setup,
  NULL,                 /* EP0_TxSent, */
  USBD_OSCILL_EP0_RxReady,
  USBD_OSCILL_DataIn,
  USBD_OSCILL_DataOut,
  NULL,
  NULL,
  NULL,     
  NULL,  
  USBD_OSCILL_GetFSCfgDesc,    
  NULL, 
  NULL,
};


/* USB OSCILL device Configuration Descriptor */
__ALIGN_BEGIN uint8_t USBD_OSCILL_CfgFSDesc[USB_OSCILL_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /*Configuration Descriptor*/
  0x09,   /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,      /* bDescriptorType: Configuration */
  USB_OSCILL_CONFIG_DESC_SIZ,                /* wTotalLength:no of returned bytes */
  0x00,
  0x01,   /* bNumInterfaces: 1 interface */
  0x01,   /* bConfigurationValue: Configuration value */
  0x00,   /* iConfiguration: Index of string descriptor describing the configuration */
  0xC0,   /* bmAttributes: self powered */
  0x32,   /* MaxPower 0 mA */
  
  /*---------------------------------------------------------------------------*/
  
  /*Interface Descriptor */
  0x09,   /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,  /* bDescriptorType: Interface */
  /* Interface descriptor type */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints: Two endpoints used */
  0xFF,   /* bInterfaceClass: Communication Interface Class */
  0x00,   /* bInterfaceSubClass: Abstract Control Model */
  0x00,   /* bInterfaceProtocol: Common AT commands */
  0x00,   /* iInterface: */
  
  /*Endpoint CMD Descriptor*/
  0x07,                           /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,         /* bDescriptorType: Endpoint */
  OSCILL_IN_EP,                  /* bEndpointAddress */
  USBD_EP_TYPE_BULK,              /* bmAttributes */
  LOBYTE(USB_FS_MAX_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(USB_FS_MAX_PACKET_SIZE),
  0x00,                            /* bInterval: ignored */
  /*---------------------------------------------------------------------------*/
  
  /*Endpoint IN CONF Descriptor*/
  0x07,   											  /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,         /* bDescriptorType: Endpoint */
  OSCILL_OUT_EP,              /* bEndpointAddress */
  USBD_EP_TYPE_BULK,              /* bmAttributes */
  LOBYTE(USB_FS_MAX_PACKET_SIZE), /* wMaxPacketSize: */
  HIBYTE(USB_FS_MAX_PACKET_SIZE),
  0x00                           /* bInterval: ignored */
} ;

/**
  * @}
  */ 

/** @defgroup USBD_OSCILL_Private_Functions
  * @{
  */ 
/**
  * @brief  USBD_OSCILL_Init
  *         Initilaize the OSCILL interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_OSCILL_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{
	pdev->pClassData = USBD_malloc(sizeof (USBD_OSCILL_HandleTypeDef));
  

	if(pdev->pClassData == NULL)
  {
    return 1; 
  }
  else
  {
    
    /* Init  physical Interface components */
    ((USBD_OSCILL_ItfTypeDef *)pdev->pUserData)->Init();
    
    /* Init Xfer states */
       
  }
  /* Open OUT EP */
  USBD_LL_OpenEP(pdev,
                 OSCILL_OUT_EP,
                 USBD_EP_TYPE_BULK,
                 USB_FS_MAX_PACKET_SIZE);
  USBD_OSCILL_HandleTypeDef   *hoscill = pdev->pClassData;
  USBD_LL_PrepareReceive(pdev, OSCILL_OUT_EP, hoscill->RxBuffer, USB_FS_MAX_PACKET_SIZE);  
  
	/* Open EP IN */
	USBD_LL_OpenEP(pdev,
								 OSCILL_IN_EP,
								 USBD_EP_TYPE_BULK,
								 USB_FS_MAX_PACKET_SIZE);
	
  return 0;
}

/**
  * @brief  USBD_OSCILL_Init
  *         DeInitialize the OSCILL layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_OSCILL_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{
  uint8_t ret = 0;
  
  /* Close EP IN */
  USBD_LL_CloseEP(pdev,
              OSCILL_IN_EP);
  
  /* Close EP OUT */
  USBD_LL_CloseEP(pdev,
              OSCILL_OUT_EP);
  
  
  /* DeInit  physical Interface components */
  if(pdev->pClassData != NULL)
  {
    ((USBD_OSCILL_ItfTypeDef *)pdev->pUserData)->DeInit();
    USBD_free(pdev->pClassData);
    pdev->pClassData = NULL;
  }
  
  return ret;
}

/**
  * @brief  USBD_OSCILL_Setup
  *         Handle the OSCILL specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t  USBD_OSCILL_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req)
{
  USBD_OSCILL_HandleTypeDef   *hoscill = pdev->pClassData;
  
  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
    if (req->wLength)
    {
      if (req->bmRequest & 0x80)
      {
        ((USBD_OSCILL_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                          (uint8_t *)hoscill->data,
                                                          req->wLength);
          USBD_CtlSendData (pdev, 
                            (uint8_t *)hoscill->data,
                            req->wLength);
      }
      else
      {
        hoscill->CmdOpCode = req->bRequest;
        hoscill->CmdLength = req->wLength;
        
        USBD_CtlPrepareRx (pdev, 
                           (uint8_t *)hoscill->data,
                           req->wLength);
      }
      
    }
    else
    {
        ((USBD_OSCILL_ItfTypeDef *)pdev->pUserData)->Control(req->bRequest,
                                                          NULL,
                                                          0);
    }
    break;
 
  default: 
    break;
  }
  return USBD_OK;
}

/**
  * @brief  usbd_oscill_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_OSCILL_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
//  USBD_OSCILL_HandleTypeDef   *hoscill = pdev->pClassData;
  
  if(pdev->pClassData != NULL)
  {
    
		USBD_LL_Transmit(pdev, epnum, NULL, 0);
    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}

/**
  * @brief  USBD_OSCILL_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t  USBD_OSCILL_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum)
{      
  USBD_OSCILL_HandleTypeDef   *hoscill = pdev->pClassData;
  
  /* Get the received data length */
  hoscill->RxLength = USBD_LL_GetRxDataSize (pdev, epnum);
  
  /* USB data will be immediately processed, this allow next USB traffic being 
  NAKed till the end of the application Xfer */
  if(pdev->pClassData != NULL)
  {
    ((USBD_OSCILL_ItfTypeDef *)pdev->pUserData)->Receive(hoscill->RxBuffer, &hoscill->RxLength);
		USBD_LL_PrepareReceive(pdev, OSCILL_OUT_EP, hoscill->RxBuffer, USB_FS_MAX_PACKET_SIZE);  
    return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
  }
}



/**
  * @brief  USBD_OSCILL_EP0_RxReady
  *         Data received on control Out endpoint
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_OSCILL_EP0_RxReady (USBD_HandleTypeDef *pdev)
{ 
  USBD_OSCILL_HandleTypeDef   *hoscill = pdev->pClassData;
  
  if((pdev->pUserData != NULL) && (hoscill->CmdOpCode != 0xFF))
  {
    ((USBD_OSCILL_ItfTypeDef *)pdev->pUserData)->Control(hoscill->CmdOpCode,
                                                      (uint8_t *)hoscill->data,
                                                      hoscill->CmdLength);
      hoscill->CmdOpCode = 0xFF; 
      
  }
  return USBD_OK;
}

/**
  * @brief  USBD_OSCILL_GetFSCfgDesc 
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_OSCILL_GetFSCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_OSCILL_CfgFSDesc);
  return USBD_OSCILL_CfgFSDesc;
}


/**
* @brief  USBD_OSCILL_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t  USBD_OSCILL_RegisterInterface  (USBD_HandleTypeDef   *pdev, 
                                      USBD_OSCILL_ItfTypeDef *fops)
{
  uint8_t  ret = USBD_FAIL;
  
  if(fops != NULL)
  {
    pdev->pUserData= fops;
    ret = USBD_OK;    
  }
  
  return ret;
}

/**
  * @brief  USBD_OSCILL_SetTxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Tx Buffer
  * @retval status
  */
uint8_t  USBD_OSCILL_SetTxBuffer  (USBD_HandleTypeDef   *pdev,
                                uint8_t  *pbuff,
                                uint16_t length)
{
  USBD_OSCILL_HandleTypeDef   *hoscill = pdev->pClassData;
  
  hoscill->TxBuffer = pbuff;
  hoscill->TxLength = length;  
  
  return USBD_OK;  
}


/**
  * @brief  USBD_OSCILL_SetRxBuffer
  * @param  pdev: device instance
  * @param  pbuff: Rx Buffer
  * @retval status
  */
uint8_t  USBD_OSCILL_SetRxBuffer  (USBD_HandleTypeDef   *pdev,
                                   uint8_t  *pbuff)
{
  USBD_OSCILL_HandleTypeDef   *hoscill = pdev->pClassData;
  
  hoscill->RxBuffer = pbuff;
  
  return USBD_OK;
}

/**
  * @brief  USBD_OSCILL_TransmitPacket
  *         Data sent through CONF_EP
  * @param  pdev: device instance
  * @retval status
  */
uint8_t  USBD_OSCILL_TransmitPacket(USBD_HandleTypeDef *pdev)
{      
  USBD_OSCILL_HandleTypeDef   *hoscill = pdev->pClassData;
  
  if(pdev->pClassData != NULL)
  {
      
      /* Transmit next packet */
      USBD_LL_Transmit(pdev,
                       OSCILL_IN_EP,
                       hoscill->TxBuffer,
                       hoscill->TxLength);
      
      /* Tx Transfer in progress */
			return USBD_OK;
  }
  else
  {
    return USBD_FAIL;
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
