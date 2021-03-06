#ifndef URMSENSOR_H
#define URMSENSOR_H

#include "Arduino.h"

/**
 * Value that marks invalid result of the measure. Receiving this value as the measurement result
 * usually means that the URMSensor class either detected invalid state of the sensor pins on 
 * startMeasure() call, timed out waiting for sensor's response when starting measure, or
 * did not receive echo from sensor in time.
 */
#define URM_INVALID_VALUE 0xFFFFFFFF

/**
 * Unlocks direct work with port registers instead of digitalRead() and digitalWrite().
 * This will make refreshState() and finishedMeasure() to work a bit faster (for about 5-10 us), 
 * but the URMSensor class will consume more RAM (about 12 bytes per instance), and the code 
 * will no more be compatible with Arduino Due.
 */
#define URM_USE_PORTS_DIRECTLY


/**
 * Constant used to convert pulse width from DFRobot URM37 sensor (in PWM mode) to range.
 */
#define URM37_US_PER_CM 50 // us/cm (from datasheet)

#define URM37_TRIG_ACTIVE_STATE LOW
#define URM37_ECHO_ACTIVE_STATE LOW

#define URM37_TRIG_PULSE_WIDTH 1 // us

#define URM37_TIMEOUT_FOR_PULSE_START 50000 // us
#define URM37_MAX_PULSE_WIDTH 45000 // us

/**
 * Constant used to convert pulse width from HC-SR04 sensor to range.
 */
#define HC_SR04_US_PER_CM 61 // us/cm (1e6 us/s / (100 cm/m * 330 m/s / 2))

#define HC_SR04_TRIG_ACTIVE_STATE HIGH
#define HC_SR04_ECHO_ACTIVE_STATE HIGH

#define HC_SR04_TRIG_PULSE_WIDTH 10 // us

#define HC_SR04_TIMEOUT_FOR_PULSE_START 10000 // us
#define HC_SR04_MAX_PULSE_WIDTH HC_SR04_US_PER_CM * 450

/**
 * Possible states of any instance of class URMSensor. You can use it to interpret
 * the getState() return value.
 */
enum URMState
{
	/**
	 * The library is idle. Usually this state will be encountered only between 
	 * startup and first startMeasure() call, after interruptMeasure() call or
	 * after encountering any errors while doing measure. The finishedMeasure() 
	 * method will return true and URM_INVALID_VALUE in this state.
	 */
	Idle,
	
	/**
	 * The library did receive startMeasure() call and is now waiting for ECHO line
	 * to switch to the ACTIVE state. The finishedMeasure() method will return false
	 * in this state.
	 */
	WaitingForPulse,
	
	/**
	 * The library did receive startMeasure() call, received pulse start from the sensor
	 * and started to measure it's width. The finishedMeasure() method will
	 * return false in this state.
	 * 
	 */
	Measuring,
	
	/**
	 * The library did successfully measure the distance. The finishedMeasure() method 
	 * will return true and measured distance in this state.
	 */
	FinishedMeasure
};

/**
 * A class representing single ultrasonic ranging sensor.
 *
 * @author Andrey A. Vasenev
 */
class URMSensor
{
	public:
		/**
		 * Constructor.
		 */
		URMSensor()
		{
			_isAttached = false;
		}
		
		
		// ====== Attach- and detach-related methods ==================================================================
		
