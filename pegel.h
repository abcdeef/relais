#include <INA219_WE.h>

#ifndef PEGEL_H
#define PEGEL_H
#endif

#define PEGELS 5
#define MIN_MV 4
#define MAX_MV 20

class t_pegel : public INA219_WE {
  public:
    bool read(void);
    String print(void);
    t_pegel(void);
    uint8_t min, max, cur;
    void write(void);
    bool status;
  private:
    uint8_t index;
    uint8_t raw[PEGELS];
};

t_pegel::t_pegel(void) : INA219_WE(0x40) {
  this->min = EEPROM.read(1);
  this->max = EEPROM.read(2);
  this->cur = 0;
}

void t_pegel::write(void) {
  EEPROM.write(1, this->min);
  EEPROM.write(2, this->max);
  if (!EEPROM.commit()) {
    msg("ERROR! EEPROM commit failed");
  }
}

int sort_pegels(const void *a, const void *b) {
  return ( *(uint8_t*)a - * (uint8_t*)b );
}
String t_pegel::print(void) {
  return String(((float)this->min ) / 10.0) + "/" + String(this->cur / 10.0) + "/" + String(((float)this->max) / 10.0);
}

bool t_pegel::read(void) {
  if (!this->status) {
    msg("INA219 not connected!");
    return false;
  }
  uint8_t a =  (uint8_t) (this->getCurrent_mA() * 10.0);
  this->raw[this->index++] = a;
  this->index %= PEGELS;

  uint8_t b[PEGELS];
  memcpy(b, this->raw, PEGELS);
  qsort(b, PEGELS, 1, sort_pegels);

  this->cur = b[(PEGELS - 1) / 2];
  bool flag_write = false;

  if (this->min > this->cur && this->cur > MIN_MV) {
    this->min = this->cur;
    flag_write = true;
  }
  if (this->max < this->cur && this->cur < MAX_MV) {
    this->max = this->cur;
    flag_write = true;
  }
  if (flag_write) {
    this->write();
  }
  return true;
}

t_pegel pegel = t_pegel();
