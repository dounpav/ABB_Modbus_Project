/*
	===============================================================================
	 Authors      : Pavel Dounaev & Aaro Mäkinen
	 Version     : 1.0
	===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#include <cr_section_macros.h>
#include <atomic>
#include "ModbusMaster.h"
#include "SerialPort.h"
#include "ITM_write.h"
#include "LiquidCrystal.h"
#include "I2C.h"
#include <string>
#include "Mode.h"
#include "Manual.h"
#include "Automatic.h"
#include "sleep.h"
//#include <cstdio.h>

/* SysTick rate in Hz */
#define TICKRATE_HZ         (1000)

static volatile std::atomic_int counter;
static volatile uint32_t systicks;

#ifdef __cplusplus
extern "C" {
#endif
/**  * @brief Handle interrupt from SysTick timer
 * @return Nothing  */

void SysTick_Handler(void) {
	systicks++;
	if(counter > 0) counter--;
}

#ifdef __cplusplus
}

#endif

void Sleep(int ms) {
	counter = ms;
	while(counter > 0) {
		__WFI();
	}
}

uint32_t millis(){
	return systicks;
}

// initializes modbus connection
void initModbus(ModbusMaster* node){

	node->begin(9600); // set transmission rate - other parameters are set inside the object and can't be changed here
	node->writeSingleRegister(0, 0x0406); // prepare for starting: write to control word register(0)

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well

	node->writeSingleRegister(0, 0x047F); // set drive to start mode: write to control word register(0)

	Sleep(1000); // give converter some time to set up
	// note: we should have a startup state machine that check converter status and acts per current status
	//       but we take the easy way out and just wait a while and hope that everything goes well
}

int main(void) {

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
	Board_LED_Set(0, true);
#endif
#endif

	/* Confugring systick*/
	SysTick_Config(Chip_Clock_GetSysTickClockRate() / 1000);
	/*Configurinng RI timer*/
	Chip_RIT_Init(LPC_RITIMER);
	ITM_init();

	/*Configutring pins for LCD*/
	DigitalIoPin rs(0, 8, false, true, false);
	DigitalIoPin en(1, 6, false, true, false);
	DigitalIoPin d4(1, 8, false, true, false);
	DigitalIoPin d5(0, 5, false, true, false);
	DigitalIoPin d6(0, 6, false, true, false);
	DigitalIoPin d7(0, 7, false, true, false);

	/*Configuring pins for Buttons*/
	DigitalIoPin sw1(1, 3, true, true, true);
	DigitalIoPin sw3(0, 9, true, true, true);
	DigitalIoPin sw2(0, 10, true, true, true);
	DigitalIoPin sw4(0, 0, true, true, true);

	/*Configuring LCD*/
	LiquidCrystal lcd(&rs, &en, &d4, &d5, &d6, &d7);
	lcd.begin(16, 2);

	/*Creating I2C object*/
	I2C i2c(0, 100000);

	/*Configuring ModbusMaster object*/
	ModbusMaster node(2); 				// Create modbus object that connects to slave id 2
	initModbus(&node);					// initialize modbus connection to slave

	/*Creating manual object for manual mode*/
	Manual manual(&lcd, &sw1, &sw2, &sw3, &sw4, &i2c, &node);
	/*Creating automatic object for automatic mode*/
	Automatic automatic(&lcd, &sw1, &sw2, &sw3, &sw4, &i2c, &node);

	// MAIN LOOP
	while(true){
		manual.run();
		automatic.run();
	}

	return 0 ;
}
