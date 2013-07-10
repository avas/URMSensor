#ifndef URMSENSOR_H
#define URMSENSOR_H

#include "Arduino.h"

/**
 * Marker value that is used to mark unassigned pin. It is used internally by the URMSensor class;
 * most probably you will not need it in your code.
 */
#define URM_INVALID_PIN 255

/**
 * Value that marks invalid result of the measure. Receiving this value as the measurement result
 * usually means that the URMSensor class either detected invalid state of the sensor pins on 
 * startMeasure() call, timed out waiting for sensor's response when starting measure, or
 * did not receive echo from sensor in time.
 */
#define URM_INVALID_RANGE 0xFFFFFFFF



/**
 * Constant used to convert pulse width from DFRobot URM37 sensor (in PWM mode) to range.
 */
#define URM37_US_PER_CM 50 // us/cm

/**
 * Constant used to convert pulse width from HC-SR04 sensor to range.
 */
#define HC_SR04_US_PER_CM 30 // us/cm



#define URM_TRIG_IDLE_STATE HIGH
#define URM_TRIG_ACTIVE_STATE LOW

#define URM_ECHO_IDLE_STATE HIGH
#define URM_ECHO_ACTIVE_STATE LOW



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
			_trigPin = URM_INVALID_PIN;
			_echoPin = URM_INVALID_PIN;
		}
		
		
		
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
			unsigned long maxPulseDuration)
		{
			_trigPin = trigPin;
			_echoPin = echoPin;
			
			_usPerCm = usPerCm;
			
			_timeoutForPulseStart = timeoutForPulseStart;
			_maxPulseDuration = maxPulseDuration;
		}
			
		/**
		 * Detaches the sensor from physical pins of the target Arduino board. The instance of 
		 * the URMSensor class will not be able to communicate with the sensor after calling 
		 * this method.
		 */
		void detach()
		{
			_trigPin = URM_INVALID_PIN;
			_echoPin = URM_INVALID_PIN;
		}
		
		/**
		 * Indicates whether this instance is attached to sensor.
		 *
		 * @return true if this instance is attached to sensor.
		 */
		boolean isAttached()
		{
			return (_trigPin != URM_INVALID_PIN) && (_echoPin != URM_INVALID_PIN);
		}
		
		
		
		/**
		 * Starts the measure.
		 */
		void startMeasure();
		
		/**
		 * Indicates whether this instance is currently doing the measure.
		 *
		 * @return true if this instance currently measures the distance.
		 */
		boolean isMeasuring();
		
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
		 * Indicates whether this instance finished the measure. Note that this method will NOT
		 * start a new measurement - it will just return the value from the previous measurement.
		 *
		 * @param distance Pointer to the variable to store the distance to. If this method returns false,
		 * the value of the pointed variable will remain unchanged.
		 *
		 * @result true if this instance is not currently measuring the distance.
		 */
		boolean finishedMeasure(unsigned long* distance);
		
		
		
		/**
		 * Synchronously gets the distance in front of the sensor. Maximal working time for 
		 * this method is (timeoutForPulseStart + maxPulseDuration) us.
		 *
		 * @return Distance in front of the sensor in centimeters.
		 */
		unsigned long getDistance();
		
	private:
		byte _trigPin;
		byte _echoPin;
		int _usPerCm;
		unsigned long _timeoutForPulseStart;
		unsigned long _maxPulseDuration;
		
		unsigned long _startMeasureTime;
		unsigned long _currentDuration;
		
		boolean _isMeasuring;
		boolean _waitingForPulse;
};

#endif