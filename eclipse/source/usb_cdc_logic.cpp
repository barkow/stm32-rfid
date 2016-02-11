#ifdef USE_USB_CDC

#include "usb_cdc_logic.h"
#include "secrets.h"

void usbCdcLoop(MFRC522Desfire *mfrc522, USBD_HandleTypeDef *pdev){
	//Key für OpendoScala vorberechnen, da nicht mit kartenabhängigem Salt versehen
	MFRC522Desfire::DesfireAesKey opendoScalaKey = mfrc522->DeriveKeyFromPassword(IKAFKAOPENDOSCALAPASSWORD, "");
	while (1) {
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
		//Auf Erkennugn von RFID Karte warten
		while (!mfrc522->PICC_IsNewCardPresent()){}
		HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
		//Melden, dass neue RFID Karte erkannt wurde
		//usbKeyboardSendString(pdev, (uint8_t*)"!:new::!", 8);
		//UID auslesen und ausgeben
		MFRC522::Uid uid;
		if (mfrc522->PICC_Select(&uid) != mfrc522->STATUS_OK){
			continue;
		}
		//usbKeyboardSendString(pdev, (uint8_t*)"!:uid:", 6);
		//usbKeyboardSendHex(pdev, uid.uidByte, 7);
		//usbKeyboardSendString(pdev, (uint8_t*)":!", 2);

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
		//usbKeyboardSendString(pdev, (uint8_t*)"!:opnId:", 8);
		//usbKeyboardSendHex(pdev, data, 4);
		//usbKeyboardSendString(pdev, (uint8_t*)":!", 2);

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
		//usbKeyboardSendString(pdev, (uint8_t*)"!:ifiCard:", 10);
		//usbKeyboardSendHex(pdev, data, 4);
		//usbKeyboardSendString(pdev, (uint8_t*)":!", 2);
		if (mfrc522->Desfire_Authenticate(IKAFKAIDENTSTAFFIDKEYNO, IKAFKAIDENTSTAFFIDPASSWORD, uid.uidByte, uid.size, "") != mfrc522->STATUS_OK){
			continue;
		}
		dataLen = 4;
		if (mfrc522->Desfire_ReadData(IKAFKAIDENTSTAFFIDFILENO, 0, 4, data, &dataLen) != mfrc522->STATUS_OK){
			continue;
		}
		//usbKeyboardSendString(pdev, (uint8_t*)"!:ifiStff:", 10);
		//usbKeyboardSendHex(pdev, data, 4);
		//usbKeyboardSendString(pdev, (uint8_t*)":!", 2);
	}
}

#endif
