# X714 - Sistema Inteligente de Leitura RFID UHF

> **Solução profissional completa para identificação e rastreamento por RFID UHF**  
> Produto **SMARTX** - Tecnologia brasileira de alta performance

---

## 🎯 **Visão Geral**

O **X714** é um leitor RFID UHF de alto desempenho desenvolvido pela **SMARTX** para aplicações industriais e comerciais que exigem **precisão**, **confiabilidade** e **integração simples**. 

Totalmente **plug-and-play**, o X714 oferece múltiplas formas de conexão e controle:

✅ **Leitura simultânea de até 300 tags** em memória  
✅ **4 antenas independentes** com controle individual de potência (10-33 dBm)  
✅ **5 formas de conexão**: Serial, BLE, Ethernet, Emulador de Teclado e Webhook  
✅ **Interface web completa** para configuração sem necessidade de software externo  
✅ **Integração nativa** com sistemas externos via HTTP/HTTPS  
✅ **I/O programáveis** (3 entradas + 3 saídas) para automação industrial  
✅ **Escrita e proteção avançada** de tags RFID com alteração de senhas  
✅ **Pronto para usar** - não requer programação ou desenvolvimento  

---

## 🚀 **Principais Funcionalidades**

### 📡 **Leitura RFID de Alta Performance**

- **Protocolo**: EPC Gen2 (ISO 18000-6C)
- **Capacidade**: Até 300 tags simultâneas em memória
- **Modos de leitura**:
  - **Leitura contínua**: Captura automática de tags no campo
  - **Leitura por trigger GPI**: Ativação por sensor externo
  - **Leitura sob demanda**: Controle via comando ou interface web
- **Filtros configuráveis**:
  - RSSI mínimo ajustável
  - Sessão Gen2 (S0, S1, S2, S3)
  - Timeout de leitura personalizável

### 🔌 **Sistema Multi-Antena Inteligente**

- **4 portas de antena** com controle independente:
  - Liga/desliga individual por antena
  - Ajuste de potência de 10 a 33 dBm (por antena)
  - Tempo de permanência configurável (dwell time)
- **Balanceamento automático** entre antenas ativas
- **Feedback em tempo real** do RSSI (intensidade do sinal)

### 🌐 **Conectividade - 5 Formas de Integração**

O X714 oferece **máxima flexibilidade** com múltiplos protocolos de comunicação simultâneos:

#### **1. Serial**
- Conexão via cabo USB
- Protocolo de comandos baseado em texto

#### **2. Bluetooth Low Energy (BLE)**
- Comunicação sem fio de curto alcance
- Mesmo protocolo de comandos da serial
- Pareamento simples e rápido

#### **3. Ethernet**
- Conexão cabeada 10/100 Mbps full-duplex
- Suporta DHCP (automático) ou IP estático
- mDNS para descoberta automática na rede local
- Servidor Telnet integrado para debug e comandos remotos

#### **4. Emulador de Teclado (HID)**
- Simula um teclado USB conectado ao computador
- Tags lidas são digitadas automaticamente como se fossem teclado
- **Plug-and-play** total - não requer drivers ou software
- Compatível com Windows, Linux, macOS, Android

#### **5. Webhook (HTTP/HTTPS)**
- Envio automático de eventos para servidores externos
- Suporte a HTTP e HTTPS
- Payload JSON estruturado com eventos de tag e keep-alive
- Configuração via interface web (URL, período, nome do dispositivo)
- Batching inteligente (até 50 tags por requisição)

#### **Interface Web Embarcada**
Além das formas de comunicação, o X714 possui um servidor web completo acessível via navegador:

| Página | Função |
|--------|--------|
| **Home** | Dashboard com status do sistema |
| **Reader Config** | Configuração de potência, sessão, modos de leitura |
| **Antenna Config** | Controle individual das 4 antenas |
| **Ethernet Config** | Configuração de rede (DHCP/estático) |
| **Webhook Config** | Integração HTTP/HTTPS para envio de eventos |
| **GPO Test** | Teste manual de saídas digitais |
| **Reader Status** | Monitoramento de tags lidas em tempo real |

