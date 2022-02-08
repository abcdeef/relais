#include <ESP8266HTTPUpdateServer.h>

void initServer(void);

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

String processor(const String& var) {
  if (var == "SYSTEM") {
    time_t asd = time(nullptr);

    String buttons = "<table><tr><td>localtime:</td><td>";
    buttons += mytime(localtime(&asd));
    buttons += "</td></tr><tr><td>uptime:</td><td>";
    unsigned long mi = millis() / 1000;

    uint8_t minuten = ((uint8_t)floor(mi / 60)) % 60;

    uint8_t stunden = ((uint8_t)floor(mi / 3600)) % 24;

    uint8_t days = (uint8_t) floor(mi / 86400);


    buttons += String(days) + " Tage " + String(stunden) + "h " + String(minuten)  + "Minuten";
    //buttons += String(mi);

    buttons += "</td></tr>";

    /*if (flag_ds1307) {
      buttons += "<tr><td>RTC:</td><td>";
      if (flag_ds1307) {
        struct RTCx::tm tm;
        rtc.readClock(tm);
        buttons += mytime((struct tm*)&tm);
      } else {
        buttons += "not connected";
      }
      buttons += "</td></tr>";
      }*/

    /*if (pegel.status) {
      buttons += "<tr><td>Current[mA]:</td><td>";
      if (pegel.status) {
        buttons += String(pegel.print());
      } else {
        buttons += "not connected";
      }
      buttons += "</td></tr>";
      }*/

    buttons += "</table>";

    return buttons;
  }
  return String();
}

void initServer() {
  server.serveStatic("/style.css", SPIFFS, "/style.css");
  server.serveStatic("/system.css", SPIFFS, "/system.css");
  server.serveStatic("/meins.js", SPIFFS, "/meins.js");
  server.serveStatic("/index.js", SPIFFS, "/index.js");
  server.serveStatic("/system.js", SPIFFS, "/system.js");
  server.serveStatic("/favicon.ico", SPIFFS, "/favicon.ico");
  server.serveStatic("/cross.png", SPIFFS, "/cross.png");


  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/system", SPIFFS, "/system.html");

  httpUpdater.setup(&server);
  server.begin();
}
