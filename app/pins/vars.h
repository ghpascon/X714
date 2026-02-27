bool in_1 = false;
bool in_2 = false;
bool in_3 = false;

extern bool gpi_start;
extern bool read_on;

extern int gpi_stop_delay;

extern const int ant_qtd;

const int gpo_qtd = 3;
const int gpo_pin[gpo_qtd] = {out_1_pin, out_2_pin, out_3_pin};
bool gpo[gpo_qtd] = {false, false, false};

extern MySerial myserial;
extern bool buzzer_on;