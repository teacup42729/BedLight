// PostLight.h

#ifndef _POSTLIGHT_h
#define _POSTLIGHT_h

#include "arduino.h"

#include <PWM.h>
#include <Button.h>
#include <RGB.h>
#include <RGBPWM.h>

class PostLight
{
	enum state
	{
		off,
		cycleBrightness,
		fadeOn,
		on,
		fadeOut,
		fadeOnOverride,
		override,
		fadeOutOverride
	};

public:
	PostLight(int rPin, int gPin, int bPin, int buttonPin);
	void begin(HSV lightColour, HSV *overrideColour);

	void update();
	
	void setOverrideColour(HSV *hsv);
	void setLightColour(HSV *hsv);

	void setOverride(bool isOverridden);

protected:
	RGBPWM pwm;
	Button button;

	HSV currentColour;
	HSV startFadeColour;
	HSV *overrideColour;
	HSV lightColour;
	HSV black;

	void applyColour();

	void changeState(state newState);
	
	state currentState;
	bool lightOn = false;

	uint32_t startFadeTime;
	uint32_t fadeDuration;
	bool fadeUp;
};

#endif

