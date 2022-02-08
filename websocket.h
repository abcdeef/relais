void webSocketProcess( uint8_t * data, size_t len) {
  String asd;
  switch (data[0]) {
    case 1:
      // Cron-Job setzen
      for (uint8_t d = 0; d < sizeof(programme) / sizeof(t_programm); d++) {
        if (programme[d].id == data[1]) {
          char tmp[len];
          //Serial.print("|");
          for (uint8_t i = 2 ; i < len; i++) {
            //Serial.print(char(data[i]));
            tmp[i - 2] = char(data[i]);
          }
          //Serial.println("|");
          programme[d].cronsetup(tmp);
          //asd = getJSON("");
          //webSocket.broadcastTXT(asd);
          writeEEPROM();
          break;
        }
      }
      break;

    case 2:
      /* Cron-Job deaktiviern */
      for (uint8_t d = 0; d < sizeof(programme) / sizeof(t_programm); d++) {
        if (programme[d].id == data[1]) {
          programme[d].cron = false;
        }
      }
      break;
    case 5: {
        unsigned long mi = millis() / 1000;
        uint8_t minuten = ((uint8_t)floor(mi / 60)) % 60;
        uint8_t stunden = ((uint8_t)floor(mi / 3600)) % 24;
        uint8_t days = (uint8_t) floor(mi / 86400);

        time_t lf = time(nullptr);
        asd = "{" + getGPIOs() + ",\"system\":{\"localtime\":\"" + mytime(localtime(&lf)) + "\",";

        if (flag_ds1307) {
          struct RTCx::tm tm;
          rtc.readClock(tm);
          asd += "\"rtc\":\"";
          asd += mytime((struct tm*)&tm);
          asd += "\",";
        }
        if (pegel.status ) {
          asd += "\"PEGEL\":\"";
          asd += pegel.print();
          asd += "\",";
        }

        asd += "\"uptime\":\"" +  String(days) + " Tage " + String(stunden) + "h " + String(minuten)  + "Minuten\"}}";

        webSocket.broadcastTXT(asd);
        break;
      }
    case 6:
      asd = getPs();
      webSocket.broadcastTXT(asd);
      break;

    case 34:
      /* reset Pegel */
      //pegel.min = 255;
      //pegel.max = 0;
      //pegel.write();
      //asd = getJSON("");
      //webSocket.broadcastTXT(asd);

      break;

    case 37:
      {
        Dir dir = SPIFFS.openDir("/data/");
        asd = "{\"dir\": \"/data\", \"files\":[";
        uint8_t nsd = 0;

        while (dir.next()) {
          if  (nsd > 0) {
            asd += ",";
          }
          asd += "{\"name\":\"" + dir.fileName() + "\" ";

          if (dir.fileSize()) {
            asd += ", \"size\": ";

            File f = dir.openFile("r");
            asd += String(f.size());

          }
          asd += "}";
          nsd++;
        }

        asd += "]}";
        webSocket.broadcastTXT(asd);
      }
      break;
    case 55:
      /* Cron-Lauf überspringen */
      for (uint8_t d = 0; d < sizeof(programme) / sizeof(t_programm); d++) {
        if (programme[d].id == data[1]) {
          programme[d].nextcron = cron_next(&programme[d].cron_ex , programme[d].nextcron + 60);

          asd = "{" + getMSG() + "}";
          webSocket.broadcastTXT(asd);
          break;
        }
      }
      break;
    case 67:
      // C - cancel
      programme[aktivProgramm].cancel();
      //aktivProgramm = 255;
      //progress = 0;
      //old_progress = 0;

      for (uint8_t i = 0; i < sizeof(relayGPIOs) / sizeof(t_kreislauf); i++) {
        relayGPIOs[i].toggle(LOW);
      }
      steckdose.toggle(LOW);
      hauptventil.toggle(LOW);

      break;

    case 71:
      // G - gpio
      for (uint8_t i = 0; i < sizeof(relayGPIOs) / sizeof(t_kreislauf); i++) {
        if (relayGPIOs[i].gpio == data[1]) {
          relayGPIOs[i].toggle(data[2]);
          break;
        }
      }
      if (steckdose.gpio == data[1]) {
        steckdose.toggle(data[2]);
      }
      break;

    case 80:
      // P - programm

      initProgram(data[1]);


      asd =  "{\"programm\":{\"id\":" + String(programme[aktivProgramm].id) + ",\"progress\":" +  String(progress) + "}}";
      webSocket.broadcastTXT(asd);
      break;
    case 82:
      // R - Read einen gpio
      for (uint8_t n = 0; n < sizeof(relayGPIOs) / sizeof(t_kreislauf); n++) {
        if (relayGPIOs[n].gpio == data[1]) {
          asd = relayGPIOs[n].JSON();
          webSocket.broadcastTXT(asd);
          break;
        }
      }
      break;

    case 83:
      // S - Setzen der Dauer eines GPIO
      for (uint8_t n = 0; n < sizeof(relayGPIOs) / sizeof(t_kreislauf); n++) {
        if (relayGPIOs[n].gpio == data[1]) {
          relayGPIOs[n].dauer = data[2];
          writeEEPROM();
          break;
        }
      }
      if (steckdose.gpio == data[1]) {
        steckdose.dauer = data[2];
        writeEEPROM();
      }

      asd = "{" + getGPIOs() + "}";
      webSocket.broadcastTXT(asd);
      break;
    case 93:
      // S - Setzen des Verbrauch eines GPIO
      for (uint8_t n = 0; n < sizeof(relayGPIOs) / sizeof(t_kreislauf); n++) {
        if (relayGPIOs[n].gpio == data[1]) {
          relayGPIOs[n].verbrauch = data[2];
          writeEEPROM();
          break;
        }
      }
      asd = "{" + getGPIOs() + "}";
      webSocket.broadcastTXT(asd);
      break;
    case 99:
      msg("ESP restart");
      //ESP.restart();
      break;
    /*case 14:
      for (uint8_t d = 0; d < sizeof(programme) / sizeof(t_programm); d++) {
        if (programme[d].id == data[1]) {
          programme[d].cron = false;

          asd = getJSON("");
          webSocket.broadcastTXT(asd);

          break;
        }
      }
      break;*/

    case 255:
      asd = getINIT();
      webSocket.broadcastTXT(asd);
      break;
  }
}
void webSocketEvent(byte num, WStype_t type, uint8_t * data, size_t len) {
  switch (type) {
    case WStype_CONNECTED: {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("WebSocket client [ % u] connected from % d. % d. % d. % d\n", num, ip[0], ip[1], ip[2], ip[3]);
      } break;
    case WStype_DISCONNECTED:
      Serial.printf("WebSocket client [ % u] Disconnected!\n", num);
      break;
    case WStype_BIN:
      webSocketProcess(data, len);
  }
}
