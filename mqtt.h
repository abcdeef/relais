#ifndef MQTT_H
#define MQTT_H

void initMQTT(void) {
  BearSSL::X509List *rootCert;
  BearSSL::X509List *clientCert;
  BearSSL::PrivateKey *clientKey;
  char *ca_cert = nullptr;
  File ca = SPIFFS.open(CA_CERT_FILE, "r");
  if (!ca) {
    Serial.println("Couldn't load CA cert");
  } else {
    size_t certSize = ca.size();
    ca_cert = (char *)malloc(certSize);
    if (certSize != ca.readBytes(ca_cert, certSize)) {
      Serial.println("Loading CA cert failed");
    } else {
      Serial.println("Loaded CA cert");
      rootCert = new BearSSL::X509List(ca_cert);
      wifiClient.setTrustAnchors(rootCert);
    }
    free(ca_cert);
    ca.close();
  }

  wifiClient.setClientRSACert(clientCert, clientKey);
}

void connectMQQT(void) {
  while (! mqtt.connected()) {
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) { // Token Authentication
      //    if (mqtt.connect(MQTT_DEVICEID)) { // No Token Authentication
      msg("MQTT Connected");
      for (int i = 0; i < sizeof(relayGPIOs) / sizeof(t_kreislauf); i++) {
        relayGPIOs[i].publish();
      }
      hauptventil.publish();
      steckdose.publish();
    } else {
      msg("MQTT Failed to connect! ... retrying");
      delay(500);
    }
  }
}
