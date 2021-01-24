
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

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

#define NO_CHANGE -1
#define COLOR_VALID(x) (x >= 0 && x < 256)

struct ColorChannel {
  int r = 0;
  int g = 0;
  int b = 0;

  ColorChannel() {}
  
  void set(int r_new, int g_new, int b_new) {
    r = r_new;
    g = g_new;
    b = b_new;
  }
  
  void reset() {
    r = 0;
    g = 0;
    b = 0;
  }

} currentColors;


void set_channels(int r, int g, int b, bool animate = false) {

  int r_new = COLOR_VALID(r) ? r : currentColors.r;
  int g_new = COLOR_VALID(g) ? g : currentColors.g;
  int b_new = COLOR_VALID(b) ? b : currentColors.b;

  Serial.print("set_channels: ");
  Serial.print(r_new);
  Serial.print(", ");
  Serial.print(g_new);
  Serial.print(", ");
  Serial.print(b_new);
  Serial.print(" ");
  if (animate) {
    Serial.println("true");
  } else {
    Serial.println("false");
  }

  if (animate) {

  } else {

  }

  analogWrite(R1, r_new);
  analogWrite(R2, g_new);
  analogWrite(R3, b_new);

  analogWrite(L1, r_new);
  analogWrite(L2, g_new);
  analogWrite(L3, b_new);

  currentColors.set(r_new, g_new, b_new);
}

void all_channels_off() {
  analogWrite(R1, 0);
  analogWrite(R2, 0);
  analogWrite(R3, 0);

  analogWrite(L1, 0);
  analogWrite(L2, 0);
  analogWrite(L3, 0);

  currentColors.reset();
}

void wifi_connect() {
  WiFi.mode(WIFI_STA);
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
StaticJsonDocument<256> jsonBuffer;

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
  
  DeserializationError err = deserializeJson(jsonBuffer, msgString);
  if (err) {
    Serial.print("mqtt_callback: deserializeJson failed with code ");
    Serial.println(err.c_str());
    return;
  }

  int msgRed = NO_CHANGE;
  JsonVariant v = jsonBuffer["data"]["r"];
  if (!v.isNull()) {
    if (v >= 0 && v < 256) {
      msgRed = v;
    }
  }

  int msgGreen = NO_CHANGE;
  v = jsonBuffer["data"]["g"];
  if (!v.isNull()) {
    if (v >= 0 && v < 256) {
      msgGreen = v;
    }
  }

  int msgBlue = NO_CHANGE;
  v = jsonBuffer["data"]["b"];
  if (!v.isNull()) {
    if (v >= 0 && v < 256) {
      msgBlue = v;
    }
  }

  Serial.print("received r: ");
  Serial.print(msgRed);
  Serial.print(" g: ");
  Serial.print(msgGreen);
  Serial.print(" b: ");
  Serial.print(msgBlue);
  Serial.println("");

  set_channels(msgRed, msgGreen, msgBlue, false);
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
  all_channels_off();
}

void loop() {
  if (!client.connected())
  {
    mqtt_connect();
  }
  client.loop();

  delay(2000);
}
