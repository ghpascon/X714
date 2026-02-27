#define PRESET_VALUE 0xFFFF
#define POLYNOMIAL 0x8408
unsigned int crcValue;
byte crc1, crc2;

byte step = 0;
bool setup_done = false;

// variaveis externas
extern const int ant_qtd;
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
extern const int max_tags;
extern TAG tags[];

extern bool read_on;

extern MySerial myserial;
extern int current_tag;

// ==================== Protected Inventory ====================
extern bool protected_inventory_enabled;
extern String protected_inventory_password;
extern bool reader_in_protected_inventory;