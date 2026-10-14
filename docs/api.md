# X714 Web API (v1)

Esta documentacao descreve a API HTTP usada pelo frontend do X714.

## Autenticacao

Todas as rotas HTML e API exigem autenticacao HTTP Basic.

- Usuario: `smartx`
- Senha: `smartx12345`
- Sessao de autenticacao: 5 minutos

## Formato padrao de resposta

### Sucesso simples

```json
{
  "status": "ok"
}
```

### Erro

```json
{
  "status": "error",
  "message": "mensagem do erro"
}
```

### Sucesso com dados

```json
{
  "status": "ok",
  "data": {}
}
```

## Rotas HTML

- `GET /`
- `GET /reader`
- `GET /ant_config`
- `GET /reader_config`
- `GET /reader_modes`
- `GET /gpo_test`
- `GET /eth_config`
- `GET /webhook_config`
- `GET /prefix`
- `GET /protected_inventory`
- `GET /wifi_config`
- `GET /style.css`
- `GET /FONT_SMARTX.woff`

## API

### Health

- `GET /api/v1/status`

Resposta: status da API.

### Device

- `GET /api/v1/device/info`

Retorna dados do equipamento:

- `name`
- `bt_mac`
- `eth_mac`
- `ip`
- `version`
- `power`

- `POST /api/v1/device/restart`

Reinicia o dispositivo.

### Reader

- `GET /api/v1/reader/state`

Retorna estado de leitura:

- `reading` (bool)
- `reading_label` (ON/OFF)
- `temperature`
- `total_tags`
- `button_label`
- `button_enabled`

- `POST /api/v1/reader/toggle-reading`

Alterna estado de leitura (quando permitido).

- `POST /api/v1/reader/clear-tags`

Limpa tags lidas.

- `GET /api/v1/reader/tags`

Retorna snapshot de tags em `data`:

- `index`
- `epc`
- `tid`
- `antenna`
- `rssi`

### Reader Modes

- `GET /api/v1/reader/modes`

Retorna:

- `start_reading`
- `gpi_start`
- `always_send`
- `simple_send`
- `keyboard`
- `buzzer_on`
- `decode_gtin`
- `indicator_time`

- `POST /api/v1/reader/modes`

Atualiza um ou mais campos por JSON.

Exemplo:

```json
{
  "always_send": true,
  "indicator_time": 2500
}
```

### Reader Config

- `GET /api/v1/reader/config`

Retorna:

- `simple_send`
- `session`
- `gpi_stop_delay`

- `POST /api/v1/reader/config`

Exemplo:

```json
{
  "session": 2,
  "gpi_stop_delay": 1000
}
```

### Antenna Config

- `GET /api/v1/antenna/config`

Retorna array `data` com:

- `id`
- `active`
- `power`
- `rssi`

- `POST /api/v1/antenna/config`

Exemplo:

```json
{
  "antennas": [
    { "id": 1, "active": true, "power": 25, "rssi": -70 },
    { "id": 2, "active": false, "power": 20, "rssi": -75 }
  ]
}
```

### GPO

- `GET /api/v1/gpo`

Retorna array booleano em `data` com estados dos GPOs.

- `POST /api/v1/gpo/toggle`

Body:

```json
{
  "index": 1
}
```

### Network - Ethernet

- `GET /api/v1/network/ethernet`

Retorna:

- `dhcp_on`
- `static_ip`
- `gateway_ip`
- `subnet_mask`
- `connected`
- `current_ip`

- `POST /api/v1/network/ethernet`

Exemplo:

```json
{
  "dhcp_on": false,
  "static_ip": "192.168.1.120",
  "gateway_ip": "192.168.1.1",
  "subnet_mask": "255.255.255.0"
}
```

### Network - Wi-Fi

- `GET /api/v1/network/wifi`

Retorna:

- `wifi_ssid`
- `wifi_password`
- `connected`
- `current_ip`

- `POST /api/v1/network/wifi`

Exemplo:

```json
{
  "wifi_ssid": "MinhaRede",
  "wifi_password": "MinhaSenha"
}
```

### Webhook

- `GET /api/v1/webhook`

Retorna:

- `webhook_on`
- `webhook_url`
- `webhook_interval`

- `POST /api/v1/webhook`

Exemplo:

```json
{
  "webhook_on": true,
  "webhook_url": "https://example.com/webhook",
  "webhook_interval": 10000
}
```

### Prefix

- `GET /api/v1/prefix`

Retorna `prefix`.

- `POST /api/v1/prefix`

Exemplo:

```json
{
  "prefix": "ABC"
}
```

### Protected Inventory

- `GET /api/v1/protected-inventory`

Retorna:

- `enabled`
- `password`

- `POST /api/v1/protected-inventory`

Exemplo:

```json
{
  "enabled": true,
  "password": "A1B2C3D4"
}
```

Regras de senha:

- 8 caracteres hexadecimais (0-9, A-F)
- vazio = `00000000`

### Update

- `POST /api/v1/system/update/firmware`
- `POST /api/v1/system/update/filesystem`

Upload via `multipart/form-data` com campo `firmware`.
