# esp32-waveform-generator

Gerador de ondas configurável para ESP32 com saída DAC. Suporta formas de onda senoidal, quadrada e serra com frequência e resolução ajustáveis via botões físicos.

##  Galeria do Projeto

![Montagem do ESP 32](https://github.com/user-attachments/assets/99814cfb-010c-4430-98d4-039499c6760c)

![Senoide 0 238Khz](https://github.com/user-attachments/assets/6fdc8a39-0dad-428d-aea4-9b2637a2546c)

![Senoide 0 300Khz](https://github.com/user-attachments/assets/4c28fd49-5bb0-4f74-9da3-6924dc794f2b)

![Dente de Serra 0 796Khz](https://github.com/user-attachments/assets/513a6531-1efd-4071-8c0b-f55eab16304d)

![Senoide 0 815 Khz](https://github.com/user-attachments/assets/811c1c18-d93a-4844-9247-3d89db8f4dab)

![Quadrada 0 818Khz](https://github.com/user-attachments/assets/8d2e95d7-7871-4273-af65-1d20672347d5)

![Senoide 1 418 Khz](https://github.com/user-attachments/assets/1b8c9a4b-f63b-4628-9386-a25310b6a7a2)

![Senoide 2 196 Khz](https://github.com/user-attachments/assets/b1fa609d-5585-4416-b0c3-ec654c057170)

##  Características

- **3 Tipos de Onda**: Senoidal, Quadrada e Dente de Serra
- **Frequências Programadas**: 500 Hz, 1 kHz, 2 kHz e 4 kHz
- **Frequências Reais Medidas**: ~238 Hz a ~2.2 kHz (veja análise abaixo)
- **Resolução Configurável**: 16, 32, 64, 128, 256 ou 512 amostras por ciclo
- **Controle via Botões**: Interface física com 3 botões
- **Saída DAC**: GPIO26 (DAC2) - 0 a 3.3V, 8 bits

##  Hardware

- ESP32 (qualquer modelo com DAC)
- 3 Botões push-button
- Protoboard e jumpers
- Osciloscópio (para medição)

## 🔌 Pinagem

| Componente | GPIO |
|------------|------|
| Saída DAC | 26 |
| Botão Tipo de Onda | 27 |
| Botão Frequência | 33 |
| Botão Resolução | 14 |

Botões conectados entre GPIO e GND (pull-up interno ativo).

##  Instalação

1. Clone o repositório:
```bash
git clone https://github.com/seu-usuario/esp32-waveform-generator.git
```

2. Abra `gerador.ino` na Arduino IDE

3. Instale suporte ESP32:
   - File > Preferences > Additional Board Manager URLs
   - Adicione: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
   - Tools > Board > Boards Manager > Instale "esp32"

4. Selecione sua placa ESP32 e faça upload

##  Como Usar

- **Botão GPIO 27**: Alterna tipo de onda (Senoidal → Quadrada → Serra)
- **Botão GPIO 33**: Alterna frequência (500 Hz → 1 kHz → 2 kHz → 4 kHz)
- **Botão GPIO 14**: Alterna resolução (16 → 32 → 64 → 128 → 256 → 512 amostras)

Serial Monitor (115200 baud) mostra configurações em tempo real.

## ⚠️ Análise: Discrepância de Frequências

### Comparação Programado vs Medido

| Programado | Medido | Erro |
|------------|--------|------|
| 500 Hz | ~238-300 Hz | ~40-52% |
| 1000 Hz | ~815 Hz | ~18% |
| 2000 Hz | ~1418 Hz | ~29% |
| 4000 Hz | ~2196 Hz | ~45% |

### Causas Identificadas

**1. Overhead de Software**

O loop principal não compensa o tempo de execução:
```cpp
for (int i = 0; i < SAM; i++) {
    dacWrite(dacPin, tvalues[i]);     // ~3-5 µs
    delayMicroseconds(per);           // delay teórico
    // Overhead do loop: ~1-2 µs
}
```

Cada iteração adiciona ~6-8 µs além do delay programado, aumentando o período real e reduzindo a frequência efetiva.

**2. Cálculo do Período**

Para 1000 Hz com 64 amostras:
- Período teórico: 15.625 µs por amostra
- Período real: 15.625 + 6 = ~21.6 µs
- Frequência resultante: 1000000 / (64 × 21.6) ≈ 723 Hz

**3. Imprecisão do `delayMicroseconds()`**
- Baixa precisão para delays < 10 µs
- Arredondamento float → int
- Overhead do timer

**4. Interrupções do Sistema**
- WiFi, Bluetooth (se ativos)
- Tarefas do RTOS
- Causam jitter adicional

### Soluções Propostas

**Solução 1: Compensação Simples**
```cpp
void atualizaPeriodo() {
    float periodo = (1.0 / freq) / SAM;
    periodo *= 1e6;
    per = (int)(periodo - 6); // Compensa overhead
    if (per < 1) per = 1;
}
```

**Solução 2: Timer Hardware (Recomendado)**
```cpp
hw_timer_t *timer = NULL;

void IRAM_ATTR onTimer() {
    dacWrite(dacPin, tvalues[currentSample++]);
    if (currentSample >= SAM) currentSample = 0;
}

void setup() {
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, per, true);
    timerAlarmEnable(timer);
}
```

**Solução 3: I2S + DMA (Melhor)**

Para frequências altas e máxima estabilidade, usar driver I2S com DMA elimina completamente o overhead de software.

##  Especificações Técnicas

- **Tensão**: 0 a 3.3V
- **Resolução DAC**: 8 bits (0-255)
- **Frequência Real**: ~238 Hz a ~2.2 kHz
- **Taxa de Amostragem**: Variável conforme configuração
- **Precisão**: ±20-50% (limitação de software)

## 📄 Licença

MIT License

## 🤝 Contribuições

Pull requests são bem-vindos! Para mudanças importantes, abra uma issue primeiro.

---

**Projeto educacional para comunidade maker**

