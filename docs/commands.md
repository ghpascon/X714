# X714 - Manual de Comunicacao

## Visao geral

O X714 recebe comandos pelas interfaces:

- Serial (UART)
- USB CDC (porta virtual)
- BLE
- Ethernet/Telnet (TCP porta 23)

O mesmo parser processa os comandos para todas as interfaces.

## Regras do parser

1. Comandos sao case-insensitive no identificador do comando.
2. Parametros textuais sensiveis (exemplo: SSID, senha Wi-Fi, URL) preservam o texto original apos `:`.
3. Caracteres de controle (`\r`, `\n`, `\0`) sao removidos na entrada.
4. `\t` vira espaco.
5. Caracteres fora de ASCII imprimivel sao descartados.

## Comandos em lote

### Lote geral (multiplos comandos)

No parser atual, o lote geral e separado por novo inicio de comando `#`.

Exemplo valido:

```text
#read:on#clear#buzzer:on
```

### Lote especial do `#set_cmd:`

Dentro de `#set_cmd:` o separador e `|`, e os subcomandos devem ser enviados sem `#`.

Exemplo valido:

```text
#set_cmd:session:1|read_power:30|buzzer:on|start_reading:on
```

## Comandos suportados

### Conectividade e informacoes

| Comando           | Descricao                         | Resposta                                                                            |
| ----------------- | --------------------------------- | ----------------------------------------------------------------------------------- |
| `#ping` ou `ping` | Teste de conectividade            | `#PONG`                                                                             |
| `#get_state`      | Estado de leitura                 | `#READING` ou `#IDLE`                                                               |
| `#get_info`       | Informacoes gerais                | `#NAME:...`, `#BT_MAC:...`, `#ETH_MAC:...`, `#IP:...`, `#VERSION:...`, `#POWER:...` |
| `#get_serial`     | Nome/serial logico do dispositivo | `#SERIAL:...`                                                                       |
| `#restart`        | Reinicia ESP                      | sem resposta (reinicia)                                                             |

### Leitura de tags e buffer

| Comando                      | Descricao                                 | Resposta                                             |
| ---------------------------- | ----------------------------------------- | ---------------------------------------------------- |
| `#read:on` ou `readtag on`   | Liga leitura                              | `#READ:ON` (evento) e limpeza de buffer              |
| `#read:off` ou `readtag off` | Desliga leitura                           | `#READ:OFF` (evento)                                 |
| `#get_tags`                  | Lista EPCs em buffer                      | `#tags:@EPC1@EPC2...` e depois `#TAGS_CLEARED`       |
| `#get_tags_all`              | Lista EPC/TID/ANT/RSSI                    | #tags:@EPC\|TID\|ANT\|RSSI... e depois #TAGS_CLEARED |
| `#clear`                     | Limpa buffer de tags                      | `#TAGS_CLEARED`                                      |
| `initreadtag:on`             | Habilita leitura automatica no boot/fluxo | `#INIT_READ_TAG:on`                                  |
| `initreadtag:off`            | Desabilita leitura automatica             | `#INIT_READ_TAG:off`                                 |

### Escrita e protecao de tags

| Comando                                      | Descricao                                     | Formato                                                                                           |
| -------------------------------------------- | --------------------------------------------- | ------------------------------------------------------------------------------------------------- |
| `#write:[EPC];[PASSWORD]`                    | Escrita sem filtro                            | `#write:000102030405060708090A0B;12345678`                                                        |
| `#write:[EPC];[PASSWORD];epc;[TARGET]`       | Escrita filtrando por EPC atual               | `#write:...;...;epc;AABB...`                                                                      |
| `#write:[EPC];[PASSWORD];tid;[TARGET]`       | Escrita filtrando por TID                     | `#write:...;...;tid;E280...`                                                                      |
| `#change_password:[EPC];[NEW_PWD];[OLD_PWD]` | Altera senha da tag                           | `#change_password:000102...;ABCD1234;00000000`                                                    |
| `#protected_mode:[EPC];[PASSWORD];[ENABLE]`  | Ativa/desativa modo protegido da tag          | `#protected_mode:000102...;12345678;on`                                                           |
| `#protected_inventory:[ENABLE];[PASSWORD]`   | Inventario protegido global                   | `#protected_inventory:on;12345678` (pode emitir tambem uma linha auxiliar `1 senha` ou `0 senha`) |
| `#get_protected_inventory`                   | Le estado/senha atual do inventario protegido | `#PROTECTED_INVENTORY:...` e `#PROTECTED_INVENTORY_PASSWORD:...`                                  |

