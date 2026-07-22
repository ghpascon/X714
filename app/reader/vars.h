#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL 0x8408
unsigned int crcValue;
byte crc1, crc2;

byte step = 0;
bool setup_done = false;

// variaveis externas
extern ANTENA antena[];
extern ANTENA_COMMANDS antena_commands;
extern bool read_on;
extern byte temperatura;
extern byte session;

// evita erro no recebimento serial

bool answer_rec = true;

// config
extern CONFIG_FILE config_file_commands;

// tags
extern TAG_COMMANDS tag_commands;

extern bool read_on;

extern MySerial myserial;

// ==================== Protected Inventory ====================
extern bool protected_inventory_enabled;
extern String protected_inventory_password;
extern bool reader_in_protected_inventory;

bool one_ant = true;
bool request_clear_serial_buffers = false;
bool had_valid_frame = false; // set once setup completes at 115200; prevents unnecessary baud change
byte reconnect_count = 0;     // counts reconnect attempts when had_valid_frame; reset on successful setup

extern const bool debug_mode;
