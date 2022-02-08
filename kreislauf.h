/*
  gr. Fläche  1-----3  Garten
       Vorne  2-----4  Hinten
  Entlüftung   -----5 Hecke

 7 Steckdose    6 Hauptventil

________
|    [1]|  7
|    [2]|  6
|    [3]| 
|    [4]|  5
|    [5]|  4
|    [6]|  1
|    [7]|  2
|    [8]|  3
|_______|

*/

struct STA {
  
  int weight;
  double price;
  
};

class t_ventil {
  public:
    uint8_t gpio;
    String  name;
    uint8_t dauer;
    uint8_t verbrauch; /* in dm³ */
    uint64_t endTime;
    uint64_t startTime;
    t_ventil(uint8_t gpio,
             String  name,
             uint8_t dauer,
             uint8_t verbrauch);
    bool toggle(uint8_t state);
    String JSON(void);
    String JSON_STATE(void);
    void print(void);
    void publish(void);
};

class t_kreislauf : public t_ventil {
  public:
    t_kreislauf(uint8_t gpio,
                String  name,
                uint8_t dauer,
                uint8_t verbrauch);
    bool toggle(uint8_t state);
};

t_ventil hauptventil(4, "Hauptventil", 0, 0);



t_kreislauf::t_kreislauf(uint8_t gpio,
                         String  name,
                         uint8_t dauer,
                         uint8_t verbrauch) : t_ventil( gpio,
                               name,
                               dauer,
                               verbrauch)  {

}

t_kreislauf relayGPIOs[] =
{ {14, "Vorne", 7, 5},
  {16, "Garten", 60, 0, },
  {12, "gr. Fl&auml;che", 5, 12},
  {15, "Hecke", 60, 0},
  {13, "Hinten", 5, 4}
};
t_ventil steckdose = t_kreislauf(5, "Steckdose", 60, 0);

t_kreislauf *p_alles[] = {&relayGPIOs[0], &relayGPIOs[2], &relayGPIOs[4]};
t_kreislauf *p_tropf[] = {&relayGPIOs[1], &relayGPIOs[3]};
t_kreislauf *p_hecke[] = {&relayGPIOs[3]};
t_kreislauf *p_ansaat[] = {&relayGPIOs[2], &relayGPIOs[4]};
