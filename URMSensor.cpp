#include "URMSensor.h"

#include "HardwareSerial.h"

void URMSensor::startMeasure()
{
	// Ignoring the call if this instance already started measuring the distance.
	if (_currentState == WaitingForPulse || _currentState == Measuring) return;
	
	// Failing the measure if this instance is not attached to the sensor, or the ECHO pin
	// is not in IDLE state.
	if (!isAttached())// || 
		//(digitalRead(_echoPin) == _echoActiveState))
	{
		_currentState = Idle;
		return;
	}

	// Starting the measure 
	_currentState = WaitingForPulse;
	
	digitalWrite(_trigPin, _trigActiveState);
	delayMicroseconds(_trigPulseWidth);
	digitalWrite(_trigPin, getOppositeStateFor(_trigActiveState));
	
	digitalWrite(11, getOppositeStateFor(_echoActiveState));
	digitalWrite(8, LOW);
	
	resetTime();
	refreshState();
}

boolean URMSensor::isMeasuring()
{
	return (_currentState == WaitingForPulse || _currentState == Measuring);
}

void URMSensor::interruptMeasure()
{
	_currentState = Idle;
}

void URMSensor::refreshState()
{
	digitalWrite(12, HIGH);

	byte echoState = digitalRead(_echoPin);

	switch (_currentState)
	{
		case WaitingForPulse:
			if ((getCurrentDuration() > _timeoutForPulseStart) &&
				(echoState != _echoActiveState))
			{
				_currentState = Idle;
				digitalWrite(8, HIGH);
			}
			else if (echoState == _echoActiveState)
			{
				_currentState = Measuring;
				resetTime();
				
				digitalWrite(11, _echoActiveState);
			}
			break;
			
		case Measuring:
			if ((getCurrentDuration() > _maxPulseDuration) &&
				(echoState == _echoActiveState))
			{
				_currentState = Idle;
				digitalWrite(8, HIGH);
			}
			else if (echoState != _echoActiveState)
			{
				_currentState = FinishedMeasure;
				digitalWrite(11, getOppositeStateFor(_echoActiveState));
			}
			break;
			
		case FinishedMeasure:
		case Idle:
		default:
			break;
	}
	
	digitalWrite(12, LOW);
}

boolean URMSensor::finishedMeasure(unsigned long* distance)
{
	// If this instance haven't been attach()'d, reporting failure.
	if (!isAttached()) 
	{
		_currentState = Idle;
		*distance = URM_INVALID_VALUE;
		return true;
	}

	// Now we'll poll data and refresh state...
	refreshState();
	if (isMeasuring()) return false;
	
	// ...and, if this instance isn't doing measure anymore, return appropriate value.
	*distance = getMeasuredDistance();
	return true;
}

unsigned long URMSensor::getMeasuredDistance()
{
	if (_currentState != FinishedMeasure) return URM_INVALID_VALUE;
	
	return convertCurrentDurationToDistance();
}



/**
 * Synchronously gets the distance in front of the sensor. Maximal working time for 
 * this method is (timeoutForPulseStart + maxPulseDuration) us. This method is suitable
 * for simple applications or for situations when you just need to get distance and 
 * don't mind if the board's core will be busy for 40-50 ms doing it.
 *
 * @return Distance in front of the sensor in centimeters, or URM_INVALID_VALUE
 * in case of failure.
 */
unsigned long URMSensor::measureDistance()
{
	startMeasure();
	
	unsigned long distance;
	while (!finishedMeasure(&distance)) ;
	
	return distance;
}
		
