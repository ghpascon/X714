# X714 - Guia de Webhook

## 1) Acesso ao leitor no browser

Antes de configurar webhook, consulte o manual de conexao do leitor para o procedimento completo de rede e acesso web.

Resumo rapido para acesso via hotspot do leitor:

1. Conecte no Wi-Fi do leitor (SSID com prefixo `SMTX`, por exemplo `SMTX-XXXXXXXXXXXX`).
2. Senha do Wi-Fi: `smartx12345`.
3. Abra o navegador em: `http://192.168.4.1`.
4. Login:
   - Usuario: `smartx`
   - Senha: `smartx12345`

## 2) Como abrir a pagina de webhook

1. Na pagina HOME, clique no botao **WEBHOOK CONFIG**.
2. A pagina de configuracao de webhook sera aberta.

## 3) Como configurar o webhook

Na pagina WEBHOOK CONFIG:

1. Em **WEBHOOK**, selecione `ON` para habilitar (ou `OFF` para desabilitar).
2. Em **URL**, informe a URL de destino que vai receber os POSTs.
   - Exemplo HTTP: `http://server:5001/events`
   - Exemplo HTTPS: `https://api.exemplo.com/events`
3. Clique em **SET CONFIG** para salvar.
4. Confira em **CURRENT CONFIG** se os valores foram aplicados.

## 4) Como o envio funciona

- O leitor envia dados em `POST` com `Content-Type: application/json`.
- O envio ocorre em ciclo de aproximadamente 10 segundos.
- Se houver tags, envia eventos de `tag` em lote (array JSON).
- Se nao houver tags no periodo, envia um evento `keep_alive`.

## 5) Exemplo de POST - keep_alive

Quando nao ha tags para enviar, o payload e:

```json
[
  {
    "device": "SMTX-XXXXXXXXXXXX",
    "event_type": "keep_alive",
    "event_data": {}
  }
]
```

## 6) Exemplo de POST - tag

Quando ha leitura de tag, o payload segue este formato:

```json
[
  {
    "device": "SMTX-XXXXXXXXXXXX",
    "event_type": "tag",
    "event_data": {
      "epc": "300833B2DDD9014000000000",
      "tid": "E28011700000020F4C123456",
      "ant": 1,
      "rssi": 45
    }
  },{
    "device": "SMTX-XXXXXXXXXXXX",
    "event_type": "tag",
    "event_data": {
      "epc": "300833B2DDD9014000000001",
      "tid": "E28011700000020F4C123457",
      "ant": 1,
      "rssi": 45
    }
  }

]
```

Observacao:

- Quando existir mais de uma tag no ciclo, o leitor envia um array com varios objetos `event_type: "tag"` no mesmo POST (em batches).
