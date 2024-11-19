#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

long currenTime, lasTime;

/*------------------------------ PUMPS ------------------------------*/
Relay airPump(25);
bool airPumpState = false;

Relay waterPump(33);
bool waterPumpState = false;

/*------------------------------ SENSORS ------------------------------*/
// temp
#define sensorTemp 26
OneWire oneWire(sensorTemp);
DallasTemperature waterTemp(&oneWire);
float temp = 0;

/*------------------------------ FUNCTIONS ------------------------------*/
void setupWifi() {
  delay(1000);
  Serial.print("Connecting to: ");
  Serial.println(SSID);

  WiFi.begin(SSID, PSWD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.print("\nConnected to: ");
  Serial.println(SSID);
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  if (!client.connected()) {
    while (!client.connected()) {
      Serial.print("Attempting MQTT connection...");

      if (client.connect(MQTT_CLIENT)) { // change the client ID to something unique
        Serial.print(" Connected to: ");
        Serial.print("test.mosquitto.org");
        client.subscribe(RX_TOPIC);
        Serial.print(" Subscribed to: ");
        Serial.println(RX_TOPIC);
      } 
      else {
        Serial.print(" failed, rc= ");
        Serial.print(client.state());
        Serial.println(" try again in 5 seconds");
        delay(5000);
      }
    }
  }

  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.print(topic);
  Serial.println("");

  String msgTmp;

  for (int i = 0; i < length; i++) {
    msgTmp += (char)payload[i];
    //Serial.print((char)payload[i]);
  }
  Serial.println(msgTmp);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, msgTmp);

  if (error) {
    Serial.print(F("Error parsing JSON: "));
    Serial.println(error.f_str());
    return;
  }

  airPumpState = doc["airPump"];
  waterPumpState = doc["waterPump"];
  Serial.println(airPumpState);
  Serial.println(waterPumpState);
}

void setup() {
  Serial.begin(115200);

  setupWifi();
  client.setServer("test.mosquitto.org", 1883);
  client.setCallback(callback);

  // PUMPS
  airPump.init();
  waterPump.init();

  // SENSOR
  waterTemp.begin();
}

void loop() {
  delay(10);
  reconnect();

  if (airPumpState) {
    airPump.on();
  }
  else {
    airPump.off();
  }

  if (waterPumpState) {
    waterPump.on();
  }
  else {
   waterPump.off();
  }
  
  waterTemp.requestTemperatures();
  temp = waterTemp.getTempCByIndex(0);
  /*
  Serial.print(waterTemp.getTempCByIndex(0));
  Serial.println(" C°");
  */

  currenTime = millis();
  if (currenTime - lasTime > 5000) {
    lasTime = currenTime;

    StaticJsonDocument<200> doc;
    doc["temp"] = temp;

    char buffer[200];
    serializeJson(doc, buffer);


    if (client.publish(TX_TOPIC, buffer)) {
      Serial.println("Publishing message...");
      Serial.println(buffer);
    }
    else 
      Serial.println("Failed to publish message.");
  }
}