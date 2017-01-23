/**
 * Garage Door Opener / 
 */

#include <EtherCard.h>

#define REQUEST_RATE 5000 // milliseconds

// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
static byte myip[] = { 192,168,0,210 };
// gateway ip address
static byte gwip[] = { 192,168,0,1 };

byte Ethernet::buffer[500];   // a very small tcp/ip buffer is enough here
static long timer;

// Pins
const int TRIG_PIN = 7;
const int ECHO_PIN = 8;

// Anything over 400 cm (23200 us pulse) is "out of range"
const unsigned int MAX_DIST = 23200;



void setup() {
  // The Trigger pin will tell the sensor to range find
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  // We'll use the serial monitor to view the sensor output
  Serial.begin(9600);

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");

  ether.staticSetup(myip, gwip);

  timer = - REQUEST_RATE; // start timing out right away
  
  Serial.println("Beginning Application");
}



void loop() {
  unsigned long t1;
  unsigned long t2;
  unsigned long pulse_width;
  float cm;
  float inches;
  ether.packetLoop(ether.packetReceive());
  // Hold the trigger pin high for at least 10 us
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10000);
  digitalWrite(TRIG_PIN, LOW);
  // Wait for pulse on echo pin
  while ( digitalRead(ECHO_PIN) == 0 );
  // Measure how long the echo pin was held high (pulse width)
  // Note: the micros() counter will overflow after ~70 min
  t1 = micros();
  while ( digitalRead(ECHO_PIN) == 1);
  t2 = micros();
  pulse_width = t2 - t1;
  // Calculate distance in centimeters and inches. The constants
  // are found in the datasheet, and calculated from the assumed speed 
  //of sound in air at sea level (~340 m/s).
  cm = pulse_width / 58.0;
  inches = pulse_width / 148.0;

  // Print out results
  if ( pulse_width > MAX_DIST ) {
    //Serial.println("Out of range");
  } else if ( inches < 6 ) {
    Serial.print(cm);
    Serial.print(" cm \t");    
    Serial.println( "Door Open");
  } else if ( inches > 6 && inches < 50 ) {
    Serial.print(cm);
    Serial.print(" cm \t");
    Serial.println ( "Door Closed - Car in" );
  } else {
    Serial.print(cm);
    Serial.print(" cm \t");
    Serial.println("Door Closed - Car out" );
  /*
    Serial.print(cm);f
    Serial.print(" cm \t");
    Serial.print(inches);
    Serial.println(" in");
   */
  }
  // Wait at least 60ms before next measurement
  delay(600);
}
