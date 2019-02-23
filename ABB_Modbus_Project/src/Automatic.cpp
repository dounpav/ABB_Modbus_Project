/*
 * Automatic.cpp
 *
 *  Created on: 9 Mar 2018
 *      Author: pavel
 */

#include "Automatic.h"

Automatic::Automatic(LiquidCrystal* lcd, DigitalIoPin* sw1, DigitalIoPin* sw2,DigitalIoPin*sw3, DigitalIoPin*sw4, I2C* i2c, ModbusMaster* node)
: Mode(lcd, sw1, sw2, sw3, sw4, i2c, node)
{
	pressure_ref = 22;				// sets default reference pressure
}

void Automatic::printInfo(uint16_t& cur_speed, int16_t& pressure, int16_t& pressure_ref, bool reached){

	std::string info_pressure;
	std::string info_reached;
	std::string info_speed;

	int percent_scale = 200;
	int percent_speed =  cur_speed / percent_scale;

	// if desired value cant be reached user will be informed from lcd
	if(!reached){

		lcd->setCursor(0,0);
		info_reached = "CANT REACH VALUE        ";
		lcd->print(info_reached);

	}
	else{
		lcd->setCursor(0,0);
		info_speed = "SPEED: " + std::to_string(percent_speed) + " %	   ";
		lcd->print(info_speed);
	}

	lcd->setCursor(0,1);
	info_pressure = "PC:" + std::to_string(pressure) + "Pa " + "PR:" + std::to_string(pressure_ref) + "Pa   ";
	lcd->print(info_pressure);

}

// Menu for setting reference pressure
void Automatic::setPressure(){

	std::string pressure;
	bool pressure_set = false;

	while(pressure_set == false){

		lcd->setCursor(0,0);
		lcd->print("Change pressure");
		lcd->setCursor(0,1);
		pressure = "   - " +std::to_string(pressure_ref) + "Pa +   ";
		lcd->print(pressure);

		// if sw1 is pressed: decrease reference pressure if possible
		if(sw1->read()){
			while(sw1->read()){
				Sleep(200);

				if(pressure_ref > 0){
					pressure_ref -= 1;
				}
			}
		}

		// if sw2 is pressed current reference pressure will be accepted
		if(sw2->read()){
			while(sw2->read());
			pressure_set = true;
		}

		// if sw3 is pressed: increase reference pressure if possible
		if(sw3->read()){
			while(sw3->read()){
				Sleep(200);

				if(pressure_ref > 0){
					pressure_ref -= 1;
				}
			}
		}

	}

}

void Automatic::run(){

	automatic = true;
	pressure_counter = 0;
	cell = 0;
	timescalled  = 0;

	bool reached = true;

	while(automatic){

		printf("automatic mode\n");

		heartBeat();												// keeping the device alive
		printInfo(cur_speed, pressure_med, pressure_ref, reached); // print current condition of the system

		// if sw2 is pressed move to the pressure set menu
		if(sw2->read()){
			while(sw2->read());
			setPressure();
		}

		// sw4 is pressed: change mode
		if(sw4->read()){
			while(sw4->read());
			automatic = false;
		}

		// gathers 5 pressure values
		while(pressure_counter!=5){

			pressure_counter++;
			pressure_values[cell] = readPressure();
			ITM_write((std::to_string(pressure_values[cell])).c_str()); // for debugging
			cell++;
		}
		pressure_counter = 0;
		cell = 0;

		// calculates median value from pressure_values
		pressure_med = getMedian(pressure_values);

		ITM_write("Median: ");
		ITM_write((std::to_string(pressure_med)).c_str()); // for debugging


		/* Checking if median pressure value is within range
		   if median value has acceptable error value(accuracy) compared to reference pressure
		   in order to increase or decrease the speed
		 */
		if ( (pressure_med - pressure_ref) > accuracy || (pressure_med - pressure_ref) < -accuracy) {

			ITM_write("fan speed can be changed\n");

			// if calculated error is bigger than error, the speed will be decreased
			if ((pressure_med - pressure_ref) > accuracy) {
				if(cur_speed > 0){
					if((pressure_med - pressure_ref) >=40 && cur_speed >= 4000){
						cur_speed -= 4000;
					}
					else if((pressure_med - pressure_ref) >=30 && (pressure_med - pressure_ref) <40 && cur_speed >=3000){
						cur_speed -= 3000;
					}
					else if((pressure_med - pressure_ref) >=20 && (pressure_med - pressure_ref) <30 && cur_speed >=2000) {
						cur_speed -= 2000;
					}
					else if((pressure_med - pressure_ref) >=10 && (pressure_med - pressure_ref) <20 && cur_speed >= 1000){
						cur_speed -= 1000;
					}
					else{
						cur_speed -= 100;
						timescalled = 0;
					}
					setFrequency(cur_speed);
				}

				// if calculated error is smaller than error, the speed will be decreased
			} else {
				if(cur_speed < max_speed){
					timescalled = 0;

					if((pressure_ref - pressure_med) >=40 && cur_speed < (max_speed-4000)){
						cur_speed += 4000;
					}
					else if((pressure_ref - pressure_med) >=30 && (pressure_ref - pressure_med) <40 && cur_speed < (max_speed-3000)){
						cur_speed += 3000;
					}
					else if((pressure_ref - pressure_med) >=20 && (pressure_ref - pressure_med) <30 && cur_speed < (max_speed-2000)){
						cur_speed += 2000;
					}
					else if((pressure_ref - pressure_med) >=10 && (pressure_ref - pressure_med) <20 && cur_speed < (max_speed-1000)){
						cur_speed += 1000;
					}
					else{
						cur_speed = cur_speed + 100;
						ITM_write("OK\n");
					}
					setFrequency(cur_speed);
				}
				else{
					timescalled++;
					ITM_write(std::to_string(timescalled).c_str());
					ITM_write("\n");
				}
			}

		}

		if(timescalled > 6){
			ITM_write("Can't reach the desired value\n");
			reached = false;
		}
		else reached = true;
	}

}

Automatic::~Automatic() {
	// TODO Auto-generated destructor stub
}