		/**
		 * Attaches the sensor to physical pins of the target Arduino board. You should call this method
		 * inside the setup() function of your sketch for every instance of this class - otherwise it
		 * will not work.
		 *
		 * @param trigPin Number of the Arduino pin connected to the TRIG pin on the sensor.
		 *
		 * @param echoPin Number of the Arduino pin connected to the ECHO/PWM pin on the sensor.
		 *
		 * @param usPerCm Constant that will be used to convert pulse width to distance in centimeters (us/cm).
		 * For DFRobot URM37, you can use the value URM37_US_PER_CM; for HC-SR04, you can use the value
		 * HC_SR04_US_PER_CM. For other sensors, you should take this constant from the datasheet.
		 *
		 * @param timeoutForPulseStart Maximal allowed time between calling startMeasure() and
		 * receiving response from sensor (ECHO pin must change its state from IDLE to ACTIVE),
		 * in microseconds.
		 *
		 * @param maxPulseDuration Maximal allowed duration of pulse received from sensor (in microseconds).
		 */
		void attach(byte trigPin, byte echoPin, int usPerCm, unsigned long timeoutForPulseStart, 
			unsigned long maxPulseDuration, byte trigActiveState, byte echoActiveState,
			unsigned int trigPulseWidth)
		{
			setTrigPin(trigPin);
			setEchoPin(echoPin);
			
			_usPerCm = usPerCm;
			
			_timeoutForPulseStart = timeoutForPulseStart;
			_maxPulseDuration = maxPulseDuration;
			
			_trigActiveState = trigActiveState;
			_echoActiveState = echoActiveState;
			
			_trigPulseWidth = trigPulseWidth;
			
			_isAttached = true;
		}
			
		/**
		 * Detaches the sensor from physical pins of the target Arduino board. This instance of 
		 * the URMSensor class will not be able to communicate with the sensor after calling 
		 * this method.
		 */
		void detach()
		{
			_isAttached = false;
		}
		
		/**
		 * Indicates whether this instance is attached to sensor.
		 *
		 * @return true if this instance is attached to sensor.
		 */
		boolean isAttached()
		{
			return _isAttached;
		}
		
		
		
		// ====== Methods for asynchronous data reading ===============================================================
		
		/**
		 * Starts the measure.
		 */
		void startMeasure();
		
		/**
		 * Indicates whether this instance is currently doing the measure.
		 *
		 * @return true if this instance currently measures the distance.
		 */
		inline boolean isMeasuring();
		
		/**
		 * Interrupts the measure.
		 */
		void interruptMeasure();
		
		/**
		 * Refreshes the state of this instance. If the ECHO pin of the sensor is connected
		 * to the interrupt- or PCINT-enabled pin, you can call this method from the interrupt
		 * handler function. You can also call this method by timer or in your loop() function,
		 * just ensure that you can call this method frequently enough.
		 */
		void refreshState();
		
		/**
		 * Indicates whether this instance finished the measure. If this method returned true,
		 * you can retrieve measured distance by calling getMeasuredDistance().
		 *
		 * @return true if this instance is not currently measuring the distance.
		 */
		boolean finishedMeasure();
		
		/**
		 * Simply retrieves the value from the previous measure.
		 *
		 * @return Distance from the previous measure, or URM_INVALID_VALUE if 
		 * the previous measure failed or if this instance is currently measuring 
		 * the distance.
		 */
		unsigned long getMeasuredDistance();
		
		
		
		// ====== Synchronous distance reading ========================================================================
		
		/**
		 * Synchronously gets the distance in front of the sensor. Maximal working time for 
		 * this method is (timeoutForPulseStart + maxPulseDuration) us. This method is suitable
		 * for simple applications or for situations when you just need to get distance and 
		 * don't mind if the board's core will be busy for 40-50 ms doing it.
		 *
		 * @return Distance in front of the sensor in centimeters, or URM_INVALID_VALUE
		 * in case of failure.
		 */
		unsigned long measureDistance();
		
		// ====== Debugging-related methods ===========================================================================
		
		/**
		 * Retrieves current state of this instance. May be useful for debugging
		 * any sensor- or library-related issues.
		 */
		byte getState()
		{
			return _currentState;
		}
		
	private:
		boolean _isAttached;
		
	#ifndef URM_USE_PORTS_DIRECTLY
		byte _trigPin;
		byte _echoPin;
	#else	
		byte _trigMask;
		volatile byte* _trigPORT;
		volatile byte* _trigPIN;
		volatile byte* _trigDDR;
		
		byte _echoMask;
		volatile byte* _echoPORT;
		volatile byte* _echoPIN;
		volatile byte* _echoDDR;
	#endif
	
