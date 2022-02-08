
void t_ventil::publish(void) {
  int state =  digitalRead(this->gpio);

  /*
    #ifdef MQTT_H
    String asd = "/fheidenr/relais/" + String(this->gpio) + "/state";
    const char* topic = asd.c_str();
    const char* payload = (state == HIGH) ? "1" : "0";

    mqtt.publish(topic, payload);

    String asd2 = "/fheidenr/relais/" + String(this->gpio) + "/dauer";
    const char* topic2 = asd2.c_str();
    char charValue[10];
    sprintf(charValue, "%d", this->dauer);

    const char* payload2 = &charValue[0];

    mqtt.publish(topic2, payload2);

    String asd3 = "/fheidenr/relais/" + String(this->gpio) + "/verbrauch";
    const char* topic3 = asd3.c_str();
    char charValue2[10];
    sprintf(charValue2, "%d", this->verbrauch);

    const char* payload3 = &charValue2[0];

    mqtt.publish(topic3, payload3);
    #endif
  */
}

void t_ventil::print(void) {
  Serial.println(String(this->gpio) + ": " + this->name + " " + String(this->dauer) + " " + String(this->verbrauch) + " | " + String(this->startTime) + " " + String(this->endTime));
}

String t_ventil::JSON_STATE(void) {
  int state =  digitalRead(this->gpio);
  return "{\"gpio\":" + String(this->gpio) + ",\"state\": " + ((state == 1) ? "true" : "false") + "}";
}


String t_ventil::JSON(void) {
  int state =  digitalRead(this->gpio);

  String ret = "{\"gpio\":";

  ret += String(this->gpio);
  ret += ",\"name\":\"";
  ret += this->name;
  ret += "\",\"dauer\":";
  ret += String(this->dauer);
  ret += ",\"verbrauch\":";
  ret += String(this->verbrauch);
  ret += ",\"state\":";
  ret += (state == 1) ? "true" : "false";//String(state);
  if (state == HIGH) {
    ret += ",\"since\":" + String(millis() - this->startTime);
    ret += ",\"progress\":" + String((millis() - this->startTime) / (this->endTime - this->startTime));
  }
  //ret += (state == HIGH ? ",\"since\":" + String(millis() - this->startTime) + ",\"progress\":" + String((millis() - this->startTime) / (this->endTime - this->startTime)) : "");
  ret += "}";

  return ret;
}
bool t_ventil::toggle(uint8_t state) {
  uint8_t t = digitalRead(this->gpio);

  if (t != state) {
    digitalWrite(this->gpio, state);
    this->startTime = millis();
    this->endTime = (state == HIGH) ? this->dauer * 60000 + this->startTime + 1 : UINT64_MAX;

    //String asd = getJSON("");
    String asd = "{\"gpio\":" + this->JSON_STATE() + "}";
    webSocket.broadcastTXT(asd);

    msg("Ventil " + String(this->name) + "(" + String(this->gpio) + (state == HIGH ? ") geöffnet" : ") geschlossen"));


    /*
      #ifdef MQTT_H
        asd = "/fheidenr/relais/" + String(this->gpio) + "/state";
        const char* topic = asd.c_str();
        const char* payload = (state == HIGH) ? "1" : "0";
        mqtt.publish(topic, payload);
      #endif
    */

    /*if (state == LOW) {
      char configfile[100];

      time_t lf = time(nullptr);
      int cx = snprintf ( configfile, 100, "%s.%i.da", lf / (60 * 60 * 24), this->gpio);

      File F = SPIFFS.open(configfile, "wb+");
      double a = 0;
      unsigned int readSize = F.readBytes(a, sizeof(double));
      if (readSize == 0) {

      }
      //writeSize = F.write((byte*) Cnf, sizeof(Cnf));

    }*/

    return true;
  } else {
    return false;
  }
}

bool t_kreislauf::toggle(uint8_t state) {
  if (state == HIGH) {
    // Hauptventil öffnen oder offen halten
    hauptventil.toggle(HIGH);
  }
  return t_ventil::toggle(state);
}


t_ventil::t_ventil(uint8_t gpio,
                   String  name,
                   uint8_t dauer,
                   uint8_t verbrauch) {
  this->gpio = gpio;
  this->name = name;
  this->dauer = dauer;
  this->verbrauch = verbrauch;
  this->endTime = UINT64_MAX;
}
