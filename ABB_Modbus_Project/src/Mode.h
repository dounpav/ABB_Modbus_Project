/*
 * Mode.h
 *
 *  Created on: 8 Mar 2018
 *      Author: pavel
 */

#ifndef MODE_H_
#define MODE_H_

#include "ModbusMaster.h"
#include "SerialPort.h"
#include "ITM_write.h"
#include "LiquidCrystal.h"
#include "I2C.h"
#include <string>
#include "sleep.h"

class Mode {
public:
	Mode(LiquidCrystal* lcd, DigitalIoPin* sw1, DigitalIoPin* sw2,DigitalIoPin*sw3, DigitalIoPin*sw4, I2C* i2c, ModbusMaster* node);
	void printRegister(uint16_t reg);
	bool setFrequency(uint16_t freq);
	int16_t readPressure();
	void printInfo();
	int16_t getMedian(int16_t* pressure_values);
	void heartBeat();
	void run();
	virtual ~Mode();

protected:
	LiquidCrystal* lcd;
	DigitalIoPin* sw1;
	DigitalIoPin* sw2;
	DigitalIoPin* sw3;
	DigitalIoPin* sw4;
	I2C* i2c;
	ModbusMaster* node;

	const uint16_t max_speed = 20000;			// max speed of the fan
	uint16_t cur_speed;							// current speed of the fan

	int16_t pressure_values[5];					// pressure values used for calculating median
	int16_t pressure_med;						// median pressure
	int pressure_counter;						// counter for counting pressure values
	int cell;									// index of the pressure value

	// button states
	bool sw1_state;
	bool sw3_state;
	bool sw4_state;

};

#endif /* MODE_H_ */