		void setTrigPin(byte trigPin)
		{
		#ifdef URM_USE_PORTS_DIRECTLY
			_trigMask = digitalPinToBitMask(trigPin);
			
			byte trigPortIndex = digitalPinToPort(trigPin);
			_trigPORT = portOutputRegister(trigPortIndex);
			_trigPIN = portInputRegister(trigPortIndex);
			_trigDDR = portModeRegister(trigPortIndex);
		#else
			_trigPin = trigPin;
		#endif

			setTrigMode(OUTPUT);
		}
		
		byte fastDigitalReadTrig()
		{
			byte state = LOW;
			setTrigMode(INPUT);
			
		#ifdef URM_USE_PORTS_DIRECTLY
			return ((*_trigPIN & _trigMask) != 0) ? HIGH : LOW;
		#else
			state = digitalRead(_trigPin);
		#endif
		
			setTrigMode(OUTPUT);
			return state;
		}
		
		void setTrigMode(byte newMode)
		{
		#ifdef URM_USE_PORTS_DIRECTLY
			if (newMode == OUTPUT)
			{
				*_trigDDR |= _trigMask;
			}
			else
			{
				*_trigDDR &= ~_trigMask;
			}
		#else
			pinMode(_trigPin, newMode);
		#endif
		}
		
		void fastDigitalWriteTrig(byte value)
		{
		#ifdef URM_USE_PORTS_DIRECTLY
			if (value == HIGH)
				*_trigPORT |= _trigMask;
			else
				*_trigPORT &= ~_trigMask;
		#else
			digitalWrite(_trigPin, value);
		#endif
		}
		
		void setEchoPin(byte echoPin)
		{
		#ifdef URM_USE_PORTS_DIRECTLY
			_echoMask = digitalPinToBitMask(echoPin);
			
			byte echoPortIndex = digitalPinToPort(echoPin);
			_echoPORT = portOutputRegister(echoPortIndex);
			_echoPIN = portInputRegister(echoPortIndex);
			_echoDDR = portModeRegister(echoPortIndex);
			
			*_echoDDR &= ~_echoMask;
			*_echoPORT &= ~_echoMask;
		#else
			_echoPin = echoPin;
			pinMode(_echoPin, INPUT);
		#endif
		}
		
		byte fastDigitalReadEcho()
		{
		#ifdef URM_USE_PORTS_DIRECTLY
			return ((*_echoPIN & _echoMask) != 0) ? HIGH : LOW;
		#else
			return digitalRead(_echoPin);
		#endif
		}
		
		int _usPerCm;
		
		unsigned long _timeoutForPulseStart;
		unsigned long _maxPulseDuration;
		
		byte _trigActiveState;
		byte _echoActiveState;
		
		byte getOppositeStateFor(byte activeState)
		{
			return (activeState == HIGH) ? LOW : HIGH;
		}
		
		unsigned int _trigPulseWidth;
		
		unsigned long _startMeasureTime;
		unsigned long _currentDuration;
		
		void resetTime()
		{
			_currentDuration = 0;
			_startMeasureTime = micros();
		}
		
		unsigned long getCurrentDuration()
		{
			_currentDuration = micros() - _startMeasureTime;
			return _currentDuration;
		}
		
		unsigned long convertCurrentDurationToDistance()
		{
			return _currentDuration / _usPerCm;
		}

		byte _currentState;
};

class URM37 : public URMSensor
{
	public:
		void attach(byte trigPin, byte pwmPin)
		{
			URMSensor::attach(trigPin, pwmPin, URM37_US_PER_CM, URM37_TIMEOUT_FOR_PULSE_START,
				URM37_MAX_PULSE_WIDTH, URM37_TRIG_ACTIVE_STATE, URM37_ECHO_ACTIVE_STATE,
				URM37_TRIG_PULSE_WIDTH);
		}
};

class HC_SR04 : public URMSensor
{
	public:
		void attach(byte trigPin, byte echoPin)
		{
			URMSensor::attach(trigPin, echoPin, HC_SR04_US_PER_CM, HC_SR04_TIMEOUT_FOR_PULSE_START,
				HC_SR04_MAX_PULSE_WIDTH, HC_SR04_TRIG_ACTIVE_STATE, HC_SR04_ECHO_ACTIVE_STATE,
				HC_SR04_TRIG_PULSE_WIDTH);
		}
};

#endif