**Acesso**: Basta digitar o IP do dispositivo no navegador - sem instalação de software!

### 🔒 **Funcionalidades Avançadas de Tags RFID**

#### **Escrita e Programação**

- **Escrita de EPC** com e sem filtros de seleção
- **Suporte a filtros** EPC e TID para escrita seletiva
- **Validação automática** de dados hexadecimais
- **Configuração de potência** específica para escrita

#### **Gerenciamento de Segurança**

- **Alteração de senhas**: Modificação segura das senhas de acesso das tags
- **Modo protegido**: Ativação/desativação de proteção contra escrita não autorizada
- **Autenticação por senha**: Controle de acesso baseado em senha da tag
- **Validação de integridade**: Verificação automática de parâmetros antes da escrita

**Casos de uso de segurança:**
- Proteção de tags críticas contra alteração acidental
- Controle de acesso a dados sensíveis
- Rastreabilidade com garantia de integridade
- Personalização segura de identificadores

### 🔌 **I/O Industrial**

#### **Entradas Digitais (GPI)**

- **3 entradas** opto-isoladas (5V/24V compatível)
- **Modos de operação**:
  - Trigger de leitura
  - Início/parada automática
  - Delay configurável de parada

#### **Saídas Digitais (GPO)**

- **3 saídas** programáveis
- **Aplicações**:
  - Sinalização visual (torre luminosa)
  - Controle de cancelas/portas
  - Acionamento de alarmes
  - Trigger de câmeras

### 🎨 **Feedback Visual e Sonoro**

- **LED RGB integrado** com indicação de status:
  - 🟢 Verde: Leitura ativa
  - 🔴 Vermelho: Erro de comunicação
  - 🟡 Amarelo: Aguardando leitura
- **Buzzer** para feedback sonoro (configurável via software)

---

## 📊 **Casos de Uso**

### 🏭 **Industrial**

- Controle de produção (WIP tracking)
- Rastreamento de ferramentas e ativos
- Controle de acesso de pessoal

### 📦 **Logística**

- Controle de estoque em tempo real
- Portaria inteligente (entrada/saída)
- Separação automatizada (picking)

### 🏬 **Varejo**

- Inventário automatizado
- Antifurto inteligente
- Self-checkout

### 🏥 **Saúde**

- Rastreamento de medicamentos
- Controle de instrumentais cirúrgicos
- Gestão de prontuários

---

## ⚙️ **Especificações Técnicas**

| Característica | Especificação |
|----------------|--------------|
| **Frequência** | 902–907,5 - 915–928 MHz |
| **Potência de saída** | 10 a 33 dBm  |
| **Portas de antena** | 4  |
| **Interface de rede** | Ethernet 10/100 Mbps |
| **Protocolos** | HTTP, HTTPS, Telnet, mDNS, Serial (UART), Bluetooth (BLE), HID (Teclado) |
| **GPO/GPI** | 3 entradas / 3 saídas |
| **Alimentação** | 5V DC |

---

## 🔐 **Segurança**

- Persistência local de configurações (LittleFS)
- Suporte a HTTPS (webhook com certificados auto-assinados)
- Controle de acesso via rede (IP estático)
- Watchdog multi-core para recuperação automática

---

## 🛠️ **Manutenção e Suporte**

### **Diagnóstico**

- Logs via Telnet (porta 23)
- Monitor serial (115200 baud)
- Página de status web em tempo real

### **Atualizações**

- OTA (Over-The-Air) via interface web:
  - Upload de firmware (.bin)
  - Upload de filesystem (.bin)

---

## 📝 **Configuração Rápida**

### **Opção 1: Conexão via WiFi (Access Point)**

**Ideal para primeira configuração ou quando não há cabo de rede disponível**

1. **Ligue o equipamento**
   - Aguarde cerca de 5 segundos até o LED ficar amarelo

2. **Conecte-se à rede WiFi do dispositivo**
   - No seu computador, tablet ou smartphone, procure por redes WiFi disponíveis
   - Procure pela rede: **`SMTX-<número>`** (exemplo: SMTX-A1B2C3D4)
   - Senha padrão: **`smartx12345`**

