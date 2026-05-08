# Documentação do Conector GPIO — X714

O conector GPIO do X714 possui **10 pinos** e permite integrar sensores externos, atuadores e sinalizadores diretamente ao equipamento. Abaixo está toda a documentação necessária para fazer as conexões corretamente.

---

## 📌 Pinagem do Conector

```
        X714
       ┌──────────┐
  1 ───┤ GPO3     │  ← Saída (open drain)
  2 ───┤ GPO2     │  ← Saída (open drain)
  3 ───┤ GPO1     │  ← Saída (open drain)
  4 ───┤ GPO_TAG  │  ← Saída (ativa ao ler tag)
  5 ───┤ GND_OPTO │  ← GND dos optoacopladores
  6 ───┤ IN3      │  ← Entrada digital 3
  7 ───┤ IN2      │  ← Entrada digital 2
  8 ───┤ IN1      │  ← Entrada digital 1
  9 ───┤ GND      │  ← GND geral
 10 ───┤ +12V     │  ← Alimentação 12V
       └──────────┘
```

---

## 📋 Tabela de Pinos

| Pino | Nome     | Tipo                      | Descrição                                             |
| ---- | -------- | ------------------------- | ----------------------------------------------------- |
| 1    | GPO3     | Saída (open drain / sink) | Puxa para GND_OPTO quando acionado                    |
| 2    | GPO2     | Saída (open drain / sink) | Puxa para GND_OPTO quando acionado                    |
| 3    | GPO1     | Saída (open drain / sink) | Puxa para GND_OPTO quando acionado                    |
| 4    | GPO_TAG  | Saída (open drain / sink) | Puxa para GND_OPTO quando uma tag é lida (automático) |
| 5    | GND_OPTO | Referência                | GND isolado para as saídas (optoacopladores)          |
| 6    | IN3      | Entrada digital           | Entrada 3 — aciona ao receber GND (nível baixo)       |
| 7    | IN2      | Entrada digital           | Entrada 2 — aciona ao receber GND (nível baixo)       |
| 8    | IN1      | Entrada digital           | Entrada 1 — aciona ao receber GND (nível baixo)       |
| 9    | GND      | Alimentação / referência  | GND geral da placa                                    |
| 10   | +12V     | Alimentação               | Saída de 12V para alimentar dispositivos              |

---

## 🔵 Entradas Digitais (IN1, IN2, IN3)

As entradas `IN1`, `IN2` e `IN3` são ativadas quando recebem **GND** (nível baixo). Internamente possuem pull-up, ou seja, ficam em nível alto em repouso e ativam quando o sinal é puxado para GND.

**Uso típico:** sensores NPN (coletor aberto), chaves para GND, botões ligados ao GND.

> ⚠️ **Importante:** use o GND do próprio conector (pino 9) como referência para os sensores.

---

### 📐 Exemplo: Sensor de Proximidade → IN1

Ligação de um sensor de 3 fios tipo **NPN** (coletor aberto — puxa saída para GND quando detecta) nas entradas do X714:

```
                                         X714
  ┌─────────────────────┐               ┌──────────┐
  │    SENSOR (NPN)     │               │          │
  │                     │               │          │
  │  VCC (+) ───────────┼───────────────┤ 10 (+12V)│
  │                     │               │          │
  │  GND (-) ───────────┼───────────────┤  9 (GND) │
  │                     │               │          │
  │  OUT ───────────────┼───────────────┤  8 (IN1) │
  │  (puxa para GND     │               │          │
  │   ao detectar)      │               │          │
  └─────────────────────┘               └──────────┘
```

**Como funciona:** em repouso, `IN1` fica em nível alto (pull-up interno). Quando o sensor detecta e puxa `OUT` para GND, a entrada é ativada.

**Resumo da ligação:**

| Fio do sensor | Conectar em      | Pino |
| ------------- | ---------------- | ---- |
| VCC / +       | +12V             | 10   |
| GND / -       | GND              | 9    |
| OUT / Sinal   | IN1 (ou IN2/IN3) | 8    |

> 💡 O mesmo esquema vale para **IN2** (pino 7) e **IN3** (pino 6). Basta trocar o fio de sinal para o pino correspondente.

---

## 🔴 Saídas Open Drain (GPO1, GPO2, GPO3, GPO_TAG)

As saídas do X714 funcionam como **chaves de aterramento** (open drain / sink). Isso significa que, quando ativadas, elas **puxam o pino para o `GND_OPTO`**. Quando inativas, o pino fica "flutuando" (circuito aberto).

