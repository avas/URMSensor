/**
 * Description:
 * This code demonstrates how does URMSensor library work. It just measures the distance 
 * using the ultrasonic sensor and outputs it to serial terminal.
 *
 * Connections:
 *    Pin 9 (Arduino) -> TRIG (URM sensor)
 *    Pin 10 (Arduino) -> ECHO (URM sensor)
 *    +5V (Arduino) -> VCC (URM sensor)
 *    GND (Arduino) -> GND (URM sensor)
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
// Uncomment one of the following lines to select which sensor you will use.

// HC_SR04 sensor;
URM37 sensor;


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

void loop()
{
  digitalWrite(LED, HIGH);
  
  // Getting current distance...
  unsigned long distance = sensor.measureDistance();
  
  digitalWrite(LED, LOW);

  // ...printing result to the serial terminal...
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
  
  // ...and waiting a bit before the next iteration will start.
  delay(500);
}
