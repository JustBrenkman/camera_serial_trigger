/*
 * numberEditor.c
 *
 * Created: 8/6/2020 3:30:14 PM
 *  Author: chadb
 */ 

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "buttonDriver.h"
#include "printf.h"
#include "displayDriver.h"
#include "globalVar.h"
#include "statusDriverLine.h"
#define START_COL 3
#define MAX_EDIT_COL 4
#define MAX_EDIT_COL_INT 3
static bool floatFunc = false;
static void (*returnIntFunction)(uint8_t);
static void (*returnFloatFunction)(float);
static float myFloat = 0.0;
static uint8_t myInt = 0;
static float stepValue = 0.0;
static float minValue = 0.0;
static float maxValue = 0.0;
static char title[21];
static uint8_t currentCollumn = 0;
static uint8_t col = 0;




static void copyTitle(char* string){
	for(uint8_t i = 0; i < 21; i++){
		title[i] = string[i];
	}
}




static void setEditColumn(uint8_t newColumn){
	displayDriver_setCurserPos(1, currentCollumn);
	displayDriver_writeChar(' ');
	displayDriver_setCurserPos(3, currentCollumn);
	displayDriver_writeChar(' ');
	displayDriver_writeCustChar(0, 1, newColumn);
	displayDriver_writeCustChar(1, 3, newColumn);
	displayDriver_setCurserPos(2, newColumn);
	currentCollumn = newColumn;
}

static void incrementDisplay(){
	bool doubleBack = false;
	unsigned char newChar = (displayDriver_getCurrentChar() + 1);
	if(newChar == '/'){
		displayDriver_curserLeft();
		newChar = displayDriver_getCurrentChar() + 1;
		doubleBack = true;
	}
	if(newChar > '9'){
		displayDriver_curserLeft();
		incrementDisplay();
		displayDriver_curserRight();
		newChar = '0';
	}
	displayDriver_writeChar(newChar);
	displayDriver_curserLeft();
	if(doubleBack){
		displayDriver_curserRight();
	}
}

static void decrementDisplay(){
	bool doubleBack = false;
	unsigned char newChar = (displayDriver_getCurrentChar() - 1);
	if(newChar == '-'){
		displayDriver_curserLeft();
		newChar = displayDriver_getCurrentChar() - 1;
		doubleBack = true;
	}
	if(newChar < '0'){
		displayDriver_curserLeft();
		decrementDisplay();
		displayDriver_curserRight();
		newChar = '9';
	}
	displayDriver_writeChar(newChar);
	displayDriver_curserLeft();
	if(doubleBack){
		displayDriver_curserRight();
	}	
}

static void right(){
	uint8_t maxCol;
	if(floatFunc){
		maxCol = MAX_EDIT_COL;
	} else {
		maxCol = MAX_EDIT_COL_INT;
	}
	if(col < (maxCol)){
		col++;
		stepValue = stepValue / 10;
		setEditColumn(col + START_COL);
		if(displayDriver_getCurrentChar() == '.'){
			col++;
			setEditColumn(col + START_COL);
		}
	}	
}

static void left(){
	if(col > 0){
		col--;
		stepValue = stepValue * 10;
		setEditColumn(col + START_COL);
		if(displayDriver_getCurrentChar() == '.'){
			col--;
			setEditColumn(col + START_COL);
		}
	}	
}

static void increment(){
	float temp = myFloat + stepValue;
	if(!floatFunc){
		temp = myInt + stepValue;
	}
	if(temp <= maxValue){
		incrementDisplay();
		if(floatFunc){
			myFloat = temp;
		} else {
			myInt = (uint8_t) temp;
		}
	}
}

static void decrement(){
	float temp = myFloat - stepValue;
	if(!floatFunc){
		temp = myInt - stepValue;
	}
	if(temp >= minValue){
		decrementDisplay();
		if(floatFunc){
			myFloat = temp;
		} else {
			myInt = (uint8_t) temp;
		}
	}

}

void intReturnCall(){
	returnIntFunction(myInt);
}

void floatReturnCall(){
	returnFloatFunction(myFloat);
}

void numberEditor_initFloat(float oldValue, void (*returnFunction)(float), handler backFunction, char* valueName, char* valueLabel, float min, float max){
	statusDriverLine_setStatus(false);
	floatFunc = true;
	col = 4;
	myFloat = oldValue;
	returnFloatFunction = returnFunction;
	displayDriver_disableCurser();
	copyTitle(valueName);
	minValue = min;
	maxValue = max;
	displayDriver_clearDispaly();
	displayDriver_tick();
	displayDriver_writeString(title);
	char myString[7];
	sprintf(myString, "%05.2f", myFloat);
	displayDriver_setCurserPos(2, START_COL);
	displayDriver_writeString(myString);
	displayDriver_writeChar(' ');
	displayDriver_writeString(valueLabel);
	setEditColumn(START_COL + MAX_EDIT_COL);
	stepValue = 0.01;
	buttonDriver_clearAllHandlers();
	buttonDriver_addHandler(3, true, increment);
	buttonDriver_addHandler(5, true, decrement);
	buttonDriver_addHandler(2, true, right);
	buttonDriver_addHandler(6, true, left);
	buttonDriver_addHandler(4, true, floatReturnCall);
	buttonDriver_addHandler(1, true, backFunction);
}



void numberEditor_initInt(uint8_t oldValue, void (*returnFunction)(uint8_t), handler backFunction, char* valueName, char* valueLabel, uint8_t min, uint8_t max){
		statusDriverLine_setStatus(false);
		floatFunc = false;
		col = 3;
		myInt = oldValue;
		returnIntFunction = returnFunction;
		displayDriver_disableCurser();
		copyTitle(valueName);
		minValue = min;
		maxValue = max;
		displayDriver_clearDispaly();
		displayDriver_tick();
		displayDriver_writeString(title);
		char myString[7];
		sprintf(myString, "%04d", myInt);
		displayDriver_setCurserPos(2, START_COL);
		displayDriver_writeString(myString);
		displayDriver_writeChar(' ');
		displayDriver_writeString(valueLabel);
		setEditColumn(START_COL + MAX_EDIT_COL_INT);
		stepValue = 1;
		buttonDriver_clearAllHandlers();
		buttonDriver_addHandler(3, true, increment);
		buttonDriver_addHandler(5, true, decrement);
		buttonDriver_addHandler(2, true, right);
		buttonDriver_addHandler(6, true, left);
		buttonDriver_addHandler(4, true, intReturnCall);
		buttonDriver_addHandler(1, true, backFunction);
}

void numberEditor_tick();