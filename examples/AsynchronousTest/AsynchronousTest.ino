/**
 * Description:
 * This sketch demonstrates how to work with URMSensor library in asynchronous mode.
 * It polls the sensor's state and outputs it to the serial terminal. It also simulates
 * software PWM on built-in LED to demonstrate the power of async work.
 * 
 * Connections:
 *    Pin 9 (Arduino) -> TRIG (URM sensor)
 *    Pin 10 (Arduino) -> ECHO (URM sensor)
 *    +5V (Arduino) -> VCC (URM sensor)
 *    GND (Arduino) -> GND (URM sensor)
 *
 * Q: The code looks way too complicated! Do I really need to do all this stuff 
 *    to just measure the distance?
 * A: No, of course you don't. Look at another example, SimpleTest, for simplier way.
 *
 * Q: Why aren't you use the hardware PWM? It would be much easier to implement!
 * A: The PWM here is just the way to demonstrate async work of this library.
 *    You can run any tasks instead of it (as long as it does not take too much 
 *    time to run).
 *
 * Q: I found comments with the word CHANGEME. What does it mean?
 * A: It means that you can experiment with following line(s) of code and see 
 *    what will happen. Read comment marked with CHANGEME for further explanations.
 *
 * Q: I liked the software-PWM feature so much. Can it be used separately?
 * A: Yes, you can copy and paste function managePWM() into your sketch.
 *    Someday I will make separate library for that, so stay tuned.
 *
 * Have fun! :)
 */

#include <URMSensor.h>

// Serial port that will be used to output the information. You might want to change it
// if you're using Arduino Leonardo or Arduino Mega with some wireless transceiver.
#define TERMINAL Serial

// Pins used to communicate with the sensor.
#define URM_TRIG 9
#define URM_ECHO 10

// Built-in LED used to indicate the activity.
#define LED 13

// Instance of the class representing your ultrasonic sensor.
// ===================================================================================
// CHANGEME: Uncomment one of the following lines to select which sensor you will use.

// HC_SR04 sensor;
URM37 sensor;
// ===================================================================================


void setup()
{
  // Attaching the sensor to Arduino pins and initializing it.
  sensor.attach(URM_TRIG, URM_ECHO);
  
  // Initializing the LED to be able to blink.
  pinMode(LED, OUTPUT);
  
  // Initializing the serial port...
  TERMINAL.begin(9600);
  
  // ...and waiting for user to connect. This is only useful when running this sketch
  // on Arduino Leonardo or other Arduino boards with cut-off RESET-EN jumper. 
  // Other boards will reboot when user will open the Port Monitor window, and this step
  // will be skipped.
  while (!TERMINAL) blink();  
  
  TERMINAL.println("setup() is over, starting measures...");
}

void blink()
{
  digitalWrite(LED, HIGH);
  delay(5);
  digitalWrite(LED, LOW);
  delay(95);
}


// Last measurement time. Used in manageSensorStartup() function
// to start measurement only twice a second.
unsigned long lastMeasurementTime = 0;

// Flag value used to output measured distance only once per measurement.
// The function manageSensorStartup() sets this flag when starting the measure, 
// and the function handleMeasurementResults() unsets this flag when it detects
// that the measurement is finished.
boolean receivedDistanceIsActual = false;


// Last time the PWM rate was update at. It is used inside of updatePWMRate()
// function to not update PWM rate too often.
unsigned long lastPWMRateUpdateTime = 0;

// Thsi flag is used inside of updatePWMRate() function and shows whether we should
// increase or decrease PWM rate (to make LED fade in and out).
boolean increasingPWMRate = true;

// Current PWM rate - it sets the brightness of LED. It is being set inside of
// updatePWMRate() function, and it's value is used in managePWM() function.
byte currentPWMRate = 0;

// Current LED pin state. The function managePWM() uses it to run a bit faster.
byte currentPinState = LOW;

/**
 * This function starts distance measurement twice a second.
 */
void manageSensorStartup()
{
  // Calculating time since last measurement.
  unsigned long currentUptime = millis();
  unsigned long msSinceLastMeasurement = currentUptime - lastMeasurementTime;
  
  // If it is less than 500 ms, exiting.
  // ======================================================================
  // CHANGEME: Try replacing 500 with some other value and look at the LED.
  //           Normally it should behave exactly the same, without freezing
  //           or slowing down.
  if (msSinceLastMeasurement < 500) return;
  // ======================================================================
  
  // Otherwise, we should start the measure...
  sensor.startMeasure();
  
  // ...reset the time counter...
  lastMeasurementTime = currentUptime;
  
  // ...and set the flag for handleMeasurementResults(), so it will print received value
  // exactly once when the sensor will finish the measure.
  receivedDistanceIsActual = true;
}

