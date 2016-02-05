#ifdef USE_USB_HID

#include "usb_hid_logic.h"

void usbKeyboardSendHex(USBD_HandleTypeDef *pdev, uint8_t *dat, uint16_t len){
	for (int i = 0; i < len; i++){
		uint8_t high = dat[i] / 16;
		uint8_t low = dat[i] & 0x0f;
		uint8_t hexstr[2];
		hexstr[0] = high <= 9 ? high + '0' : high - 10 + 'a';
		hexstr[1] = low <= 9 ? low + '0' : low - 10 + 'a';
		usbKeyboardSendString(pdev, hexstr, 2);
	}
}

void usbKeyboardSendString(USBD_HandleTypeDef *pdev, uint8_t *dat, uint16_t len){
	uint8_t usbReportBuf[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	for(uint8_t i = 0; i < len; i++){
		uint8_t character = dat[i];
		if (!((character >= 'A') && (character <= 'Z')) && !((character >= 'a') && (character <= 'z')) && !((character >= '0') && (character <= '9')) && !(character == ':') && !(character == '!')){
			continue;
		}
		//Prüfen, ob Shift Taste aktiviert werden muss
		if ((character >= 'A')&&(character <= 'Z')){
			usbReportBuf[0] = 0x02;
			//In Kleinbuchstaben wandeln
			character = character + ('a' - 'A');
		}
		if ((character >= 'a')&&(character <= 'z')){
			usbReportBuf[2] = 0x04 + character - 'a';
		}
		if ((character >= '0')&&(character <= '9')){
			usbReportBuf[0] = 0x00;
			usbReportBuf[2] = character == '0' ? 0x27 : 0x1e + character - '1';
		}
		if ((character == '!')){
			usbReportBuf[0] = 0x02;
			usbReportBuf[2] = 0x1e;
		}
		if ((character == ':')){
			usbReportBuf[0] = 0x02;
			usbReportBuf[2] = 0x37;
		}
		USBD_HID_SendReport(pdev, usbReportBuf, 8);
		HAL_Delay(USBHIDKEYDELAY);
		usbReportBuf[0] = 0;
		usbReportBuf[2] = 0;
		USBD_HID_SendReport(pdev, usbReportBuf, 8);
		HAL_Delay(USBHIDKEYDELAY);
	}
}
#endif