Observacoes de validacao:

- `ENABLE`: `on/off` ou `true/false`.
- `#write` valida hex e tamanho multiplo de 4 caracteres para campos hex.
- `#change_password` exige EPC com 24 hex e senhas com 8 hex.

### Potencia, sessao, antena e modo

| Comando                                  | Descricao                                            | Resposta                                     |
| ---------------------------------------- | ---------------------------------------------------- | -------------------------------------------- |
| `#get_power`                             | Le potencia atual da antena 1                        | `#ANT_POWER:[valor]`                         |
| `#read_power:[VALOR]`                    | Define potencia de leitura global                    | `#READ_POWER:[valor]` e `#POWER:[valor]`     |
| `readpower:[VALOR]`                      | Alias para potencia de leitura                       | `#READ_POWER:[valor]`                        |
| `#get_session`                           | Le sessao atual                                      | `#SESSION:[valor]`                           |
| `#session:[VALOR]`                       | Define sessao                                        | `#SESSION:[valor]`                           |
| `gen2session:[VALOR]`                    | Alias legacy de sessao                               | `#GEN2_SESSION:[valor]`                      |
| `#set_ant:[N],[ATIVO],[POTENCIA],[RSSI]` | Configura antena                                     | `#ANT_CONFIGURED:N,on/off,power,rssi`        |
| `#setup_reader`                          | Reinicia sequencia de setup do modulo reader         | `#SETUP_STARTED` (e depois eventos de setup) |
| `readmode hid`                           | Saida em modo teclado                                | `#READ_MODE:HID`                             |
| `readmode normal`                        | Saida normal (USB CDC)                               | `#READ_MODE:USB`                             |
| `#buzzer:on` / `#buzzer:off`             | Liga/desliga buzzer                                  | `#BUZZER:on/off`                             |
| `#decode_gtin:on` / `#decode_gtin:off`   | Liga/desliga conversao EPC para GTIN                 | `#DECODE_GTIN:on/off`                        |
| `#keyboard:on` / `#keyboard:off`         | Liga/desliga envio em teclado HID                    | `#KEYBOARD:on/off`                           |
| `#always_send:on` / `#always_send:off`   | Forca envio continuo de tags                         | `#ALWAYS_SEND:on/off`                        |
| `#simple_send:on` / `#simple_send:off`   | Envio simplificado de tag                            | `#SIMPLE_SEND:on/off`                        |
| `#gpi_start:on` / `#gpi_start:off`       | Modo GPI controla leitura ou apenas reporta entradas | `#GPI_START:on/off`                          |
| `#gpi_stop_delay:[MS]`                   | Delay de parada GPI                                  | sem resposta                                 |
| `#measure_rl:[ANT]`                      | Mede RL com frequencia padrao                        | sem resposta textual garantida               |
| `#measure_rl:[ANT],[FREQ]`               | Mede RL em frequencia especifica                     | sem resposta textual garantida               |

### Prefixo e regravacao assistida

| Comando                 | Descricao                                           | Resposta                |
| ----------------------- | --------------------------------------------------- | ----------------------- |
| `#prefix:[VALOR]`       | Filtro de prefixo para aceitar tags                 | `#PREFIX:[valor]`       |
| `#get_prefix`           | Le prefixo configurado                              | `#PREFIX:[valor]`       |
| `#write_prefix:[VALOR]` | Prefixo de destino para fluxo de regravacao por TID | `#WRITE_PREFIX:[valor]` |
| `#get_write_prefix`     | Le `write_prefix` atual                             | `#WRITE_PREFIX:[valor]` |

### Rede, Wi-Fi e webhook

