void printKreislaufState(void);
String getJSON(String extra);
void initProgram(void);

const char compile_date[] = __DATE__ " " __TIME__;

int sort_asc(const void *cmp1, const void *cmp2) {
  time_t a = programme[*((uint8_t *)cmp1)].nextcron;
  time_t b = programme[*((uint8_t *)cmp2)].nextcron;

  return a < b ? -1 : (a > b ? 1 : 0);
}
char* mytime(const struct tm *timeptr) {
  static const char wday_name[][4] = {
    "So.", "Mo.", "Di.", "Mi.", "Do.", "Fr.", "Sa."
  };
  static const char mon_name[][4] = {
    "Jan", "Feb", "Mar", "Apr", "Mai", "Jun",
    "Jul", "Aug", "Sep", "Okt", "Nov", "Dez"
  };
  static char result[26];
  sprintf(result, "%.3s %3d %.3s %.2d:%.2d",
          wday_name[timeptr->tm_wday],
          timeptr->tm_mday,
          mon_name[timeptr->tm_mon],
          timeptr->tm_hour,
          timeptr->tm_min);
  return result;
}
String getMSG() {
  uint8_t m = sizeof(programme) / sizeof(t_programm);

  uint8_t sort[m], i = 0;
  for (uint8_t n = 0; n < m; n++) {
    if (programme[n].cron) {
      sort[i] = n;
      i++;
    }
  }
  if (i == 0)
    return "";

  qsort(sort, i, sizeof(sort[0]), sort_asc);

  String buttons = "\"msg\":[";
  for (uint8_t n = 0; n < i; n++) {
    if (n > 0)
      buttons += ",";
    buttons += "{\"id\":" + String(programme[sort[n]].id) + ",\"name\":\"" + String(programme[sort[n]].name) + "\",\"time\":\"" + mytime(localtime(&programme[sort[n]].nextcron));
    buttons +=  "\"}";
  }
  buttons += "]";

  /*if (pegel.status) {
    buttons +=  ",\"pegel\":{\"min\":" + String(pegel.min) + ",\"cur\":" + String(pegel.cur) + ",\"max\":" + String(pegel.max) ;
    buttons += "}";
    }*/
  return buttons;
}
String getPs() {
  String buttons = "\"programme\":[";
  for (uint8_t n = 0; n < sizeof(programme) / sizeof(t_programm); n++) {
    if (n > 0) {
      buttons += ",";
    }
    buttons += "{\"id\":" + String(programme[n].id) + ",\"name\":\"" \
               + programme[n].name + "\"";
    if (programme[n].cron) {
      buttons += ",\"nextcron\":\"" + String(mytime(localtime(&programme[n].nextcron))) + "\"";
    }
    buttons += "}";
  }
  buttons += "]";
  return buttons;
}
String getGPIOs() {
  int state;
  String buttons = "\"gpios\":[";
  for (uint8_t n = 0; n < sizeof(relayGPIOs) / sizeof(t_kreislauf); n++) {
    buttons += relayGPIOs[n].JSON() + ",";
  }
  buttons +=  steckdose.JSON();
  buttons +=  ",";
  buttons += hauptventil.JSON();
  buttons += "]";

  return buttons;
}
String getINIT() {
  String buttons = "{\"VERSION\": \"" + String(VERSION) + "\",";

  buttons += getGPIOs() + "," + getPs();

  String msg = getMSG();
  if (msg.length() > 0 ) {
    buttons += "," + msg;
  }
  if (aktivProgramm != 255 ) {
    buttons += ",\"programm\":{\"id\":" + String(programme[aktivProgramm].id) + ",\"progress\":" +  String(progress) + "}";
  }
  buttons += "}";
  return buttons;
}

void initProgram(uint8_t tmp) {
  if (aktivProgramm != 255) {
    programme[aktivProgramm].cancel();
  }
  for (uint8_t n = 0; n < sizeof(programme) / sizeof(t_programm); n++) {
    if (programme[n].id == tmp) {
      aktivProgramm = n;
      break;
    }
  }

  msg("Programm " + String(programme[aktivProgramm].name) + " gestartet");
  programme[aktivProgramm].init();
}

