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
              bool ok = !Update.hasError();
              if (ok)
              {
                server.send(200, "text/plain", "Atualizacao de firmware bem-sucedida! Reiniciando...");
                Serial.println("Atualizacao de firmware bem-sucedida! Reiniciando...");
                delay(200);
                ESP.restart();
              }
              else
              {
                server.send(500, "text/plain", "Erro na atualizacao de firmware");
                Serial.println("Erro na atualizacao de firmware");
              } }, []()
            {
              HTTPUpload &upload = server.upload();
              if (upload.status == UPLOAD_FILE_START)
              {
                Serial.printf("Iniciando upload de firmware: %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN))
                {
                  Update.printError(Serial);
                }
              }
              else if (upload.status == UPLOAD_FILE_WRITE)
              {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                  Update.printError(Serial);
                }
              }
              else if (upload.status == UPLOAD_FILE_END)
              {
                if (!Update.end(true))
                {
                  Update.printError(Serial);
                }
              } });

  server.on("/update_fs", HTTP_POST, []()
            {
              server.send(200, "text/plain", (Update.hasError()) ? "Falha" : "Sucesso");
              ESP.restart(); }, []()
            {
              HTTPUpload &upload = server.upload();
              if (upload.status == UPLOAD_FILE_START)
              {
                Serial.printf("Recebendo %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_SPIFFS, 0x290000))
                {
                  Update.printError(Serial);
                }
              }
              else if (upload.status == UPLOAD_FILE_WRITE)
              {
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
                {
                  Update.printError(Serial);
                }
              }
              else if (upload.status == UPLOAD_FILE_END)
              {
                if (Update.end(true))
                {
                  Serial.println("Atualização do FS concluída");
                }
                else
                {
                  Update.printError(Serial);
                }
              } });
}