3. **Acesse a interface web**
   - Abra seu navegador (Chrome, Firefox, Edge, Safari)
   - Digite na barra de endereços: **`192.168.4.1`**
   - Pressione Enter

4. **Pronto!**
   - A interface de configuração do X714 será exibida
   - Configure as antenas, Ethernet e demais parâmetros

> 💡 **Dica**: Após configurar o Ethernet, você pode desconectar do WiFi e acessar o dispositivo via cabo de rede

---

### **Opção 2: Conexão via Ethernet**

**Ideal para instalação permanente em rede corporativa**

1. **Conecte o cabo Ethernet**
   - Conecte um cabo de rede entre o X714 e seu switch/roteador

2. **Descubra o IP do dispositivo**
   - **Opção A (via WiFi)**: Conecte-se ao WiFi do dispositivo (veja Opção 1) e verifique o IP atual em **Ethernet Config**
   - **Opção B (via serial)**: Use um terminal serial e envie o comando `#get_info`

3. **Acesse a interface web**
   - Digite o IP descoberto no navegador
   - Configure o modo de rede (DHCP ou IP estático) conforme sua infraestrutura

---

### **Opção 3: Primeiro Teste Rápido (Modo Teclado)**

**Para testar sem configuração - apenas conecte e use**

1. **Ative o modo Emulador de Teclado**
   - Conecte via WiFi ou Ethernet (opções acima)
   - Na interface web, vá em **Reader Modes**
   - Ative a opção **"Keyboard"**

2. **Conecte às antenas**
   - Conecte as antenas RFID nas portas desejadas

3. **Teste imediatamente**
   - Abra um bloco de notas (Notepad, Word, etc.)
   - Aproxime uma tag RFID da antena
   - O código será "digitado" automaticamente no documento

> 💡 **Plug-and-Play**: Nesse modo, o X714 funciona como um teclado - não precisa instalar nada!

---


### **Teste de Leitura**

1. Acesse **Reader** na interface web
2. Clique em **"READ"**
3. Aproxime tags RFID das antenas
4. Visualize as leituras em tempo real na tela

---

## 🎓 **Diferenciais Competitivos**

| Característica | X714 | Concorrentes |
|----------------|-----------|--------------|
| **5 formas de conexão** | ✅ Serial, BLE, Ethernet, HID, Webhook | ❌ Apenas 1 ou 2 protocolos |
| **Interface web embarcada** | ✅ Completa, sem instalação | ❌ Requer software proprietário |
| **Emulador de teclado** | ✅ Plug-and-play total | ❌ Não disponível |
| **Webhook nativo** | ✅ HTTP/HTTPS integrado | ⚠️ Apenas via gateway externo |
| **Escrita avançada de tags** | ✅ EPC, TID, filtros, senhas | ⚠️ Funcionalidade limitada |
| **Modo protegido** | ✅ Proteção contra escrita | ❌ Não disponível |
| **Configuração simples** | ✅ Via navegador | ❌ Software desktop obrigatório |
| **Multi-plataforma** | ✅ Windows, Linux, macOS, Android | ⚠️ Apenas Windows |
| **Suporte técnico** | ✅ Nacional (Brasil) | ❌ Importado, suporte limitado |

---

## 🔐 **Segurança e Confiabilidade**

- Persistência local de configurações em memória flash
- Suporte a HTTPS para comunicação segura via webhook
- Watchdog automático para recuperação de falhas
- Proteção contra perda de dados em quedas de energia
- Rede WiFi com senha protegida (padrão: smartx12345)

---

## 📞 **Contato e Suporte**

Para mais informações sobre o **X714**:

- **Fabricante**: SMARTX - Tecnologia RFID Brasileira
- **Suporte técnico**: Disponível em português
- **Documentação**: Guia completo de comandos e integração disponível

---

## 📄 **Licença**

Produto desenvolvido e fabricado por **SMARTX** - Todos os direitos reservados.

---

**Transforme sua operação com tecnologia RFID inteligente. Escolha X714.**


