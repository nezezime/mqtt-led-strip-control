
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "config.h" // create a file with the required constants

// GPIO pin mapping
#define L1 2
#define L2 0
#define L3 4

#define R1 12
#define R2 13
#define R3 14

WiFiClient wifiClient;
PubSubClient client(wifiClient);

void wifi_connect() {
  WiFi.begin(ssid, password);
  Serial.println("\n\n");
  Serial.print("Connecting to SSID: ");
  Serial.print(ssid);
  Serial.print(" ");

  //bool statusLed = true;
  while (WiFi.status() != WL_CONNECTED) 
  {
    //digitalWrite(LED_BUILTIN, statusLed);
    //statusLed = !statusLed;
    delay(500);
    Serial.print("*");
  }

  Serial.println("");
  Serial.print("WiFi connected, client IP:");
  Serial.println(WiFi.localIP());
}

void mqtt_connect() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");
    if (client.connect(mqtt_device_id, "", "")) { 
      Serial.println("connected");
      client.subscribe(mqtt_topic_downstream);
      return;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

char message_buff[256];
void mqtt_callback(char *led_control, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(led_control);
  Serial.print("] len: ");
  Serial.println(length);

  int i;
  for (i = 0; i < length; i++)
  {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  String msgString = String(message_buff);
  Serial.println(msgString);
  // TODO parse the received message
}

void all_channels_off() {
  analogWrite(R1, 0);
  analogWrite(R2, 0);
  analogWrite(R3, 0);

  analogWrite(L1, 0);
  analogWrite(L2, 0);
  analogWrite(L3, 0);
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(L1, OUTPUT);
  pinMode(L2, OUTPUT);
  pinMode(L3, OUTPUT);

  pinMode(R1, OUTPUT);
  pinMode(R2, OUTPUT);
  pinMode(R3, OUTPUT);

  delay(500);
  all_channels_off();
  digitalWrite(LED_BUILTIN, HIGH);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqtt_callback);

  wifi_connect();
  mqtt_connect();
}

void loop() {
  if (!client.connected())
  {
    mqtt_connect();
  }
  client.loop();

  analogWrite(R1, 10);
  analogWrite(R2, 10);
  analogWrite(R3, 10);

  analogWrite(L1, 10);
  analogWrite(L2, 10);
  analogWrite(L3, 10);

  delay(1000);
}
