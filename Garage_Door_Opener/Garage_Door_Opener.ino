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
const int TRIG_PIN = 8;
const int ECHO_PIN = 7;
const int RELAY_PIN = 2;

// Anything over 400 cm (23200 us pulse) is "out of range"
const unsigned int MAX_DIST = 23200;
char message_buff[80];
int curState = 0;

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
      // ... and resubscribe
      client.subscribe("house/garage/door/control");
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
  Serial.println("Begin");
  
  client.setServer(server, 1883);
  client.setCallback(callback);

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
  
  // The Trigger pin will tell the sensor to range find
  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(RELAY_PIN, OUTPUT);  
}

void loop()
{
  Serial.println("Loop in");
  if (!client.connected()) {
    reconnect();
  }
  Serial.println("Before Loop");
  client.loop();
  Serial.println("After Loop");
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
  Serial.print("Distance ");
  Serial.println(inches);
  if ( pulse_width > MAX_DIST ) {
    Serial.println("Out of range");
  } else if ( inches <= 6 ) {
    if ( curState != 1 ) { 
      client.publish("house/garage/doorstate","open");
      curState = 1;
    } // don't publish unlress required
  } else if ( inches > 6 && inches < 50 ) {
    if ( curState != 2 ) { 
      client.publish("house/garage/doorstate","closed");
      client.publish("house/garage/carstate","in");
      curState = 2;
    } // don't publish unless required
  } else {
    if ( curState != 3 ) { 
      client.publish("house/garage/doorstate","closed");
      client.publish("house/garage/carstate","out");
      curState = 3;
    } // don't publish unless required
  }
  // 100ms for response times..
  delay(100);
}



// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {
//  Serial.println("INLOOP");
  int i = 0;
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  String msgString = String(message_buff);
  
  if (msgString == "ON" || msgString == "OFF" ) { 
     // Place holder for later, this is where
     // sensor code + relay to enable sensor would go
     // for state.
    digitalWrite(RELAY_PIN, HIGH);  
    Serial.println ("ON");
    delay(100);
    digitalWrite(RELAY_PIN, LOW);  
    Serial.println ("OFF");
    
  };

//  Serial.println(msgString);
  //  client.publish("house/garage/door/control", message_buff);
  

} // Callback
