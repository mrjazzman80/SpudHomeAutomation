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
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xEF };
IPAddress ip(192,168,0,203);
IPAddress server(192,168,0,254);
IRsend irsend;

// Variables
char message_buff[80];


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
  Serial.println("Starting application");

  Ethernet.begin(mac, ip);
  // Allow the hardware to sort itself out
  delay(1500);
  
  Serial.println("Network initialised");
  
  // Now that we have a network, try the MSQTT stuff
  client.setServer(server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  // house/upstairs/aircon/response
  
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

