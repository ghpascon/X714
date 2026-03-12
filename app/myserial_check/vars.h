extern READER reader_module;

extern bool read_on;

extern TAG_COMMANDS tag_commands;

extern bool answer_rec;

extern ANTENA_COMMANDS antena_commands;

extern bool start_reading;
extern bool gpi_start;
extern int gpi_stop_delay;
extern bool always_send;

extern const byte min_power;
extern const byte max_power;
extern const byte min_rssi;
extern byte session;
extern byte max_session;

extern READER reader_module;

extern bool simple_send;

extern ANTENA antena[];
extern bool hotspot_on;
extern MySerial myserial;
extern bool keyboard;
extern bool buzzer_on;
extern bool decode_gtin;

// Prefix config (extern only)
extern String prefix;
