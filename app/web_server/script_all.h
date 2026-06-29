void all_script()
{
  server.on("/read_on", HTTP_GET, []()
            {
                if(!gpi_start)
                    read_on = !read_on;
                server.send(200, "application/json", String(read_on)); });

  server.on("/clear_tags", HTTP_GET, []()
            {
                tag_commands.clear_tags();
                server.send(200, "application/json", "TAGS CLEARED"); });

  // Atualizacao de firmware via WebServer (sincrono)
  server.on("/update_firmware", HTTP_POST, []()
            {
              // Resposta enviada após upload terminar
              bool ok = !Update.hasError();
              if (ok)
              {
                server.send(200, "text/plain", "Atualizacao de firmware concluida com sucesso. Reinicie o dispositivo manualmente ou acesse /restart para reiniciar via web.");
                Serial.println("Atualizacao de firmware concluida com sucesso. Aguardando reinicio manual.");
              }
              else
              {
                server.send(500, "text/plain", "Erro na atualizacao de firmware. Verifique os logs.");
                Serial.println("Erro na atualizacao de firmware");
              } }, []()
            {
              HTTPUpload &upload = server.upload();
              if (upload.status == UPLOAD_FILE_START)
              {
                Serial.printf("Iniciando upload de firmware: %s\n", upload.filename.c_str());
                esp_task_wdt_reset();
                size_t expectedSize = (upload.totalSize > 0) ? upload.totalSize : UPDATE_SIZE_UNKNOWN;
                if (!Update.begin(expectedSize))
                {
                  Serial.println("Falha ao iniciar Update.begin()");
                  Update.printError(Serial);
                }
              }
              else if (upload.status == UPLOAD_FILE_WRITE)
              {
                esp_task_wdt_reset();
                yield();
                if (upload.currentSize && Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                  Serial.println("Erro ao gravar chunk de firmware");
                  Update.printError(Serial);
                }
              }
              else if (upload.status == UPLOAD_FILE_END)
              {
                esp_task_wdt_reset();
                if (Update.end(true))
                {
                  Serial.printf("Atualizacao de firmware recebida: %u bytes\n", upload.totalSize);
                }
                else
                {
                  Serial.println("Erro ao finalizar Update.end()");
                  Update.printError(Serial);
                }
              }
              else
              {
                // Outros status (abort/error)
                esp_task_wdt_reset();
                yield();
              } });

  // Atualizacao do filesystem (SPIFFS/LittleFS partition)
  server.on("/update_fs", HTTP_POST, []()
            {
              // Resposta enviada apos upload terminar
              server.send(200, "text/plain", (Update.hasError()) ? "Falha na atualizacao do filesystem. Verifique os logs." : "Atualizacao do filesystem concluida com sucesso. Reinicie o dispositivo manualmente ou acesse /restart para reiniciar via web."); }, []()
            {
              HTTPUpload &upload = server.upload();
              if (upload.status == UPLOAD_FILE_START)
              {
                Serial.printf("Recebendo FS: %s\n", upload.filename.c_str());
                esp_task_wdt_reset();

                // Desmonta LittleFS antes de gravar na particao do filesystem para evitar corrupcao
                if (fs_loaded)
                {
                  Serial.println("[update_fs] Desmontando LittleFS antes de gravar particao");
                  LittleFS.end();
                  fs_loaded = false;
                  delay(10);
                }

                size_t expectedSize = (upload.totalSize > 0) ? upload.totalSize : UPDATE_SIZE_UNKNOWN;
                if (!Update.begin(expectedSize, U_SPIFFS, 0x290000))
                {
                  Serial.println("Falha ao iniciar Update.begin() para FS");
                  Update.printError(Serial);
                  // tenta remount para evitar ficar sem filesystem
                  if (!fs_loaded)
                  {
                    if (LittleFS.begin(false))
                    {
                      fs_loaded = true;
                      Serial.println("[update_fs] Remontado LittleFS apos falha em Update.begin()");
                    }
                    else
                    {
                      Serial.println("[update_fs] Remontagem do LittleFS falhou");
                    }
                  }
                }
              }
              else if (upload.status == UPLOAD_FILE_WRITE)
              {
                esp_task_wdt_reset();
                yield();
                if (upload.currentSize && Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                  Serial.println("Erro ao gravar chunk do FS");
                  Update.printError(Serial);
                }
              }
              else if (upload.status == UPLOAD_FILE_END)
              {
                esp_task_wdt_reset();
                if (Update.end(true))
                {
                  Serial.println("Atualizacao do FS concluida");
                }
                else
                {
                  Serial.println("Erro ao finalizar Update.end() para FS");
                  Update.printError(Serial);
                  // tenta remount do LittleFS caso a atualizacao tenha falhado
                  if (!fs_loaded)
                  {
                    if (LittleFS.begin(false))
                    {
                      fs_loaded = true;
                      Serial.println("[update_fs] LittleFS remontado apos falha no Update.end()");
                    }
                    else
                    {
                      Serial.println("[update_fs] Nao foi possivel remontar LittleFS apos falha");
                    }
                  }
                }
              }
              else
              {
                esp_task_wdt_reset();
                yield();
              } });

  // Endpoint para reiniciar o dispositivo via web
  server.on("/restart", HTTP_POST, []()
            {
              Serial.println("Requisicao de restart via /restart recebida. Reiniciando...");
              ESP.restart();
              server.send(200, "text/plain", "Reiniciando dispositivo..."); });
}
