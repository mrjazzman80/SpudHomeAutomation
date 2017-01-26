/*
  AirCon Remote - Publish through MQTT
  
  This deals with 
    + ethernet connections
    + MQTT
    + 
*/

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <IRremote.h>

// Update these with values suitable for your network.
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xED };
IPAddress ip(192,168,0,202);
IPAddress server(192,168,0,254);
IRsend irsend;

// Variables
char message_buff[100];

/* 
  * void callback(char* topic, byte* payload, unsigned int length) {
  *  Serial.print("Message arrived [");
  *  Serial.print(topic);
  *  Serial.print("] ");
  *  for (int i=0;i<length;i++) {
  *    Serial.print((char)payload[i]);
  *  } 
  *  Serial.println();
  * }
  */

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
      client.publish("house/upstairs/aircon/response","Initialising");
      // ... and resubscribe
      client.subscribe("house/upstairs/aircon/request");
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
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // house/upstairs/aircon/response
  


  // IRsend irsend;
  
}




// handles message arrived on subscribed topic(s)
void callback(char* topic, byte* payload, unsigned int length) {
  IRsend irsend;
  int i = 0;
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  String msgString = String(message_buff);
  
  Serial.println(msgString);
  irsend.sendNEC(msgString.toInt(), 32);
  client.publish("house/upstairs/aircon/response", message_buff);

} // Callback

