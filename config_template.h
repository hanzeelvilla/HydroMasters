#ifndef CONFIG_H
#define CONFIG_H

// DO NOT CHANGE THIS INCLUDES 
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define SSID "SSID"
#define PSWD "PSWD"

#define BROKER "test.mosquitto.org"
#define PORT 1883
#define RX_TOPIC "/RX_TOPIC"
#define TX_TOPIC "/TX_TOPIC"

#define MQTT_CLIENT "Client"

#endif