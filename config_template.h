// RENAME THIS FILE TO config.h
#ifndef CONFIG_H
#define CONFIG_H

// DO NOT CHANGE THIS INCLUDES 
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal_I2C.h>
#include "Relay.h"
#include "DS1307_RTC.h"
#include "MicroSD.h"
#include "TDS.h"

#define SSID "SSID"
#define PSWD "PSWD"

#define BROKER "test.mosquitto.org"
#define PORT 1883
#define RX_TOPIC "/RX_TOPIC"
#define TX_TOPIC "/TX_TOPIC"
#define MQTT_CLIENT "Client"

#define WIFI_TIME_OUT 10000
#define MSG_COOLDOWN 5000

#endif