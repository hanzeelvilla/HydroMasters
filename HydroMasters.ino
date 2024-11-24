#include "config.h"

WiFiClient espClient;
PubSubClient client(espClient);

long currenTime, lasTime;

/*------------------------------ PUMPS ------------------------------*/
Relay airPump(32);
bool airPumpState = false;

Relay waterPump(14);
bool waterPumpState = false;

/*------------------------------ SENSORS ------------------------------*/
// temp
#define sensorTemp 25
OneWire oneWire(sensorTemp);
DallasTemperature waterTemp(&oneWire);
float temp = 0;

// LLS
#define LLS 33

// LCD
LiquidCrystal_I2C lcd (0x27, 16, 2);

// RTC
DS1307_RTC rtc;

// SD 
MicroSD MSD;

/*------------------------------ FUNCTIONS ------------------------------*/
void reconnectWifi() {
  unsigned long startWifi = millis();

  Serial.print("Connecting to: ");
  Serial.println(SSID);

  WiFi.begin(SSID, PSWD);

  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startWifi > WIFI_TIME_OUT) {
      Serial.println("\nWiFi connection timeout. Proceeding without connection...");
      return;
    }
    delay(500);
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
}

void setup() {
  Serial.begin(115200);

  reconnectWifi();
  client.setServer("test.mosquitto.org", 1883);
  client.setCallback(callback);

  // PUMPS
  airPump.init();
  waterPump.init();

  // SENSOR
  waterTemp.begin();
  pinMode(LLS, INPUT);
  rtc.init();
  lcd.init();
  MSD.init();
  lcd.backlight();
}

void loop() {
  /*------------------------------ SENSORS READING ------------------------------*/
  waterTemp.requestTemperatures();
  temp = waterTemp.getTempCByIndex(0);
  /*
  Serial.print(waterTemp.getTempCByIndex(0));
  Serial.println(" C°");
  */

  int val_LLS = digitalRead(LLS);
  //Serial.println(val_LLS);

  // show date and time in the LCD
  lcd.clear();
    
  String textDate = rtc.formated_date();
  String textTime = rtc.formated_time();

  lcd.setCursor(0, 0);
  lcd.print(textDate);

  lcd.setCursor(0, 1);
  lcd.print(textTime);

  /*------------------------------ CHANGE PUMP STATES ------------------------------*/
    if (airPumpState)
    airPump.on();
    else
      airPump.off();

    if (waterPumpState)
      waterPump.on();
    else
    waterPump.off();

    if (val_LLS)
      waterPumpState = false;

  // if the esp has internet
  if (WiFi.status() == WL_CONNECTED) {
    delay(10);
    reconnect();

    /*------------------------------ PUBLISH TO TX TOPIC ------------------------------*/
    currenTime = millis();
    if (currenTime - lasTime > MSG_COOLDOWN) {
      lasTime = currenTime;

      StaticJsonDocument<200> doc;
      doc["temp"] = temp;
      doc["airPump"] = airPumpState;
      doc["waterPump"] = waterPumpState;
      doc["date"] = textDate;
      doc["time"] = textTime;

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

  // if the esp doesn't has internet
  else {
    // to not waste energy
    waterPumpState = false;
    airPumpState = false;
    airPump.off();
    waterPump.off();
    Serial.println("WiFi disconnected");

    /*------------------------------ SAVING DATA IN SD CARD ------------------------------*/
    currenTime = millis();
    if (currenTime - lasTime > MSG_COOLDOWN) {
      lasTime = currenTime;

      StaticJsonDocument<200> doc;
      doc["temp"] = temp;
      doc["airPump"] = airPumpState;
      doc["waterPump"] = waterPumpState;
      doc["date"] = textDate;
      doc["time"] = textTime;

      char buffer[200];
      serializeJson(doc, buffer);

      Serial.println("Saving data in SD card");
      String fullTime = rtc.formated_fullDate('_', '_', '_');
      //Serial.println(fullTime);
      String fileName = "/" + fullTime + ".json";

      Serial.println(buffer);
      MSD.saveJson(fileName, doc);
    }
    reconnectWifi();

    delay(1000);
  }
}