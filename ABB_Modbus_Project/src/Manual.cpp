/*
 * Manual.cpp
 *
 *  Created on: 9 Mar 2018
 *      Author: pavel
 */

#include "Manual.h"

Manual::Manual(LiquidCrystal* lcd, DigitalIoPin* sw1, DigitalIoPin* sw2,DigitalIoPin*sw3, DigitalIoPin*sw4, I2C* i2c, ModbusMaster* node)
: Mode(lcd, sw1, sw2, sw3, sw4, i2c, node){

	//cur_speed = 5000; // setting default speed
}

void Manual::printInfo(uint16_t& cur_speed, int16_t& pressure){

	int percent_scale = 200;
	int percent_speed =  cur_speed / percent_scale;		// current speed as precent value

	std::string info_speed;								// current speed of the fan used for printing
	std::string info_pressure;							// current pressusre in the system

	lcd->setCursor(0,0);
	info_speed = " - SPEED: " + std::to_string(percent_speed) + "% +";
	lcd->print(info_speed);
	lcd->setCursor(0,1);
	info_pressure = "PRESSURE: " + std::to_string(pressure) + " Pa ";
	lcd->print(info_pressure);

}

// Runs manual mode
void Manual::run(){

	manual = true;
	cur_speed = 5000;
	pressure_med = 0;
	pressure_counter = 0;
	cell = 0;

	bool sw1_state = false;
	bool sw3_state = false;
	bool sw4_state = false;

	// put fan to default speed = 5000
	// function is called twice because because of the modbus bug
	setFrequency(cur_speed);
	setFrequency(cur_speed);

	while(manual){

		printf("manual mode\n");						// used for debugging

		// Reading five pressure values and at the same time polling buttons
		while(pressure_counter!=5){
			if(sw1->read()){
				sw1_state=true;
			}
			if(sw3->read()){
				sw3_state=true;
			}
			if(sw4->read()){
				sw4_state=true;
			}
			pressure_counter++;
			pressure_values[cell] = readPressure();
			ITM_write((std::to_string(pressure_values[cell])).c_str()); // for debugging
			cell++;
		}
		pressure_counter = 0;
		cell = 0;
		pressure_med = getMedian(pressure_values); 				// read pressure from i2c
		printInfo(cur_speed, pressure_med); 					// print current condition of the system
		heartBeat();											// keeping the device alive

		// IF SW1 BUTTON IS PRESSED DECREASE SPEED OF THE FAN
		if(sw1->read() || sw1_state == true){
			sw1_state=false;
			if (cur_speed == 0){
				// don't decrease speed/freq of the fan
			}
			else {
				cur_speed = cur_speed - 100;
				setFrequency(cur_speed);
			}
			ITM_write("sw1 pressed");
		}

		// IF SW3 BUTTON IS PRESSED: INCREASE SPEED OF THE FAN
		if(sw3->read() || sw3_state == true){
			sw3_state=false;
			if (cur_speed > max_speed){
				// don't decrease speed/freq of the fan
			}
			else {
				cur_speed = cur_speed + 100;
				setFrequency(cur_speed);

			}
			ITM_write("sw3 pressed");
		}
		// SW4 PRESSED EXITS THE LOOP
		if(sw4->read() || sw4_state == true){
			sw4_state=false;
			while(sw4->read());
			manual = false;
			printf("sw4 pressed");
		}
	}

}

Manual::~Manual() {
	// TODO Auto-generated destructor stub
}

