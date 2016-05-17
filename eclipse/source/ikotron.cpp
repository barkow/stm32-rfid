/*
 * ikotron.cpp
 *
 *  Created on: 16.05.2016
 *      Author: barkow
 */
#include "ikotron.h"

uint32_t ikotron::frame[2];
uint8_t ikotron::bitCnt;
uint8_t ikotron::phaseCnt;
TIM_HandleTypeDef* ikotron::htim = NULL;

void ikotron::sendFrame(uint16_t id) {
	/*Frame Format: D950083 0802 01 E2 D
	D950083:   Konstante
	0802:      Id
	01:        Konstante
	E2:        Checksumme
	D:         Konstante
	 */

	//Frame setzt sich aus zwei 32bit Werten zusammen
	ikotron::frame[1] = 0xD9500830;
	ikotron::frame[0] = 0x0000100D;

	//Id in Frame schreiben
	ikotron::frame[1] |= (((uint32_t)id) >> 12) & 0x000000ff;
	ikotron::frame[0]  |= (((uint32_t)id) << 20) & 0xfff00000;

	//Checksumme berechnen und in Frame schreiben
	uint32_t checksum = 0x95 ^ 0x00 ^ 0x83 ^ ((id >> 8) & 0xff) ^ (id & 0xff) ^ 0x01 ^ 0xff;
	ikotron::frame[0] |= (checksum << 4) & 0x00000ff0;

	//Timerbasiertes Versenden vorbereiten
	ikotron::phaseCnt = 3;
	ikotron::bitCnt = 64;
	//TODO: Timer aktivieren
	HAL_TIM_Base_Start_IT(ikotron::htim);
}

void ikotron::sendLoop() {
	if (ikotron::bitCnt == 0){
		HAL_GPIO_WritePin(IKOTRON_DATA_GPIO_Port, IKOTRON_DATA_Pin, GPIO_PIN_RESET);
		//TODO: Timer deaktivieren
		HAL_TIM_Base_Stop_IT(ikotron::htim);
		return;
	}

	switch (ikotron::phaseCnt) {
		case 3:
			//DATA ausgeben
			if((ikotron::frame[(ikotron::bitCnt - 1) / 32] >> ((ikotron::bitCnt - 1) % 32)) & 0x01){
				HAL_GPIO_WritePin(IKOTRON_DATA_GPIO_Port, IKOTRON_DATA_Pin, GPIO_PIN_SET);
			}
			else {
				HAL_GPIO_WritePin(IKOTRON_DATA_GPIO_Port, IKOTRON_DATA_Pin, GPIO_PIN_RESET);
			}
			ikotron::phaseCnt--;
			break;
		case 2:
			//CLK auf high
			HAL_GPIO_WritePin(IKOTRON_CLOCK_GPIO_Port, IKOTRON_CLOCK_Pin, GPIO_PIN_SET);
			ikotron::phaseCnt--;
			break;
		case 1:
			//CLK auf low
			HAL_GPIO_WritePin(IKOTRON_CLOCK_GPIO_Port, IKOTRON_CLOCK_Pin, GPIO_PIN_RESET);
			ikotron::phaseCnt = 3;
			ikotron::bitCnt--;
			break;
	}
}

extern "C" void sendLoopWrapper(){
	ikotron::sendLoop();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if (htim->Instance==TIM2){
		ikotron::sendLoop();
	}
}

