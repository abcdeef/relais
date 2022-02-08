WiFiEventHandler gotIpEventHandler, disconnectedEventHandler, stationConnectedHandler;

#define HOSTNAME "relais"

void onStationModeGotIP(const WiFiEventStationModeGotIP & event) {
  digitalWrite(LED_BUILTIN, HIGH);

  WiFi.softAPdisconnect(true);
  udp.begin(localPort);
  syslog = true;
  msg("ESP gestartet ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname(HOSTNAME);
  //ArduinoOTA.setPassword(OTA_PW);
  ArduinoOTA.begin();

  //initMQTT();
}
void onSoftAPModeStationConnected(const WiFiEventSoftAPModeStationConnected & event) {
  digitalWrite(LED_BUILTIN, HIGH);
}
void onStationModeDisconnected(const WiFiEventStationModeDisconnected & event) {
  digitalWrite(LED_BUILTIN, LOW);
  WiFi.begin(ssid, password);
}
void initWiFi() {
  WiFi.persistent(false);
  WiFi.mode(WIFI_AP_STA);

  stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onSoftAPModeStationConnected);
  gotIpEventHandler = WiFi.onStationModeGotIP(&onStationModeGotIP);
  disconnectedEventHandler = WiFi.onStationModeDisconnected(&onStationModeDisconnected);


  WiFi.begin(ssid, password);
 
  initServer();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}
