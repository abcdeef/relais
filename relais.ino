#include "credentials.h"
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#include <Wire.h>
#include <stdio.h>
#include <RTCx.h>

#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <time.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <EEPROM.h>

#define VERSION 1.93
#define EEPROM_VERSION 171
#define UINT64_MAX 0xFFFFFFFF

void msg(String message);
bool syslog = false;
uint8_t aktivProgramm = 255;
uint8_t progress = 0, old_progress = 0;

WebSocketsServer webSocket(81);

bool flag_ds1307 = false;

#include "pegel.h"
#include "kreislauf.h"
#include "programm.h"
#include "util.h"
#include "webserver.h"
#include "websocket.h"
//#include "mqtt.h"
#include "wifi.h"

#ifdef MQTT_H
#include <PubSubClient.h>
void callback(char* topic, byte* payload, unsigned int length);
BearSSL::WiFiClientSecure wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);
#endif
/*
  grün -RX
  weiss-TX
*/

/*
  Version 2.0

  Version 1.1
  - Compiledatum
  - "since" in Datenstruktur und json
  - erst neues Ventil öffnen dann vorheriges schliessen
  - WLAN-AP bei erfolgreicher Verbindung mit anderen AP schliessen
  - mit NTP-Zeitserver synchronisieren
  - Android: reconnect nach idle

  Versoin 1.2
  - einfaches CRON

  Version 1.22
  - Bugfix

  Version 1.3
  - rsyslog client für 192.168.1.101

  Version 1.31
  - aufräumen

  Version 1.32
  - automatisches Schliessen der Ventil nach definierter Zeit

  Version 1.33
  - Bugfix

  Version 1.4
  - Objektorientiert

  Version 1.41
  - Bugfix

  Version 1.5
  - WS_BINARY
  - relayGPIOs im EEPROM + Read/Set per Websocket

  Version 1.6
  - CronStrings für Programme im EEPROM + Read/Set per Websocket

  Version 1.61
  - Bugfix

  Version 1.7
  - Pegelmessung vorbereitet

  Version 1.8
  - OTA

  Version 1.8
  - Bugfix
  - Aufräumen

  Version 1.86
  - "since" in UI implemtieren
  - Behandlung von steckdose und hauptventil

  Version 1.9
  - Nebenläufigkeit der WiFi-Initialisierung entfernt
  - automatische Erkennung von INA29 und RTC

  Version 1.91
  - ESPTemplateProcessor.h entfernt

  Version 2.0
  - MQTT
*/

/*

  - anzahl geöffneter Ventile berücksichtigen
  - Sprengung planen (Startzeit wählen)
  - js: wenn WS nicht verbunden -> periodisch neu verbinden
*/

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  for (int i = 0; i < sizeof(relayGPIOs) / sizeof(t_kreislauf); i++) {
    pinMode(relayGPIOs[i].gpio, OUTPUT);
    delay(100);
    digitalWrite(relayGPIOs[i].gpio, LOW);
    delay(100);
  }

  pinMode(hauptventil.gpio, OUTPUT);
  delay(100);
  digitalWrite(hauptventil.gpio, LOW);
  delay(100);
  pinMode(steckdose.gpio, OUTPUT);
  delay(100);
  digitalWrite(steckdose.gpio, LOW);

  pinMode(1, FUNCTION_3);
  pinMode(3, FUNCTION_3);
  Wire.begin(1, 3);

  pegel.status = pegel.init();
  if (rtc.autoprobe()) {
    flag_ds1307 = true;
    rtc.enableBatteryBackup();
    rtc.startClock();
    rtc.setSQW(RTCx::freq4096Hz);
  }

  if ( !pegel.status  && !flag_ds1307 ) {
    pinMode(1, OUTPUT);
    pinMode(3, OUTPUT);
    Serial.begin(115200);
    delay(500);
    msg("Serial_out gestartet");
  }
  if (pegel.status) {
    msg("INA219 gefunden");
  }
  if (flag_ds1307) {
    msg("ds1307 gefunden");
  }

  if (!SPIFFS.begin()) {
    msg("An Error has occurred while mounting SPIFFS");
    return;
  }

  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    Serial.print(dir.fileName());
    if (dir.fileSize()) {
      Serial.print(" ");
      File f = dir.openFile("r");
      Serial.println(f.size());
    }
  }

  initWiFi();

  // EEPROM
  EEPROM.begin(512);
  int address = 0;
  byte value;
  value = EEPROM.read(address);
  if (value != EEPROM_VERSION) {
    msg("keine Daten im EEPROM. Schreibe Datei ins EEPROM");
    //pegel.min = 255;
    //pegel.max = 0;
    writeEEPROM();
  } else {
    msg("Daten im EEPROM gefunden");
    readEEPROM();
  }

  configTime("CET-1CEST,M3.5.0/02,M10.5.0/03", "pool.ntp.org");

  uint8_t r = 0;
  while ((time(nullptr) < 10000) && (r < 20)) {
    delay(1000);
    r++;
  }
