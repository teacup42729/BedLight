#include <IRremoteInt.h>
#include <IRremote.h>
#include <RGB.h>
#include <RGBPWM.h>
#include <Button.h>
#include "PostLight.h"
#include <PWM.h>
#include <Wire.h>
#include <RTC.h>
#include <PWMDriver.h>

RTC rtc;

IRrecv irrecv(3);
decode_results results;
unsigned long previousResult;

PostLight leftPost(9, 10, 5, 2);

HSV overrideColour = {0, 1, 1};

//should come from rtc
HSV leftPostColour = {0, 1, 1};

HSV *irControlTarget;

Button overrideButton(4, true, true, 2);

bool colourOverride = false;

float overrideSpeed = 0.01;

float adjustmentAmount = 0.05;
float hueAdjustmentAmount = 5;

uint32_t lastMillis;
uint32_t lastIRUpdate;

void setup()
{
	Serial.begin(9600);
	RGB.begin(256);
	irrecv.enableIRIn();

	leftPost.begin(leftPostColour, &overrideColour);
}

void loop()
{
	if (millis() == lastMillis) return;
	lastMillis = millis();

	//Serial.println(freeRam());

	processIR();

	overrideButton.update();
	
	overrideColour.hue = fmod((overrideColour.hue + overrideSpeed), 360);

	if(overrideButton.wasPressed())
	{
		colourOverride = !colourOverride;
		leftPost.setOverride(colourOverride);
	}

	leftPost.update();

	//delay(50);
}

int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void processIR()
{
	if (irrecv.decode(&results))
	{
		if (results.value == REPEAT)
		{
			results.value = previousResult;
		}
		switch (results.value)
		{
		case 0xFF22DD: Serial.println("LEFT"); break;
		case 0xFF629D: 
			Serial.println("UP - leftPostColour"); 
			irControlTarget = &leftPostColour;
			break;
		case 0xFFC23D:
			Serial.println("RIGHT - overrideColour");
			irControlTarget = &overrideColour;
			break;
		case 0xFFA857: Serial.println("DOWN"); break;
		case 0xFF02FD: Serial.println("OK"); break;
		case 0xFF6897:
			{
			Serial.println("1 - hue up");
			float hue = irControlTarget->hue + hueAdjustmentAmount;
			while (hue >= 360) hue -= 360;
			irControlTarget->hue = hue;
			}
			break;
		case 0xFF9867:
			{
			Serial.println("2 - saturation up");
			irControlTarget->print();
			float saturation = irControlTarget->saturation + adjustmentAmount;
			if (saturation > 1) saturation = 1;
			irControlTarget->saturation = saturation;
			irControlTarget->print();
			}
			break;
		case 0xFFB04F:
			{
			Serial.println("3 - value up");
			float value = irControlTarget->value + adjustmentAmount;
			if (value > 1) value = 1;
			irControlTarget->value = value;
			}
			break;
		case 0xFF30CF:
			{
			Serial.println("4 - hue down");
			float hue = irControlTarget->hue - hueAdjustmentAmount;
			while (hue < 0) hue += 360;
			irControlTarget->hue = hue;
			}
			break;
		case 0xFF18E7:
			{
			Serial.println("5 - saturation down");
			irControlTarget->print();
			float saturation = irControlTarget->saturation - adjustmentAmount;
			if (saturation < 0) saturation = 0;
			irControlTarget->saturation = saturation;
			irControlTarget->print();
			}
			break;
		case 0xFF7A85:
			{
			Serial.println("6 - value down");
			float value = irControlTarget->value - adjustmentAmount;
			if (value < 0) value = 0;
			irControlTarget->value = value;
			}
			break;
		case 0xFF10EF: Serial.println("7"); break;
		case 0xFF38C7: Serial.println("8"); break;
		case 0xFF5AA5: Serial.println("9"); break;
		case 0xFF42BD: Serial.println("*"); break;
		case 0xFF4AB5: Serial.println("0"); break;
		case 0xFF52AD: Serial.println("#"); break;
		default:
			Serial.print("UNKOWN: ");
			Serial.println(results.value, HEX);
		}
		previousResult = results.value;
		irrecv.resume();
	}
}