/**
 * This function detects whether the sensor finished the measure or not
 * and prints received value to the serial port (TERMINAL).
 */
void handleMeasurementResults()
{
  // ========================================================================================
  // Next line is very important - it polls sensor's state and checks if it finished measure.
  // The method finishedMeasure() is the core method of the URMSensor class 
  // and it's descendants - it unleashes the true power of async work.
  // ========================================================================================
  if (sensor.finishedMeasure() && receivedDistanceIsActual)
  {
    // If received value is actual, we'll print it to the serial console.
    unsigned long distance = sensor.getMeasuredDistance();
    if (distance == URM_INVALID_VALUE)
    {
      TERMINAL.print("Failed to read distance, current state: "); 
      TERMINAL.println(sensor.getState());
    }
    else
    {  
      TERMINAL.print("Measured distance: ");
      TERMINAL.println(distance);
    }
    
    // Resetting the flag, so this function will not do anything until the next measurement starts.
    receivedDistanceIsActual = false;
  }
}

/** ==============================================================================================
 * Length of period for software-PWM, in microseconds. This value is used in managePWM() function.
 * CHANGEME: try replacing this value with any multiple of 256. The less this value will be,
 *           the more time-critical managePWM() function will become.
 */
#define PWM_PERIOD_US 1024
// ===============================================================================================

/**
 * This function implements some kind of software PWM to make LED flashy-flash.
 */
void managePWM()
{
  // Note: The code of this function can be replaced with the following line of code:
  //
  //  analogWrite(LED, currentPWMRate);
  //
  // ...but it has some drawbacks:
  // 1. It will not work for ATmega328-based Arduino boards (Uno, Duemilanove, etc.) - 
  //    the LED pin on those boards does not have the hardware-PWM ability.
  // 2. Hardware PWM is not as time-critical as software PWM. Since this example
  //    aims to show you the ability to do some kind-of-time-critical tasks while 
  //    working with sensor, software PWM is more useful here.
  
  int requiredPulseWidth = currentPWMRate * (PWM_PERIOD_US / 256);
  int currentTime = micros() % PWM_PERIOD_US;

  if ((currentTime >= requiredPulseWidth) && (currentPinState == HIGH))
  {
    currentPinState = LOW;
    digitalWrite(LED, LOW);
  }
  else if ((currentTime < requiredPulseWidth) && (currentPinState == LOW))
  {
    currentPinState = HIGH;
    digitalWrite(LED, HIGH);
  }
}

// ===================================================================================
// CHANGEME: try experimenting with these values (defaults are 2/0/255):
//           1. Set low LED fade delay (1/0/255). Will the LED blink smoothly?
//              Will it be affected if you lower the PWM_PERIOD_US?
//           2. Set high LED fade delay and lower the maximal brightness (100/0/50).
//              Will the LED still fade in and out smoothly? Will it be affected
//              by low PWM_PERIOD_US? If you're running this code on Leonardo or Mega,
//              you can also replace managePWM() with analogWrite(LED, currentPWMRate)
//              and compare results.

/**
 * This value sets how fast the LED will blink (basically, it sets the delay between
 * PWM rate changes, in milliseconds). The higher the value, the slower the LED will blink.
 */
#define LED_FADE_DELAY 2

/**
 * This value defines the minimal brightness of LED.
 */
#define MIN_PWM_RATE 0

/**
 * This value defines the maximal brightness of LED.
 */
#define MAX_PWM_RATE 255
// ===================================================================================

/**
 * This function changes PWM rate for LED to make it slowly fade in and out.
 */
void updatePWMRate()
{
  unsigned long currentTime = millis();
  if (currentTime - lastPWMRateUpdateTime < LED_FADE_DELAY) return;
  
  if (increasingPWMRate)
  {
    currentPWMRate++;
    if (currentPWMRate == MAX_PWM_RATE) increasingPWMRate = false;
  }
  else
  {
    currentPWMRate--;
    if (currentPWMRate == MIN_PWM_RATE) increasingPWMRate = true;
  }
  
  lastPWMRateUpdateTime = currentTime;
}

void loop()
{
  manageSensorStartup();
  handleMeasurementResults();

  // ========================================================================================
  // CHANGEME: try commenting these lines and replacing it with your own code (just be sure
  //           that it does not take too long to run). You can place here delayMicroseconds()
  //           with different values and see how it affects the accuracy of measurements.
  managePWM();
  updatePWMRate();
  // ========================================================================================
}
