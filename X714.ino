#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#define ant_qtd 4

#include "version.h"
#include "pins.h"
#include "helpers.h"
#include <freertos/semphr.h>
#include "libs.h"
#include "vars.h"

static TaskHandle_t core0TaskHandle = NULL;
static const uint32_t CORE0_TASK_STACK_SIZE = 12288; // bytes
static const size_t STACK_WARN_WATERMARK_BYTES = 1024;

static void warn_low_stack(const char *task_name, TaskHandle_t handle)
{
    const UBaseType_t free_words = uxTaskGetStackHighWaterMark(handle);
    const size_t free_bytes = (size_t)free_words * sizeof(StackType_t);
    if (free_bytes < STACK_WARN_WATERMARK_BYTES)
    {
        Serial.printf("[stack] WARN %s low stack: %u bytes free\n", task_name, (unsigned int)free_bytes);
    }
}

// ==================== Core 0 Task (RGB + Pins) ====================
void core0Task(void *pvParameters)
{
    // Configure watchdog for Core 0
    esp_task_wdt_add(NULL);

    while (true)
    {
        // Reset watchdog for Core 0
        esp_task_wdt_reset();

        // RGB state management
        rgb.state();

        // Check inputs
        pins.check_inputs();

        // Update outputs
        pins.set_outputs();

        // Save configuration
        config_file_commands.save_config();

        static unsigned long last_stack_check = 0;
        if (millis() - last_stack_check > 5000)
        {
            last_stack_check = millis();
            warn_low_stack("Core0Task", NULL);
        }

        // Small delay to prevent task from starving other processes
        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay
    }
}

// ==================== Setup ====================
void setup()
{
    Serial.begin(115200);
    // Initialize the file system (tenta montar; se falhar, tenta formatar)
    Serial.println("[fs] Montando LittleFS...");
    if (LittleFS.begin(false))
    {
        Serial.println("[fs] LittleFS montado com sucesso");
        fs_loaded = true;
    }
    else
    {
        Serial.println("[fs] Falha ao montar LittleFS, tentando formatar e montar...");
        if (LittleFS.begin(true))
        {
            Serial.println("[fs] LittleFS formatado e montado com sucesso");
            fs_loaded = true;
        }
        else
        {
            Serial.println("[fs] Erro: LittleFS nao montado apos tentativas. Recursos de arquivo desabilitados.");
            fs_loaded = false;
        }
    }

    // Configure the Watchdog for both cores
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WATCHDOG_TIMEOUT * 1000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true};
    esp_task_wdt_init(&wdt_config);
    esp_task_wdt_add(NULL);

    // Load configuration
    if (fs_loaded)
        config_file_commands.get_config();

    // Initialize modules
    myserial.setup();
    if (hotspot_on)
    {
        connection.setup();
        web_server.setup();
        webhook.setup();
    }
    tag_commands.clear_tags();
    rgb.setup();
    pins.setup();
    reader_module.setup();

    // Create task for Core 0 (RGB + Pins)
    BaseType_t task_ok = xTaskCreatePinnedToCore(
        core0Task,             // Function to implement the task
        "Core0Task",           // Name of the task
        CORE0_TASK_STACK_SIZE, // Stack size in bytes (ESP32/ESP-IDF)
        NULL,                  // Task input parameter
        1,                     // Priority of the task (1 = low priority)
        &core0TaskHandle,      // Task handle
        0                      // Core where the task should run (0)
    );
    if (task_ok != pdPASS)
    {
        Serial.println("[task] Erro ao criar Core0Task");
    }
}

// ==================== Loop (Core 1 - Main) ====================
void loop()
{
    // Reset the Watchdog
    esp_task_wdt_reset();

    static unsigned long last_stack_check = 0;
    if (millis() - last_stack_check > 5000)
    {
        last_stack_check = millis();
        warn_low_stack("LoopTask", NULL);
    }

    // Process serial communication
    myserial.loop();

    tag_commands.ensure_protect_mode_correct();

    // Process reader module (Core 1)
    reader_module.functions();
    myserialcheck.loop();

    if (hotspot_on)
    {
        // Handle web server requests (synchronous server)
        web_server.loop();

        // Webhook tick
        webhook.loop();
    }
}