void writeEEPROM() {
  msg("writeEEPROM");
  int address = 0;
  byte value;

  EEPROM.write(address, EEPROM_VERSION);
  /*if (pegel.status) {
    EEPROM.write(++address, pegel.min);
    EEPROM.write(++address, pegel.max);
    } else {*/
  address += 2;
  //}

  EEPROM.write(++address, sizeof(relayGPIOs) / sizeof(t_kreislauf));

  for (uint8_t n = 0; n < sizeof(relayGPIOs) / sizeof(t_kreislauf); n++) {
    EEPROM.write(++address, relayGPIOs[n].gpio);
    EEPROM.write(++address, relayGPIOs[n].dauer);
    EEPROM.write(++address, relayGPIOs[n].verbrauch);
  }
  EEPROM.write(++address, steckdose.gpio);
  EEPROM.write(++address, steckdose.dauer);

  byte cpro = 0;
  for (uint8_t n = 0; n < sizeof(programme) / sizeof(t_programm); n++) {
    if (programme[n].cron) {
      cpro++;
    }
  }
  EEPROM.write(++address, cpro);
  for (uint8_t n = 0; n < sizeof(programme) / sizeof(t_programm); n++) {
    if (programme[n].cron) {
      Serial.println("Cron-Programme: " + programme[n].name);
      EEPROM.write(++address, programme[n].id);
      for (uint8_t i = 0; i < 8; i++) {
        EEPROM.write(++address, programme[n].cron_ex.seconds[i]);
      }
      for (uint8_t i = 0; i < 8; i++) {
        EEPROM.write(++address, programme[n].cron_ex.minutes[i]);
      }
      for (uint8_t i = 0; i < 3; i++) {
        EEPROM.write(++address, programme[n].cron_ex.hours[i]);
      }
      EEPROM.write(++address, programme[n].cron_ex.days_of_week[0]);
      for (uint8_t i = 0; i < 4; i++) {
        EEPROM.write(++address, programme[n].cron_ex.days_of_month[i]);
      }
      EEPROM.write(++address, programme[n].cron_ex.months[0]);
      EEPROM.write(++address, programme[n].cron_ex.months[1]);
    }
  }

  if (!EEPROM.commit()) {
    msg("ERROR! EEPROM commit failed");
  }
}
void readEEPROM() {
  int address = 0;
  byte i = EEPROM.read(address);
  if (i != EEPROM_VERSION) {
    msg("Falsche EEPROM-Version.");
    return;
  }
  //pegel.min = EEPROM.read(++address);
  ++address;
  //pegel.max = EEPROM.read(++address);
  ++address;

  i = EEPROM.read(++address);
  for (byte n = 0; n < i; n++) {
    relayGPIOs[n].gpio = EEPROM.read(++address);
    relayGPIOs[n].dauer = EEPROM.read(++address);
    relayGPIOs[n].verbrauch = EEPROM.read(++address);
  }
  steckdose.gpio = EEPROM.read(++address);
  steckdose.dauer = EEPROM.read(++address);

  i = EEPROM.read(++address);
  for (byte n = 0; n < i; n++) {
    byte id = EEPROM.read(++address);
    for (uint8_t d = 0; d < sizeof(programme) / sizeof(t_programm); d++) {
      if (programme[d].id == id)  {
        for (uint8_t w = 0; w < 8; w++) {
          programme[d].cron_ex.seconds[w] = EEPROM.read(++address);
        }
        for (uint8_t w = 0; w < 8; w++) {
          programme[d].cron_ex.minutes[w] = EEPROM.read(++address);
        }
        for (uint8_t w = 0; w < 3; w++) {
          programme[d].cron_ex.hours[w] = EEPROM.read(++address);
        }
        programme[d].cron_ex.days_of_week[0] = EEPROM.read(++address);
        for (uint8_t w = 0; w < 4; w++) {
          programme[d].cron_ex.days_of_month[w] = EEPROM.read(++address);
        }
        programme[d].cron_ex.months[0] = EEPROM.read(++address);
        programme[d].cron_ex.months[1] = EEPROM.read(++address);

        programme[d].cron = true;
        programme[d].nextcron = cron_next(&programme[d].cron_ex, time(nullptr));

        break;
      }
    }
  }
}

WiFiUDP udp;
unsigned int localPort = 2390;
IPAddress syslogServer(192, 168, 1, 101);

void msg(String message) {
  Serial.println(message);

  if (syslog) {

    unsigned int msg_length = message.length();
    byte* p = (byte*)malloc(msg_length);
    memcpy(p, (char*) message.c_str(), msg_length);

    udp.beginPacket(syslogServer, 514);
    udp.write(": ");
    udp.write(p, msg_length);
    udp.endPacket();
    free(p);
  }
}
