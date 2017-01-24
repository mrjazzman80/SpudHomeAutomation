/*
  Garage Door
  
  This deals with 
    + ethernet connections
    + MQTT
    + 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192,168,0,201);
IPAddress server(192,168,0,254);

// Ultrasonic Pins
const int TRIG_PIN = 7;
const int ECHO_PIN = 8;

// Anything over 400 cm (23200 us pulse) is "out of range"
const unsigned int MAX_DIST = 23200;

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

EthernetClient ethClient;
PubSubClient client(ethClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("house/garage/carstate","hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  Serial.begin(57600);

  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
  
  // The Trigger pin will tell the sensor to range find
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  //
  // Distance stuff....
  // 
  unsigned long t1;
  unsigned long t2;
  unsigned long pulse_width;
  float cm;
  float inches;
  // Hold the trigger pin high for at least 10 us
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10000);
  digitalWrite(TRIG_PIN, LOW);
  // Wait for pulse on echo pin  
  while (digitalRead(ECHO_PIN) == 0);
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
    client.publish("house/garage/doorstate","open");
    //client.publish("house/garage/carstate","open");
  } else if ( inches > 6 && inches < 50 ) {
    Serial.print(cm);
    Serial.print(" cm \t");
    Serial.println ( "Door Closed - Car in" );
    client.publish("house/garage/doorstate","closed");
    client.publish("house/garage/carstate","in");
  } else {
    Serial.print(cm);
    Serial.print(" cm \t");
    Serial.println("Door Closed - Car out" );
    client.publish("house/garage/doorstate","closed");
    client.publish("house/garage/carstate","out");
  }
  //client.publish("house/garage/carstate","hi");
  delay(5000);
}
