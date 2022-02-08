#include "ccronexpr/ccronexpr.h"

void printCronExp(cron_expr *cron_ex);

class t_programm {
  public:
    uint8_t id;
    String name;
    unsigned long startTime = 0;
    uint32_t dauer = 0;
    t_kreislauf** ablauf;
    uint8_t ablauf_anzahl;
    bool aktiv = false;
    uint8_t p_i = 0;

    cron_expr cron_ex;
    bool cron = false;
    String cronString;
    time_t nextcron = 0;

    void init(void);
    void test(void);
    void next(void);
    void cancel(void);
    void cronsetup(const char* expression);
    t_programm(uint8_t id, String name,  t_kreislauf** ablauf, uint8_t n);
};
void t_programm::cronsetup(const char* expression) {
  msg(String(this->id) + ": \"" + expression + "\"");
  const char* err = NULL;
  cron_parse_expr(expression, &this->cron_ex, &err);
  this->cron = true;

  this->cronString = String(expression);

  this->nextcron = cron_next(&this->cron_ex, time(nullptr));
  msg("nextcron: " + String(this->nextcron));
  //printCronExp(&this->cron_ex);
}

int sort_ablauf(const void *a, const void *b) {
  msg( (*(t_kreislauf*)a).JSON() );
  return 0;//( *(uint8_t*)a - * (uint8_t*)b );
}
void t_programm::test() {
  qsort(&this->ablauf[0], this->ablauf_anzahl,  sizeof(&relayGPIOs[0]), sort_ablauf);
}
t_programm::t_programm(uint8_t id, String name, t_kreislauf **ablauf, uint8_t n) {
  this->id = id;
  this->name = name;
  this->ablauf = ablauf;
  this->ablauf_anzahl = n;
}

void t_programm::next() {
  this->p_i++;
  if (this->p_i < this->ablauf_anzahl) {
    this->ablauf[this->p_i]->toggle(HIGH);
    delay(1000);
  } else {
    msg("Programm " + String(this->name) + " beendet");
    this->cancel();
  }
}

void t_programm::cancel() {
  this->aktiv = false;
  this->ablauf[this->p_i]->toggle(LOW);

  String asd = "{\"programm\":[]}";
  webSocket.broadcastTXT(asd);
  msg(String(this->name) + " cancel() " + String(this->p_i ) + " " + this->ablauf[this->p_i]->name);

  aktivProgramm = 255;
  progress = 0;
  old_progress = 0;
  this->p_i = 0;
}

void t_programm::init() {
  this->startTime = millis();
  this->p_i = 0;

  this->dauer = 0;
  for ( uint8_t  i = 0 ; i < this->ablauf_anzahl; i++) {
    this->dauer += (*this->ablauf[i]).dauer;
  }
  this->dauer *= 60000;

  this->ablauf[0]->toggle(HIGH);
}

t_programm programme[] =
{ {100, "normal",  p_alles, sizeof(p_alles) / 4},
  {101, "Tröpfchen", p_tropf, sizeof(p_tropf) / 4},
  {102, "Entlüftung",  p_hecke, sizeof(p_hecke) / 4},
  {103, "Ansaat", p_ansaat, sizeof(p_ansaat) / 4}
};
