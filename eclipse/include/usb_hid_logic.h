#ifndef _USB_HID_LOGIC_H_
#define _USB_HID_LOGIC_H_
#include "usbd_hid.h"

#define USBHIDKEYDELAY 40

void usbKeyboardSendHex(USBD_HandleTypeDef *pdev, uint8_t *dat, uint16_t len);
void usbKeyboardSendString(USBD_HandleTypeDef *pdev, uint8_t *dat, uint16_t len);
#endif
