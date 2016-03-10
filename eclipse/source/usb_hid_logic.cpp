#ifdef USE_USB_HID

#include "usb_hid_logic.h"
#include "secrets.h"

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

//Ausgabe eines nullterminierten Strings
void usbKeyboardSendString(USBD_HandleTypeDef *pdev, uint8_t *dat){
	uint16_t len = 0;
	//Implizit wird angenommen, dass der String nicht länger als 100 Zeichen ist
	while((dat[len] != 0) && (len < 100)){
		len++;
	}
	usbKeyboardSendString(pdev, dat, len);
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

void usbHidLoop(MFRC522Desfire *mfrc522, USBD_HandleTypeDef *pdev){
	//Key für OpendoScala vorberechnen, da nicht mit kartenabhängigem Salt versehen
	MFRC522Desfire::DesfireAesKey opendoScalaKey = mfrc522->DeriveKeyFromPassword(IKAFKAOPENDOSCALAPASSWORD, "");
	while (1) {
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
		//Auf Erkennugn von RFID Karte warten
		while (!mfrc522->PICC_IsNewCardPresent()){}
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
		//Melden, dass neue RFID Karte erkannt wurde
		usbKeyboardSendString(pdev, (uint8_t*)"!:new::!", 8);
		//UID auslesen und ausgeben
		MFRC522::Uid uid;
		if (mfrc522->PICC_Select(&uid) != mfrc522->STATUS_OK){
			continue;
		}
		usbKeyboardSendString(pdev, (uint8_t*)"!:uid:", 6);
		usbKeyboardSendHex(pdev, uid.uidByte, 7);
		usbKeyboardSendString(pdev, (uint8_t*)":!", 2);

		//Applikation OpendoScala auslesen
		if (mfrc522->Desfire_SelectApplication(IKAFKAOPENDOSCALAAPPID) != mfrc522->STATUS_OK){
			continue;
		}
		if (mfrc522->Desfire_Authenticate(IKAFKAOPENDOSCALAKEYNO, opendoScalaKey) != mfrc522->STATUS_OK){
			continue;
		}
		byte data[32];
		byte dataLen = 4;
		if (mfrc522->Desfire_ReadData(IKAFKAOPENDOSCALAFILENO, 0, 4, data, &dataLen) != mfrc522->STATUS_OK){
			continue;
		}
		usbKeyboardSendString(pdev, (uint8_t*)"!:opnId:", 8);
		usbKeyboardSendHex(pdev, data, 4);
		usbKeyboardSendString(pdev, (uint8_t*)":!", 2);

		//Applikation IkaFkaIdent auslesen
		if (mfrc522->Desfire_SelectApplication(IKAFKAIDENTAPPID) != mfrc522->STATUS_OK){
			continue;
		}
		if (mfrc522->Desfire_Authenticate(IKAFKAIDENTCARDIDKEYNO, IKAFKAIDENTCARDIDPASSWORD, uid.uidByte, uid.size, "") != mfrc522->STATUS_OK){
			continue;
		}
		dataLen = 4;
		if (mfrc522->Desfire_ReadData(IKAFKAIDENTCARDIDFILENO, 0, 4, data, &dataLen) != mfrc522->STATUS_OK){
			continue;
		}
		usbKeyboardSendString(pdev, (uint8_t*)"!:ifiCard:", 10);
		usbKeyboardSendHex(pdev, data, 4);
		usbKeyboardSendString(pdev, (uint8_t*)":!", 2);
		if (mfrc522->Desfire_Authenticate(IKAFKAIDENTSTAFFIDKEYNO, IKAFKAIDENTSTAFFIDPASSWORD, uid.uidByte, uid.size, "") != mfrc522->STATUS_OK){
			continue;
		}
		dataLen = 32;
		if (mfrc522->Desfire_ReadData(IKAFKAIDENTSTAFFIDFILENO, 0, 32, data, &dataLen) != mfrc522->STATUS_OK){
			continue;
		}
		usbKeyboardSendString(pdev, (uint8_t*)"!:ifiStff:", 10);
		usbKeyboardSendString(pdev, data);
		usbKeyboardSendString(pdev, (uint8_t*)":!", 2);
	}
}

#endif
