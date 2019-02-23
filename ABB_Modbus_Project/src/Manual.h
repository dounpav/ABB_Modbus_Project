/*
 * Manual.h
 *
 *  Created on: 9 Mar 2018
 *      Author: pavel
 */

#ifndef MANUAL_H_
#define MANUAL_H_

#include "Mode.h"

// Manual inherits from Mode class

class Manual : public Mode {
public:
	Manual(LiquidCrystal* lcd, DigitalIoPin* sw1, DigitalIoPin* sw2,DigitalIoPin*sw3, DigitalIoPin*sw4, I2C* i2c, ModbusMaster* node);
	void printInfo(uint16_t& cur_speed, int16_t& pressure);
	void run();
	virtual ~Manual();
private:
	bool manual;
};

#endif /* MANUAL_H_ */
