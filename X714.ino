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
    connection.setup();
    myserial.setup();
    web_server.setup();
    webhook.setup();
    tag_commands.clear_tags();
    rgb.setup();
    pins.setup();
    reader_module.setup();

    // Create task for Core 0 (RGB + Pins)
    xTaskCreatePinnedToCore(
        core0Task,   // Function to implement the task
        "Core0Task", // Name of the task
        4096,        // Stack size in words
        NULL,        // Task input parameter
        1,           // Priority of the task (1 = low priority)
        NULL,        // Task handle
        0            // Core where the task should run (0)
    );
}

// ==================== Loop (Core 1 - Main) ====================
void loop()
{
    // Reset the Watchdog
    esp_task_wdt_reset();

    // Process serial communication
    myserial.loop();

    tag_commands.ensure_protect_mode_correct();

    // Process reader module (Core 1)
    reader_module.functions();
    myserialcheck.loop();

    // Handle web server requests (synchronous server)
    web_server.loop();

    // Webhook tick
    webhook.loop();
}
