/*
 * eventEditor.c
 *
 * Created: 8/12/2020 2:34:21 PM
 *  Author: chadb
 */ 
#include "eventEditor.h"
#include "printf.h"
#include "menuDriver.h"
#include "eepromDriver.h"
#include "mainDisplay.h"
#include "displayDriver.h"
#include "statusDriverLine.h"
#include "buttonDriver.h"
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>

static uint8_t lastMenuItem = 0;
static uint8_t lastIndex = 0;
static uint32_t types = 0;


static uint8_t getEventType(uint32_t eventTypes, uint8_t index){
	uint8_t temp = (uint8_t) (eventTypes >> (index * 2));
	return (temp & 0b11);
}

static void setEventType(uint32_t* eventTypes, uint8_t index, uint8_t newEvent){
	uint32_t mask = 0b11;
	uint32_t temp = (uint32_t) newEvent;
	temp = temp << (index * 2);
	mask = mask << (index * 2);
	*eventTypes &= ~mask;
	*eventTypes |= temp;
}


static void back(){
	eepromDriver_writeDword(GLOBALVAR_TIMING_SETPOINTS, types);
	mainDisplay_reopenMenu();
}

static void returnToEventEditor(){
	menuDriver_setIndex(lastMenuItem);
	eventEditor_init();
}

static void handleTypeSet(){
	setEventType(&types, lastIndex, menuDriver_getLastMenuIndex());
	returnToEventEditor();
}

void eventEditor_loadEeprom(){
	types = eepromDriver_readDword(GLOBALVAR_TIMING_SETPOINTS);
}


static void handleType(){
	lastMenuItem = menuDriver_getLastMenuIndex();
	lastIndex = lastMenuItem / 2;
	displayDriver_curserHome();
	uint8_t event = lastIndex + 1;
	char title[21];
	sprintf(title, "Edit Type: Event %02d", event);
	displayDriver_writeString(title);
	menuDriver_configure(4, returnToEventEditor);
	menuDriver_setIndex(getEventType(types, lastIndex));
	menuDriver_addItem("Enable Shutter   \0", handleTypeSet, GLOBALVAR_EN_SHUT);
	menuDriver_addItem("Enable Focus     \0", handleTypeSet, GLOBALVAR_EN_FOC);
	menuDriver_addItem("Disable Shutter  \0", handleTypeSet, GLOBALVAR_DS_SHUT);
	menuDriver_addItem("Disable Focus    \0", handleTypeSet, GLOBALVAR_DS_FOC);
	menuDriver_drawMenu();
}

static void setTiming(float newTime){
	eepromDriver_writeFloat(GLOBALVAR_TIMING_BEGIN_MEM_SLOT + lastIndex, newTime);
	returnToEventEditor();
}

static void handleTiming(){
	lastMenuItem = menuDriver_getLastMenuIndex();
	lastIndex = lastMenuItem / 2;
	displayDriver_curserHome();
	uint8_t event = lastIndex + 1;
	char title[21];
	sprintf(title, "Edit Time: Event %02d", event);
	float oldValue = eepromDriver_readFloat(lastIndex + GLOBALVAR_TIMING_BEGIN_MEM_SLOT);
	numberEditor_initFloat(oldValue, setTiming, eventEditor_init, title, "Seconds\0", 0.02, 99.99);
}

void eventEditor_init(){
	cli();
	statusDriverLine_setStatus(false);
	displayDriver_curserHome();
	displayDriver_writeString("   Timing Editor   \0");
	uint8_t eventCount = eepromDriver_readDword(GLOBALVAR_TIMING_SET_COUNT);
	eventCount *= 2;
	menuDriver_configure(eventCount, back);
	for(uint8_t i = 0; i < eventCount; i += 2){
		uint8_t eventNumber = ((i / 2) + 1);
		char eventTitleString[20];
		char timingTitleString[20];
		sprintf(eventTitleString, "Event %02d type", eventNumber);
		sprintf(timingTitleString, "Event %02d time", eventNumber);
		menuDriver_addItem(eventTitleString, handleType, i);
		menuDriver_addItem(timingTitleString, handleTiming, i + 1);
		
	}
	menuDriver_drawMenu();
	sei();
}