/*
 * Mode.cpp
 *
 *  Created on: 8 Mar 2018
 *      Author: pavel
 */

#include "Mode.h"


Mode::Mode(LiquidCrystal* lcd, DigitalIoPin* sw1, DigitalIoPin* sw2,DigitalIoPin*sw3, DigitalIoPin*sw4, I2C* i2c, ModbusMaster* node) {

	this->lcd = lcd;
	this->sw1 = sw1;
	this->sw2 = sw2;
	this->sw3 = sw3;
	this->sw4 = sw4;
	this->i2c = i2c;
	this->node = node;

}

// function for keeping the device running/used for debugging
void Mode::printRegister(uint16_t reg){

	uint8_t result;
	// slave: read 16-bit registers starting at reg to RX buffer
	result = node->readHoldingRegisters(reg, 1);

	// do something with data if read is successful
	if (result == node->ku8MBSuccess)
	{
		printf("R%d=%04X\n", reg, node->getResponseBuffer(0));
	}
	else {
		printf("R%d=???\n", reg);
	}

}

// function for setting the speed of the fan
bool Mode::setFrequency(uint16_t freq){

	uint8_t result;										// used for debugging
	int ctr;
	bool atSetpoint;
	const int delay = 1;

	node->writeSingleRegister(1, freq);					// set motor frequency

	printf("Set freq = %d\n", freq/40); 				// used for debugging

	// wait until we reach set point or timeout occurs
	ctr = 0;
	atSetpoint = false;
	do {
		Sleep(delay);
		// read status word
		result = node->readHoldingRegisters(3, 1);
		// check if we are at setpoint
		if (result == node->ku8MBSuccess) {
			if(node->getResponseBuffer(0) & 0x0100) atSetpoint = true;
		}
		ctr++;
	} while(ctr < 20 && !atSetpoint);

	printf("Elapsed: %d\n", ctr * delay); 				// used for debugging

	return atSetpoint;

}

// function for reading pressure
int16_t Mode::readPressure(){

	uint8_t pressureData[3];
	uint8_t readPressureCmd = 0xF1;
	int16_t pressure = 0;

	// float(pressure / 240) * 0.95)
	const int scale_factor= 240;
	const double altitude_correction = 0.95;

	if (i2c->transaction(0x40, &readPressureCmd, 1, pressureData, 3)) {
		pressure = (pressureData[0] << 8) | pressureData[1];						// calculates pressure
		pressure = pressure / 240 * 0.95;
		printf("Pressure: %d\n", pressure);											// used for debugging
	}
	else {
		printf("Error reading pressure\n");
	}
	Sleep(10);
	return pressure;
}

// function for printing system information
void Mode::printInfo(){
}

// calculating median of the pressure values
int16_t Mode::getMedian(int16_t* pressure_values){

	int n = 5;								// size of the array
	int16_t temp;							// temporary variable

	// sorts array in ascending oreder
	for (int i = 0; i < n; i++){
		for(int j=i; j < n; j++){
			if(pressure_values[i] > pressure_values[j]){
				temp = pressure_values[i];
				pressure_values[i] = pressure_values[j];
				pressure_values[j] = temp;
			}
		}
	}
	// returns median value of the array
	return pressure_values[2];

}

// function used for keeping the modbus alive
void Mode::heartBeat(){
	node->readHoldingRegisters(102, 1);
}

// running current mode
void Mode::run(){
}

Mode::~Mode() {
	// TODO Auto-generated destructor stub
}

