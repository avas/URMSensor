#include "URMSensor.h"

void URMSensor::startMeasure()
{
	// Ignoring the call if this instance already started measuring the distance.
	if (_currentState == WaitingForPulse || _currentState == Measuring) return;
	
	// Failing the measure if this instance is not attached to the sensor, or the ECHO pin
	// is not in IDLE state.
	if (!isAttached() || 
		(fastDigitalReadEcho() == _echoActiveState))
	{
		_currentState = Idle;
		return;
	}

	// Starting the measure.
	_currentState = WaitingForPulse;
	
	fastDigitalWriteTrig(_trigActiveState);
	delayMicroseconds(_trigPulseWidth);
	fastDigitalWriteTrig(getOppositeStateFor(_trigActiveState));
	
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
	byte echoState = fastDigitalReadEcho();

	switch (_currentState)
	{
		case WaitingForPulse:
			if ((getCurrentDuration() > _timeoutForPulseStart) &&
				(echoState != _echoActiveState))
			{
				_currentState = Idle;
			}
			else if (echoState == _echoActiveState)
			{
				_currentState = Measuring;
				resetTime();
			}
			break;
			
		case Measuring:
			if ((getCurrentDuration() > _maxPulseDuration) &&
				(echoState == _echoActiveState))
			{
				_currentState = Idle;
			}
			else if (echoState != _echoActiveState)
			{
				_currentState = FinishedMeasure;
			}
			break;
			
		case FinishedMeasure:
		case Idle:
		default:
			break;
	}
}

boolean URMSensor::finishedMeasure()
{
	// If this instance haven't been attach()'d, reporting failure.
	if (!_isAttached) 
	{
		_currentState = Idle;
		return true;
	}

	// Now we'll poll data and refresh state...
	refreshState();
	
	// ...and tell the user whether we are finished the work or not.
	return (!isMeasuring());
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
	
	while (!finishedMeasure()) ;
	
	return getMeasuredDistance();
}
		