| Comando                        | Descricao                     | Resposta                    |
| ------------------------------ | ----------------------------- | --------------------------- |
| `#hotspot:on` / `#hotspot:off` | Liga/desliga hotspot          | `#HOTSPOT:ON/OFF`           |
| `#dhcp:on` / `#dhcp:off`       | Liga/desliga DHCP da Ethernet | `#DHCP:ENABLED/DISABLED`    |
| `#static_ip:[IP]`              | Define IP estatico            | `#STATIC_IP:[ip]`           |
| `#gateway_ip:[IP]`             | Define gateway                | `#GATEWAY_IP:[ip]`          |
| `#subnet_mask:[MASK]`          | Define mascara                | `#SUBNET_MASK:[mask]`       |
| `#wifi_ssid:[SSID]`            | Define SSID de STA            | `#WIFI_SSID:[ssid]`         |
| `#wifi_password:[SENHA]`       | Define senha de STA           | `#WIFI_PASSWORD:[senha]`    |
| `#webhook:on` / `#webhook:off` | Liga/desliga envio webhook    | `#WEBHOOK:ENABLED/DISABLED` |
| `#webhook_url:[URL]`           | Define URL do webhook         | `#WEBHOOK_URL:[url]`        |

### GPIO de saida (GPO)

| Comando               | Descricao           | Resposta        |
| --------------------- | ------------------- | --------------- |
| `#gpo:[N],on`         | Liga saida GPO N    | `#GPO:N,ON`     |
| `#gpo:[N],off`        | Desliga saida GPO N | `#GPO:N,OFF`    |
| `#gpo:[N],true/false` | Alias booleano      | `#GPO:N,ON/OFF` |

### `#set_cmd:` (atalho de multiplas configuracoes)

Formato:

```text
#set_cmd:cmd1|cmd2|cmd3
```

Subcomandos suportados (sem `#` dentro do payload):

- `set_ant:[N],[ATIVO],[POTENCIA],[RSSI]`
- `session:[VALOR]`
- `read_power:[VALOR]`
- `buzzer:on/off`
- `gpi_stop_delay:[MS]`
- `decode_gtin:on/off`
- `start_reading:on/off`
- `gpi_start:on/off`
- `always_send:on/off`
- `simple_send:on/off`
- `keyboard:on/off`

Observacao importante:

- O firmware atual nao envia `#SET_CMD:OK/NOK` nem `#CMD:...`.
- Cada subcomando responde individualmente com sua propria resposta normal.

## Mensagens emitidas pelo leitor sem pergunta (eventos espontaneos)

Esta secao lista mensagens que podem chegar sem um comando imediato do host.

### Fluxo de tags

| Mensagem                                | Quando ocorre                                          |
| --------------------------------------- | ------------------------------------------------------ |
| #T+@EPC\|TID\|ANT\|RSSI\|on/off         | Nova leitura de tag em modo normal (`simple_send=off`) |
| `EPC` (ou GTIN quando `decode_gtin=on`) | Nova leitura em modo simplificado (`simple_send=on`)   |
| `#ADDED_TO_TARGET_MAP:TID->TARGET`      | Fluxo de `write_prefix` adiciona alvo por TID          |
| `#REMOVED_FROM_TARGET_MAP:TID`          | Tag ja chegou com prefixo alvo e sai do mapa           |

### Estado de leitura e entradas

| Mensagem                 | Quando ocorre                                         |
| ------------------------ | ----------------------------------------------------- |
| `#READ:ON` / `#READ:OFF` | Mudanca de estado da leitura (comando ou logica GPI)  |
| `#IN_1:ON/OFF`           | Mudanca da entrada digital 1 (quando `gpi_start=off`) |
| `#IN_2:ON/OFF`           | Mudanca da entrada digital 2 (quando `gpi_start=off`) |
| `#IN_3:ON/OFF`           | Mudanca da entrada digital 3 (quando `gpi_start=off`) |

### Setup, watchdog e reconexao do reader

| Mensagem                                  | Quando ocorre                                 |
| ----------------------------------------- | --------------------------------------------- |
| `#STEP:[N]`                               | Avanco da maquina de setup do modulo reader   |
| `#ONE_ANT:true/false`                     | Descoberta de configuracao de antena no setup |
| `#SETUP_DONE`                             | Fim do setup do reader                        |
| `#TIMEOUT`                                | Timeout de comunicacao com o reader           |
| `#TRY_CHANGE_BAUDRATE`                    | Tentativa automatica de ajuste de baudrate    |
| `change_baudrate_ok`                      | Baudrate ajustado com sucesso                 |
| `change_baudrate_retry`                   | Nova tentativa de ajuste de baudrate          |
| `change_baudrate_nok`                     | Falha de ajuste, seguido de restart           |
| `#RECONNECT_115200 (x/y)`                 | Tentativa de reconexao no baud atual          |
| `#RECONNECT_FAILED: restart`              | Reconexao esgotada, reinicializando           |
| `#SESSION:[valor]`                        | Sessao reaplicada durante setup               |
| `#NAME:...`, `#VERSION:...`, `#POWER:...` | Informacoes emitidas ao final do setup        |

