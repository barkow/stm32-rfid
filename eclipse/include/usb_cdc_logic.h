#ifndef _USB_CDC_LOGIC_H_
#define _USB_CDC_LOGIC_H_
#include "usbd_cdc.h"
#include "MFRC522Desfire.h"

void usbCdcLoop(MFRC522Desfire *mfrc522, USBD_HandleTypeDef *pdev);
#endif
