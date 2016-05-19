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
//#include "usb_device.h"
#include "tim.h"
#include "gpio.h"
//Spi.h wird nicht benötigt
#include "spi.h"

/* USER CODE BEGIN Includes */
#include "MFRC522Desfire.h"
#include <stdexcept>
#include "secrets.h"
#include "MFRC522Desfire.h"
#include "ikotron.h"

/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
SPIClass SPI;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void send_command(int command, void *message)
{
   asm("mov r0, %[cmd];"
       "mov r1, %[msg];"
       "bkpt #0xAB"
         :
         : [cmd] "r" (command), [msg] "r" (message)
         : "r0", "r1", "memory");
}

void send_message(const char s[], uint8_t len){
	uint32_t m[] = { 2/*stderr*/, (uint32_t)s, len };
	send_command(0x05/* some interrupt ID */, m);
}
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
  MX_USART1_UART_Init();
  //MX_USB_DEVICE_Init();
  MX_TIM2_Init();
  ikotron::init(&htim2);

  send_message("Ikotron\n", 8);

  /* USER CODE BEGIN 2 */
  MFRC522Desfire mfrc522;
  mfrc522.PCD_Init();
  volatile uint8_t t = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);

  mfrc522.PCD_WriteRegister(mfrc522.GsNReg, 0xff);
  mfrc522.PCD_WriteRegister(mfrc522.CWGsPReg, 0x3f);
  //mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);
  mfrc522.PCD_SetAntennaGain(0xff);

  /* Infinite loop */
  //Key für OpendoScala vorberechnen, da nicht mit kartenabhängigem Salt versehen
  MFRC522Desfire::DesfireAesKey opendoScalaKey = mfrc522.DeriveKeyFromPassword(IKAFKAOPENDOSCALAPASSWORD, "");
  while (1) {
	  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);
	  //Auf Erkennung von RFID Karte warten
	  send_message("Wait for RFID\n", 14);
	  while (!mfrc522.PICC_IsNewCardPresent()){}
	  send_message("RFID detected\n", 14);
	  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_SET);
	  //UID auslesen und ausgeben
	  MFRC522::Uid uid;

	  if (mfrc522.PICC_Select(&uid) != mfrc522.STATUS_OK){
		  send_message("Retry\n", 6);
		  if (mfrc522.PICC_Select(&uid) != mfrc522.STATUS_OK){
			  send_message("Retry\n", 6);
			  if (mfrc522.PICC_Select(&uid) != mfrc522.STATUS_OK){
				  send_message("Err 00\n", 7);
				  mfrc522.PICC_HaltA();
				  continue;
			  }
		  }

	  }

	  //Applikation OpendoScala auslesen
	  if (mfrc522.Desfire_SelectApplication(IKAFKAOPENDOSCALAAPPID) != mfrc522.STATUS_OK){
		  send_message("Retry\n", 6);
		  if (mfrc522.Desfire_SelectApplication(IKAFKAOPENDOSCALAAPPID) != mfrc522.STATUS_OK){
			  send_message("Retry\n", 6);
			  if (mfrc522.Desfire_SelectApplication(IKAFKAOPENDOSCALAAPPID) != mfrc522.STATUS_OK){
				  send_message("Err 01\n", 7);
				  mfrc522.PICC_HaltA();
				  continue;
			  }
		  }

	  }

	  if (mfrc522.Desfire_Authenticate(IKAFKAOPENDOSCALAKEYNO, opendoScalaKey) != mfrc522.STATUS_OK){
		  send_message("Retry\n", 6);
		  if (mfrc522.Desfire_Authenticate(IKAFKAOPENDOSCALAKEYNO, opendoScalaKey) != mfrc522.STATUS_OK){
			  send_message("Retry\n", 6);
			  if (mfrc522.Desfire_Authenticate(IKAFKAOPENDOSCALAKEYNO, opendoScalaKey) != mfrc522.STATUS_OK){
				  send_message("Err 02\n", 7);
				  mfrc522.PICC_HaltA();
				  continue;
			  }
		  }

	  }
	  byte data[32];
	  byte dataLen = 4;
	  if (mfrc522.Desfire_ReadData(IKAFKAOPENDOSCALAFILENO, 0, 4, data, &dataLen) != mfrc522.STATUS_OK){
		  send_message("Err 03\n", 7);
		  mfrc522.PICC_HaltA();
		  continue;
	  }
	  //In data steht die Id
	  ikotron::sendFrame(2050);
	  send_message("TransmitId\n", 11);
	  mfrc522.PICC_HaltA();
  }
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