### Retornos de operacoes de escrita/protecao

| Mensagem                                     | Origem                                      |
| -------------------------------------------- | ------------------------------------------- |
| `#TAG_WRITE:OK` / `#TAG_WRITE:ERROR`         | Resultado de comandos `#write`              |
| `#LOCK:OK` / `#LOCK:ERROR`                   | Resultado de `#change_password`             |
| `#TAG_PROTECTED:OK` / `#TAG_PROTECTED:ERROR` | Resultado de `#protected_mode`              |
| `#ANT_ERROR:`                                | Erro de antena retornado pelo modulo reader |
| `#TEMPERATURA:[valor]`                       | Telemetria de temperatura do reader         |

### Rede/Telnet e telemetria de integracao

| Mensagem                                           | Quando ocorre                                 |
| -------------------------------------------------- | --------------------------------------------- |
| `#CONNECTED`                                       | Cliente conectou no Telnet                    |
| `#BUSY:ONLY_ONE_CLIENT_ALLOWED`                    | Segundo cliente tentou conectar no Telnet     |
| `#DISCONNECTING`                                   | Segundo cliente sera desconectado             |
| `#CONNECTED_CLIENT:[ip]`                           | IP do cliente Telnet que permaneceu conectado |
| `#ERROR:CMD_TOO_LONG`                              | Linha Telnet acima do limite de parser        |
| `Ethernet state: ...`                              | Mudanca de estado da Ethernet                 |
| `WiFi state: connected/disconnected`               | Mudanca de estado do Wi-Fi                    |
| `BLE stopped due to Ethernet connection`           | BLE desligado por conexao Ethernet ativa      |
| `BLE stopped due to WiFi connection`               | BLE desligado por conexao Wi-Fi ativa         |
| `Sending batch N`                                  | Envio de lote webhook                         |
| `Posting HTTP/HTTPS to ...`                        | Inicio de envio webhook                       |
| HTTP code: ... \| Response: ...                    | Resposta HTTP do webhook                      |
| `HTTP POST failed: ...` / `HTTPS POST failed: ...` | Falha no webhook                              |
| `Failed to start HTTP(S) connection`               | Falha de abertura de conexao webhook          |

## Erros e mensagens de fallback

| Mensagem                                                                 | Significado                                  |
| ------------------------------------------------------------------------ | -------------------------------------------- |
| `#INVALID_CMD: [comando]`                                                | Comando nao reconhecido pelo parser          |
| `#ERROR:Invalid command prefix`                                          | Prefixo invalido para comandos estruturados  |
| `#ERROR:Missing parameters`                                              | Parametros faltando                          |
| `#ERROR:Invalid parameters count`                                        | Quantidade de parametros invalida            |
| `#ERROR:Invalid EPC`                                                     | EPC invalido                                 |
| `#ERROR:Invalid password`                                                | Senha invalida                               |
| `#ERROR:Invalid new password`                                            | Nova senha invalida                          |
| `#ERROR:Invalid old password`                                            | Senha antiga invalida                        |
| `#ERROR:Invalid enable parameter`                                        | Parametro de enable invalido                 |
| `#ERROR:Invalid command format`                                          | Formato invalido (ex.: `#measure_rl`)        |
| `#ERROR:Missing EPC or password`                                         | Parametros obrigatorios ausentes em `#write` |
| `#ERROR:Too many separators`                                             | Separadores `;` em excesso em `#write`       |
| `#ERROR:Invalid target type/value (must be hex and multiple of 4 chars)` | Filtro `epc/tid` invalido em `#write`        |

## Notas finais

1. O parser interno converte o comando para minusculas antes de validar.
2. Em algumas configuracoes, um comando pode gerar mais de uma linha de resposta.
3. O equipamento pode emitir eventos espontaneos a qualquer momento durante a operacao.
4. O host deve tratar o protocolo como stream de linhas, e nao como request/response estrito.
