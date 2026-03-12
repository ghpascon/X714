# XPAD PLUS - Comandos de Comunicação

## Visão Geral

O XPAD PLUS aceita comandos através de múltiplas interfaces de comunicação:

- **Serial** (UART)
- **Bluetooth Low Energy (BLE)**
- **Ethernet (TCP/IP)**

Os comandos podem ser enviados individualmente ou em lote, utilizando o separador `|` para múltiplos comandos em uma única transmissão.

---

## Comandos Básicos

### Conectividade e Status

| Comando | Descrição | Resposta |
|---------|-----------|----------|
| `#ping` ou `ping` | Testa conectividade | `#PONG` |
| `#get_state` | Obtém estado atual de leitura | `#READING` ou `#IDLE` |
| `#get_info` | Informações do dispositivo | `#NAME:`, `#BT_MAC:`, `#ETH_MAC:`, `#IP:` |
| `#get_serial` | Serial do dispositivo | `#SERIAL:[serial]` |
| `#restart` | Reinicia o dispositivo | - |

### Controle de Leitura

| Comando | Descrição | Resposta |
|---------|-----------|----------|
| `#read:on` ou `readtag on` | Inicia leitura de tags | `#READ:ON` |
| `#read:off` ou `readtag off` | Para leitura de tags | `#READ:OFF` |
| `initreadtag:on` | Inicia leitura automaticamente | - |
| `initreadtag:off` | Desabilita leitura automática | - |

### Gerenciamento de Tags

| Comando | Descrição | Resposta |
|---------|-----------|----------|
| `#get_tags` | Lista epcs detectados | Lista de epcs |
| `#get_tags_all` | Lista todas as informações das tags | Lista completa de tags |
| `#clear` | Limpa buffer de tags | - |

### Escrita de Tags

| Comando | Descrição | Formato |
|---------|-----------|---------|
| `#write:[EPC];[PASSWORD]` | Escreve EPC sem filtro | `#write:000102030405060708090A0B;12345678` |
| `#write:[EPC];[PASSWORD];EPC;[TARGET]` | Escreve EPC com filtro no epc | `#write:000102030405060708090A0B;12345678;epc;AABBCCDDEEFF001122334455` |
| `#write:[EPC];[PASSWORD];TID;[TARGET]` | Escreve EPC com filtro no tid | `#write:000102030405060708090A0B;12345678;tid;E28012341234123412341234` |

**Parâmetros de escrita:**

- **EPC**: 24 caracteres hexadecimais (12 bytes)
- **PASSWORD**: 8 caracteres hexadecimais (4 bytes)
- **TYPE**: `epc` ou `tid`
- **TARGET**: 24 caracteres hexadecimais para filtro

### Gerenciamento Avançado de Tags

| Comando | Descrição | Formato |
|---------|-----------|---------|
| `#change_password:[EPC];[NEW_PWD];[OLD_PWD]` | Altera senha de tag específica | `#change_password:000102030405060708090A0B;ABCD1234;00000000` |
| `#protected_mode:[EPC];[PASSWORD];[ENABLE]` | Ativa/desativa modo protegido para tag específica | `#protected_mode:000102030405060708090A0B;12345678;on` |
| `#protected_inventory:[ENABLE];[PASSWORD]` | Ativa/desativa inventário protegido global | `#protected_inventory:on;12345678` |

**Parâmetros avançados:**

- **EPC**: 24 caracteres hexadecimais (12 bytes) - EPC da tag alvo
- **NEW_PWD**: 8 caracteres hexadecimais (4 bytes) - Nova senha
- **OLD_PWD**: 8 caracteres hexadecimais (4 bytes) - Senha atual (padrão: 00000000)
- **PASSWORD**: 8 caracteres hexadecimais (4 bytes) - Senha de acesso (padrão: 00000000)
- **ENABLE**: `on`/`off` ou `true`/`false` - Habilita/desabilita proteção

**Respostas específicas:**

- `#CHANGE_PASSWORD:OK` - Senha alterada com sucesso
- `#PROTECTED_MODE:ENABLED` - Modo protegido ativado para tag específica
- `#PROTECTED_MODE:DISABLED` - Modo protegido desativado para tag específica
- `#PROTECTED_INVENTORY:ENABLED` - Inventário protegido ativado globalmente
- `#PROTECTED_INVENTORY:DISABLED` - Inventário protegido desativado globalmente
- `#ERROR:[DESCRIÇÃO]` - Erro na execução (EPC inválido, senha incorreta, etc.)

---

## Configuração de Potência e Sessão

| Comando | Descrição | Faixa/Valores |
|---------|-----------|---------------|
| `#read_power:[VALOR]` ou `readpower:[VALOR]` | Define potência de leitura | Conforme especificação |
| `#get_power` | Obtém potência atual | `#POWER:[valor]` |
| `#session:[VALOR]` ou `gen2session:[VALOR]` | Define sessão Gen2 | 0 ao máximo configurado |
| `#get_session` | Obtém sessão atual | `#SESSION:[valor]` |
| `#password[...]` | Altera senha do reader | Conforme protocolo |

---

## Configurações Gerais