#ifdef MQTT_H
  initMQTT();
#endif
}
void cron_handle() {
  time_t asd = time(nullptr);
  for (uint8_t n = 0; n < sizeof(programme) / sizeof(t_programm); n++) {

    if (programme[n].cron) {
      if (programme[n].nextcron < asd) {
        msg("Cron: " + String(programme[n].name) + "(" + String(n) + "): " + mytime(localtime(&programme[n].nextcron)));

        initProgram(programme[n].id);

        programme[n].nextcron = cron_next(&programme[n].cron_ex, asd + 60);
        //ws.textAll(getJSON(""));
      }
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  payload[length] = 0; // ensure valid content is zero terminated so can treat as c-string

  msg("Message arrived [" + String(topic) + "] : " + String((char *)payload));
}

unsigned long old_millis = 0;
void loop() {

  //delay(500);
#ifdef MQTT_H
  mqtt.loop();
#endif

  server.handleClient();
  webSocket.loop();
  ArduinoOTA.handle();


  time_t asd = time(nullptr);
  if (asd > 1000000000) {
    cron_handle();

    if (flag_ds1307) {
      struct RTCx::tm tm;
      rtc.readClock(tm);
      time_t df = rtc.mktime(&tm);

      if (abs(df - asd) > 2) {
        msg("rtc muss synchronisiert werden");
        struct RTCx::tm tm;
        RTCx::time_t t = asd;
        RTCx::gmtime_r(&t, &tm);
        rtc.setClock(&tm);

      }
    }
  }

  for (uint8_t n = 0; n < sizeof(relayGPIOs) / sizeof(t_kreislauf); n++) {
    if (relayGPIOs[n].endTime < millis()) {
      if (aktivProgramm != 255) {
        if ((*programme[aktivProgramm].ablauf[programme[aktivProgramm].p_i]).gpio == relayGPIOs[n].gpio) {
          programme[aktivProgramm].next();
        }
      }
      relayGPIOs[n].toggle(LOW);
    }
  }
  if (steckdose.endTime < millis()) {
    steckdose.toggle(LOW);
  }

  if (aktivProgramm != 255) {
    uint32_t tmp = millis() - programme[aktivProgramm].startTime;
    progress = (tmp * 100) / programme[aktivProgramm].dauer;

    if (progress != old_progress) {

      String asd = "{\"programm\":{\"id\":" + String(programme[aktivProgramm].id) + ",\"progress\":" +  String(progress) + "}}";
      webSocket.broadcastTXT(asd);

      old_progress = progress;
    }
  }

  uint8_t ol = 0;
  for (uint8_t n = 0; n < sizeof(relayGPIOs) / sizeof(t_kreislauf); n++) {
    ol += digitalRead(relayGPIOs[n].gpio);
  }
  if (ol == 0 ) {
    hauptventil.toggle(LOW);
  }

  if (millis() - old_millis > 10000 ) {
#ifdef MQTT_H
    if (!mqtt.connected()) {
      connectMQQT();
    }
#endif

    if (pegel.read()) {
      msg(pegel.print());
    }

    old_millis = millis();
  }
}
