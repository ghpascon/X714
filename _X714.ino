#include "pins.h"
#include "helpers.h"
#include "libs.h"
#include "vars.h"
#include <freertos/semphr.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#define WATCHDOG_TIMEOUT 10

// SETTINGS
#include "app/settings.h"
Settings settings;

// TAGS
#include "app/tags.h"
Tags tags;

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

        // Small delay to prevent task from starving other processes
        vTaskDelay(pdMS_TO_TICKS(10)); // 10ms delay
    }
}

// ==================== Setup ====================
void setup()
{
    // Initialize the file system
    if (!LittleFS.begin())
    {
        Serial.println("Error initializing LittleFS!");
        fs_loaded = false;
    }

    // Configure the Watchdog for both cores
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WATCHDOG_TIMEOUT * 1000,
        .idle_core_mask = (1 << portNUM_PROCESSORS) - 1,
        .trigger_panic = true};
    esp_task_wdt_init(&wdt_config);
    esp_task_wdt_add(NULL);

    // Load Settings
    settings.load();

    // Load configuration
    if (fs_loaded)
        config_file_commands.get_config();

    // Initialize modules
    connection.setup();
    myserial.setup();
    web_server.setup();
    webhook.setup();

    // Pre-reserve String buffers for tags to reduce heap churn/fragmentation
    // cada EPC/TID esperado tem ~24 chars hex; reservar um pouco mais (32)
    for (int i = 0; i < max_tags; i++)
    {
        tags[i].epc.reserve(24);
        tags[i].tid.reserve(24);
    }
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

    // Pause for stability
    delay(500);
}

// ==================== Loop (Core 1 - Main) ====================
void loop()
{
    // Reset the Watchdog
    esp_task_wdt_reset();

    // SETTINGS
    settings.save();

    // Process serial communication
    myserial.loop();

    tag_commands.ensure_protect_mode_correct();

    // Process reader module (Core 1)
    reader_module.functions();
    myserialcheck.loop();

    // Save configuration
    config_file_commands.save_config();

    // Handle web server requests (synchronous server)
    web_server.loop();

    // Webhook tick
    webhook.loop();
}
