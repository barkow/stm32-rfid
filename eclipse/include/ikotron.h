/*
 * ikotron.h
 *
 *  Created on: 16.05.2016
 *      Author: barkow
 */

#ifndef INCLUDE_IKOTRON_H_
#define INCLUDE_IKOTRON_H_

#include <stdint.h>
#include "stm32f1xx_hal.h"

class ikotron {
private:
	static uint32_t frame[2];
	static uint8_t bitCnt;
	static uint8_t phaseCnt;
	static TIM_HandleTypeDef* htim;
public:
	static void sendFrame(uint16_t id);
	static void sendLoop();
	static void init(TIM_HandleTypeDef* htim) {
		ikotron::htim = htim;
	}
};



#endif /* INCLUDE_IKOTRON_H_ */
