// ==================== Bibliotecas Arduino/ESP32 ====================
#include <Arduino.h>
#include "esp_system.h"
#include "esp_task_wdt.h"

#include <LittleFS.h>

// ==================== app ====================
#include "app/connection/_main.h"
#include "app/myserial/_main.h"
#include "app/pins/_main.h"
#include "app/antena/_main.h"
#include "app/antena/antena_commands.h"
#include "app/tag/_main.h"
#include "app/tag/tag_commands.h"
#include "app/config_file/_main.h"
#include "app/reader/_main.h"
#include "app/myserial_check/_main.h"
#include "app/web_server/_main.h"
#include "app/rgb/_main.h"
#include "app/webhook/_main.h"
