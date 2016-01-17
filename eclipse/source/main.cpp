/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
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
#include "stm32f1xx_hal.h"
#include "crc.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
//Spi.h wird nicht benötigt
#include "spi.h"

/* USER CODE BEGIN Includes */
#include "MFRC522Desfire.h"
#include <stdexcept>
#include "secrets.h"
#include "usbd_hid.h"
#include "display.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
#define USBHIDKEYDELAY 40
/* Private variables ---------------------------------------------------------*/
SPIClass SPI;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void usbKeyboardSendString(USBD_HandleTypeDef *pdev, uint8_t *dat, uint16_t len);
void usbKeyboardSendHex(USBD_HandleTypeDef *pdev, uint8_t *dat, uint16_t len);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CRC_Init();
  MX_SPI1_Init();
  //SPI1 wird in der Klasse MFRC522Desfire initialisiert
  //MX_SPI2_Init();
  //SPI2 wird in der Klasse display initialisiert
  MX_USART1_UART_Init();
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN 2 */
  MFRC522Desfire mfrc522;
  mfrc522.PCD_Init();
  volatile uint8_t t = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);

  mfrc522.PCD_WriteRegister(mfrc522.GsNReg, 0xff);
  mfrc522.PCD_WriteRegister(mfrc522.CWGsPReg, 0x3f);
  //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  mfrc522.PCD_SetAntennaGain(0xff);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //Key für OpendoScala vorberechnen, da nicht mit kartenabhängigem Salt versehen
  MFRC522Desfire::DesfireAesKey opendoScalaKey = mfrc522.DeriveKeyFromPassword(IKAFKAOPENDOSCALAPASSWORD, "");
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
	  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	  //Auf Erkennugn von RFID Karte warten
	  while (!mfrc522.PICC_IsNewCardPresent()){}
	  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	  //Melden, dass neue RFID Karte erkannt wurde
	  usbKeyboardSendString(&hUsbDeviceFS, (uint8_t*)"!:new::!", 8);
	  //UID auslesen und ausgeben
	  MFRC522::Uid uid;
	  if (mfrc522.PICC_Select(&uid) != mfrc522.STATUS_OK){
		  continue;
	  }
	  usbKeyboardSendString(&hUsbDeviceFS, (uint8_t*)"!:uid:", 6);
	  usbKeyboardSendHex(&hUsbDeviceFS, uid.uidByte, 7);
	  usbKeyboardSendString(&hUsbDeviceFS, (uint8_t*)":!", 2);

	  //Applikation OpendoScala auslesen
	  if (mfrc522.Desfire_SelectApplication(IKAFKAOPENDOSCALAAPPID) != mfrc522.STATUS_OK){
	  	continue;
	  }
	  if (mfrc522.Desfire_Authenticate(IKAFKAOPENDOSCALAKEYNO, opendoScalaKey) != mfrc522.STATUS_OK){
		  continue;
	  }
	  byte data[4];
	  byte dataLen = 4;
	  if (mfrc522.Desfire_ReadData(IKAFKAOPENDOSCALAFILENO, 0, 4, data, &dataLen) != mfrc522.STATUS_OK){
		  continue;
	  }
	  usbKeyboardSendString(&hUsbDeviceFS, (uint8_t*)"!:opnId:", 8);
	  usbKeyboardSendHex(&hUsbDeviceFS, data, 4);
	  usbKeyboardSendString(&hUsbDeviceFS, (uint8_t*)":!", 2);
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBPLLCLK_DIV1_5;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */
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
/* USER CODE END 4 */

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */

/**
  * @}
*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