> ℹ️ **GPO_TAG** segue o mesmo comportamento elétrico dos demais GPOs — também puxa para `GND_OPTO` quando ativa. A diferença é que seu acionamento é **automático** (disparado pelo leitor ao detectar uma tag), enquanto GPO1/2/3 são controlados por software.

**Como funciona:**

```
  Inativo:   Pino GPO ──────── (aberto / desconectado)
  Ativo:     Pino GPO ──────── GND_OPTO (pino 5)
```

**Uso típico:** buzzers, relés, sinalizadores, LEDs com resistor.

> ⚠️ **Importante:** o `GND_OPTO` (pino 5) **deve** ser conectado a um GND para que as saídas funcionem. Como as saídas são isoladas opticamente, ele pode ser ligado ao **GND externo do circuito que você quer acionar** (uso mais comum) ou diretamente ao **GND da placa** (pino 9) — ambos funcionam. A vantagem do isolamento é poder usar um GND externo independente da placa.

---

### 📐 Exemplo: Buzzer/Sinalizador → GPO_TAG (ativa ao ler tag)

O `GPO_TAG` ativa automaticamente toda vez que o leitor captura uma tag RFID. É ideal para acionar um buzzer ou sirene como confirmação de leitura.

```
                                         X714
                            ┌────────────────────────┐
                            │                        │
  +12V ──────────────────── ┤ 10 (+12V)              │
          │                 │                        │
     ┌────┴─────┐           │                        │
     │  BUZZER  │           │                        │
     │  ou      │           │                        │
     │ SIRENE   │           │                        │
     └────┬─────┘           │                        │
          │                 │                        │
          └──────────────── ┤  4 (GPO_TAG)           │
                            │                        │
                    ┌────── ┤  5 (GND_OPTO)          │
                    │       │                        │
                    └────── ┤  9 (GND)               │
                            │                        │
                            └────────────────────────┘
```

**Como funciona:**

1. O `+12V` alimenta o positivo do buzzer/sirene
2. O negativo do buzzer vai para o `GPO_TAG` (pino 4)
3. O `GND_OPTO` (pino 5) é conectado ao GND do circuito
4. Quando uma tag é lida → `GPO_TAG` fecha o circuito para `GND_OPTO` → buzzer aciona

**Resumo da ligação:**

| Terminal do buzzer | Conectar em | Pino |
| ------------------ | ----------- | ---- |
| + (positivo)       | +12V        | 10   |
| - (negativo)       | GPO_TAG     | 4    |
| GND referência     | GND_OPTO    | 5    |

> 💡 O mesmo esquema vale para **GPO1** (pino 3), **GPO2** (pino 2) e **GPO3** (pino 1), que são acionados via comando por software.

---

## ⚡ Diagrama Completo: Sensor + Buzzer juntos

Exemplo com sensor na IN1 e buzzer no GPO_TAG ao mesmo tempo:

```
                                               X714
  ┌──────────────┐                            ┌──────────┐
  │    SENSOR    │                            │          │
  │  VCC ────────┼────────────────────────────┤ 10 (+12V)│
  │  GND ────────┼────────────────────────────┤  9 (GND) │
  │  OUT ────────┼────────────────────────────┤  8 (IN1) │
  └──────────────┘                            │          │
                                              │          │
  ┌──────────────┐                            │          │
  │    BUZZER    │                            │          │
  │    + ────────┼────────────────────────────┤ 10 (+12V)│
  │    - ────────┼────────────────────────────┤  4 GPO_TAG│
  └──────────────┘                            │          │
                                              │          │
                                      ┌────── ┤  5 GND_OPTO
                                      │       │          │
                                      └────── ┤  9 (GND) │
                                              └──────────┘
```

---

## 🔧 Dicas e Cuidados

- **GND_OPTO ≠ GND geral:** o `GND_OPTO` é o retorno isolado das saídas. Conecte-o ao GND do circuito que você quer acionar.
- **Sensor NPN vs PNP:**
  - **NPN (coletor aberto):** saída vai a GND quando ativado → **compatível direto** com IN1/IN2/IN3.
  - **PNP / saída positiva:** saída vai a +12V quando ativado → **não** é compatível direto (a entrada ativa em GND, não em positivo). Necessita circuito inversor ou adaptação.
- **Proteção contra inversão de polaridade:** verifique sempre a polaridade antes de energizar.