### Modo de Operação

| Comando | Descrição | Valores |
|---------|-----------|---------|
| `readmode hid` | Modo teclado (HID) | - |
| `readmode normal` | Modo normal | - |
| `#buzzer:on` | Habilita buzzer | `#BUZZER:ON` |
| `#buzzer:off` | Desabilita buzzer | `#BUZZER:OFF` |
| `#decode_gtin:on` | Habilita decodificação GTIN | - |
| `#decode_gtin:off` | Desabilita decodificação GTIN | - |

### Configuração de Rede

| Comando | Descrição | Valores |
|---------|-----------|---------|
| `#hotspot:on` | Habilita modo hotspot | - |
| `#hotspot:off` | Desabilita modo hotspot | - |

### Configuração de Prefixo

| Comando | Descrição | Exemplo |
|---------|-----------|---------|
| `#prefix:[VALOR]` | Define prefixo para tags | `#prefix:TAG_` |
| `#get_prefix` | Obtém prefixo atual | `#PREFIX:[valor]` |
| `#get_protected_inventory` | Obtém status do inventário protegido | `#PROTECTED_INVENTORY:[status]`, `#PROTECTED_INVENTORY_PASSWORD:[senha]` |

---

## Comando de Configuração Múltipla

### `#set_cmd:[COMANDOS]`

Permite configurar múltiplas opções simultaneamente usando o separador `|`.

**Comandos disponíveis no set_cmd:**

| Comando | Descrição | Exemplo |
|---------|-----------|---------|
| `set_ant:[N],[ATIVO],[POTÊNCIA],[RSSI]` | Configura antena específica | `set_ant:1,on,30,25` |
| `session:[VALOR]` | Define sessão | `session:1` |
| `read_power:[VALOR]` | Potência de leitura | `read_power:30` |
| `write_power:[VALOR]` | Potência de escrita | `write_power:30` |
| `buzzer:on/off` | Controle do buzzer | `buzzer:on` |
| `gpi_stop_delay:[MS]` | Delay de parada GPI | `gpi_stop_delay:1000` |
| `decode_gtin:on/off` | Decodificação GTIN | `decode_gtin:on` |
| `start_reading:on/off` | Leitura automática | `start_reading:on` |
| `gpi_start:on/off` | Início por GPI | `gpi_start:on` |
| `always_send:on/off` | Envio contínuo | `always_send:on` |
| `simple_send:on/off` | Envio simplificado | `simple_send:on` |
| `keyboard:on/off` | Modo teclado | `keyboard:on` |

**Exemplo de uso:**

```text
#set_cmd:session:1|read_power:30|buzzer:on|start_reading:on
```

**Resposta:** Para cada comando processado:

- `#CMD:[comando]` (log do comando)
- `#SET_CMD:OK` ou `#SET_CMD:NOK`

---

## Comandos em Lote

### Múltiplos Comandos com `|`

É possível enviar vários comandos em uma única transmissão, separados por `|`:

**Exemplo:**

```text
#read:on|#clear|#buzzer:on
```

**Comportamento:**

- Cada comando é processado sequencialmente
- Comandos `#set_cmd:` são tratados de forma especial (não são separados)
- Outros comandos são processados individualmente

---

## Códigos de Resposta

| Resposta | Significado |
|----------|-------------|
| `#PONG` | Resposta ao ping |
| `#READ:ON/OFF` | Estado de leitura alterado |
| `#READING/#IDLE` | Estado atual de leitura |
| `#BUZZER:ON/OFF` | Estado do buzzer |
| `#CHANGE_PASSWORD:OK` | Senha alterada com sucesso |
| `#PROTECTED_MODE:ENABLED` | Modo protegido ativado para tag específica |
| `#PROTECTED_MODE:DISABLED` | Modo protegido desativado para tag específica |
| `#PROTECTED_INVENTORY:ENABLED` | Inventário protegido ativado globalmente |
| `#PROTECTED_INVENTORY:DISABLED` | Inventário protegido desativado globalmente |
| `#PROTECTED_INVENTORY_PASSWORD:[senha]` | Senha atual do inventário protegido |
| `#INVALID_CMD` | Comando não reconhecido |
| `#ERROR:[DESCRIÇÃO]` | Erro na execução (parâmetros inválidos, etc.) |
| `#SET_CMD:OK/NOK` | Resultado da configuração |

---

## Notas Importantes

1. **Comandos são case-sensitive** - mantenha a capitalização exata
2. **Valores hexadecimais** devem usar caracteres 0-9, A-F (maiúsculos ou minúsculos)
3. **Separadores** para múltiplos comandos: `|` (pipe)
4. **Separadores** para parâmetros: `;` (ponto e vírgula) ou `:` (dois pontos)
5. **Configurações** são aplicadas imediatamente e podem reconfigurar o reader
6. **Reinicialização** pode ser necessária após certas configurações
7. **Funcionalidades avançadas** como alteração de senhas e modo protegido requerem validação rigorosa dos parâmetros

---

## Suporte Técnico

Para dúvidas técnicas ou implementação específica, consulte a documentação técnica completa ou entre em contato com o suporte.
