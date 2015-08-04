// 
// 
// 

#include "PostLight.h"

#define LONG_PRESS 1000

PostLight::PostLight(int rPin, int gPin, int bPin, int buttonPin) 
	: pwm(rPin, gPin, bPin, true), button(buttonPin, true, true, 20)
{

}

void PostLight::begin(HSV _lightColour, HSV *_overrideColour)
{
	pwm.begin();

	currentState = off;

	lightColour.copy(_lightColour);
	overrideColour = _overrideColour;
}


void PostLight::update()
{
	button.update();

	switch (currentState)
	{
	case off:
		if (button.wasReleased()) //short press
		{
			changeState(fadeOn);
		}
		else if (button.pressedFor(LONG_PRESS)) //long press
		{
			fadeUp = true;
			changeState(cycleBrightness);
		}
		break;

	case fadeOn:
		if (button.wasReleased()) //pressed while fading in
		{
			changeState(fadeOut);
		}
		else
		{
			uint32_t currentTime = millis();
			float progress = (currentTime - startFadeTime) / (float)fadeDuration;
			//Serial.print("Progress = ");
			//Serial.println(progress);
			if (progress > 1)
			{
				currentColour.copy(lightColour);
				changeState(on);
			}
			else
			{
				RGB.lerp(startFadeColour, lightColour, progress, &currentColour);
			}
		}
		break;

	case fadeOut:
		if (button.wasReleased()) //pressed while fading out
		{
			changeState(fadeOn);
		}
		else
		{
			uint32_t currentTime = millis();
			float progress = (currentTime - startFadeTime) / (float)fadeDuration;
			//Serial.print("Progress = ");
			//Serial.println(progress);
			if (progress > 1)
			{
				currentColour.copy(&black);
				changeState(off);
			}
			else
			{
				RGB.lerp(startFadeColour, black, progress, &currentColour);
			}
		}
		break;

	case on:
		if (button.wasReleased()) //short press
		{
			changeState(fadeOut);
		}
		currentColour.copy(lightColour);
		break;

	case cycleBrightness:
		if (button.wasReleased())
		{
			lightColour.copy(&currentColour);
			changeState(on);
		}
		else
		{
			uint32_t currentTime = millis();
			float progress = (currentTime - startFadeTime) / (float)fadeDuration;
			if (progress > 1)
			{
				fadeUp = !fadeUp;
				changeState(cycleBrightness);
			}
			else
			{
				if (fadeUp)
				{
					lightColour.value = RGB.lerp(0, 1, progress);
					currentColour.value = lightColour.value;
				}
				else
				{
					lightColour.value = RGB.lerp(1, 0, progress);
					currentColour.value = lightColour.value;
				}
			}
		}
		break;

	case fadeOnOverride:
		{
		uint32_t currentTime = millis();
		float progress = (currentTime - startFadeTime) / (float)fadeDuration;
		//Serial.print("Progress = ");
		//Serial.println(progress);
		if (progress > 1)
		{
			currentColour.copy(overrideColour);
			changeState(override);
		}
		else
		{
			RGB.lerp(startFadeColour, *overrideColour, progress, &currentColour);
		}
		}
		break;

	case fadeOutOverride:
		{
		uint32_t currentTime = millis();
		float progress = (currentTime - startFadeTime) / (float)fadeDuration;
		//Serial.print("Progress = ");
		//Serial.println(progress);
		if (progress > 1)
		{
			currentColour.copy(&black);
			if (lightOn)
			{
				changeState(on);
			}
			else
			{
				changeState(off);
			}
		}
		else
		{
			RGB.lerp(startFadeColour, black, progress, &currentColour);
		}
		}
		break;

	case override:
		currentColour.copy(overrideColour);
		break;
	}

	pwm.setValueHSV(currentColour);
}

void PostLight::changeState(state newState)
{
	Serial.print("newState = ");
	Serial.println(newState);

	switch (newState)
	{
	case off:
		lightOn = false;
		break;

	case fadeOn:
		startFadeTime = millis();
		fadeDuration = 1000;
		startFadeColour.copy(&currentColour);
		lightOn = true;
		break;

	case on:
		lightOn = true;
		break;
	
	case fadeOut:
		startFadeTime = millis();
		fadeDuration = 500;
		startFadeColour.copy(&currentColour);
		black.copy(&startFadeColour);
		black.value = 0;
		lightOn = true;
		break;
		
	case cycleBrightness:
		startFadeTime = millis();
		fadeDuration = 5000;
		lightOn = true;
		break;

	case fadeOnOverride:
		startFadeTime = millis();
		fadeDuration = 1000;
		startFadeColour.copy(&currentColour);
		break;

	case fadeOutOverride:
		startFadeTime = millis();
		fadeDuration = 1000;
		startFadeColour.copy(&currentColour);
		if (lightOn)
		{
			black.copy(lightColour);
		}
		else
		{
			black.copy(&currentColour);
			black.value = 0;
		}
		break;
	}
	currentState = newState;
}

void PostLight::setLightColour(HSV *hsv)
{
	//store colour in rtc
	lightColour.copy(hsv);
	//lightColour.print();

	switch (currentState)
	{
	case off:
		currentColour.copy(hsv);
		currentColour.value = 0;
		break;
	case on:
		currentColour.copy(hsv);
		break;
	}
}

void PostLight::setOverride(bool overridden)
{
	if (overridden && (currentState != fadeOnOverride || currentState != override)) 
	{
		changeState(fadeOnOverride);
	}
	else if (!overridden && (currentState == fadeOnOverride || currentState == override))
	{
		changeState(fadeOutOverride);
	}
}

void PostLight::setOverrideColour(HSV *hsv)
{
	overrideColour->copy(hsv);
}