#include <Arduino.h>

// -------- Configurações Iniciais --------
int SAM = 64;  // resolução inicial
float values[512];    // maior tamanho possível
uint8_t tvalues[512]; // buffer máximo

const int dacPin = 26;        // GPIO26 = DAC2
const int botaoTipoOnda = 27; // botão tipo de onda
const int botaoFreq = 33;     // botão frequência
const int botaoResol = 14;    // novo botão para resolução

float freq = 1000.0; // Frequência inicial
int per;             // período entre amostras (µs)
int tipoOnda = 1;    // 1=seno, 2=quadrada, 3=serra
int etapa = 0;       // controle de troca de frequência

// Debounce - tipo de onda
unsigned long lastDebounceTipo = 0;
bool ultimoEstadoTipo = HIGH;
bool estadoEstavelTipo = HIGH;

// Debounce - frequência
unsigned long lastDebounceFreq = 0;
bool ultimoEstadoFreq = HIGH;
bool estadoEstavelFreq = HIGH;

// Debounce - resolução
unsigned long lastDebounceResol = 0;
bool ultimoEstadoResol = HIGH;
bool estadoEstavelResol = HIGH;

// ---------------------------------------------------------
void geraOnda(int tipo)
{
  if (tipo == 1) // Senoidal
  {
    for (int i = 0; i < SAM; i++)
    {
      float val = (1 + sin(2 * PI * i / SAM)) / 2;
      tvalues[i] = (uint8_t)(val * 255);
    }
    Serial.println("→ Onda senoidal selecionada");
  }
  else if (tipo == 2) // Quadrada
  {
    for (int i = 0; i < SAM / 2; i++)
      tvalues[i] = 255;
    for (int i = SAM / 2; i < SAM; i++)
      tvalues[i] = 0;
    Serial.println("→ Onda quadrada selecionada");
  }
  else if (tipo == 3) // Serra
  {
    for (int i = 0; i < SAM; i++)
      tvalues[i] = (uint8_t)((i * 255) / SAM);
    Serial.println("→ Onda serra selecionada");
  }
}

void atualizaPeriodo()
{
  float periodo = (1.0 / freq) / SAM;
  periodo *= 1e6;
  per = (int)periodo;
  Serial.print("Frequência ajustada para: ");
  Serial.print(freq);
  Serial.print(" Hz | Resolução: ");
  Serial.println(SAM);
}

// ---------------- BOTÕES -----------------
void checaBotaoTipo()
{
  bool leitura = digitalRead(botaoTipoOnda);

  if (leitura != estadoEstavelTipo)
  {
    lastDebounceTipo = millis();
    estadoEstavelTipo = leitura;
  }

  if ((millis() - lastDebounceTipo) > 50)
  {
    if (estadoEstavelTipo == LOW && ultimoEstadoTipo == HIGH)
    {
      tipoOnda++;
      if (tipoOnda > 3)
        tipoOnda = 1;
      geraOnda(tipoOnda);
    }
    ultimoEstadoTipo = estadoEstavelTipo;
  }
}

void checaBotaoFreq()
{
  bool leitura = digitalRead(botaoFreq);

  if (leitura != estadoEstavelFreq)
  {
    lastDebounceFreq = millis();
    estadoEstavelFreq = leitura;
  }

  if ((millis() - lastDebounceFreq) > 50)
  {
    if (estadoEstavelFreq == LOW && ultimoEstadoFreq == HIGH)
    {
      etapa++;
      if (etapa == 1)
        freq = 500.0;
      else if (etapa == 2)
        freq = 1000.0;
      else if (etapa == 3)
        freq = 2000.0;
      else if (etapa == 4)
      {
        freq = 4000.0;
        etapa = 0;
      }
      atualizaPeriodo();
    }
    ultimoEstadoFreq = estadoEstavelFreq;
  }
}

void checaBotaoResol()
{
  bool leitura = digitalRead(botaoResol);

  if (leitura != estadoEstavelResol)
  {
    lastDebounceResol = millis();
    estadoEstavelResol = leitura;
  }

  if ((millis() - lastDebounceResol) > 50)
  {
    if (estadoEstavelResol == LOW && ultimoEstadoResol == HIGH)
    {
      // alterna resolução
      if (SAM == 16)
        SAM = 32;
      else if (SAM == 32)
        SAM = 64;
      else if (SAM == 64)
        SAM = 128;
      else if (SAM == 128)
        SAM = 256;
      else if (SAM == 256)
        SAM = 512;
      else
        SAM = 16;

      geraOnda(tipoOnda);
      atualizaPeriodo();
      Serial.print("→ Resolução alterada para ");
      Serial.println(SAM);
    }
    ultimoEstadoResol = estadoEstavelResol;
  }
}

// ---------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  pinMode(botaoTipoOnda, INPUT_PULLUP);
  pinMode(botaoFreq, INPUT_PULLUP);
  pinMode(botaoResol, INPUT_PULLUP);

  Serial.println("\n=== GERADOR DE ONDAS ESP32 ===");

  geraOnda(tipoOnda);
  atualizaPeriodo();
  dacWrite(dacPin, 0);
}

// ---------------------------------------------------------
void loop()
{
  checaBotaoTipo();
  checaBotaoFreq();
  checaBotaoResol();

  for (int i = 0; i < SAM; i++)
  {
    dacWrite(dacPin, tvalues[i]);
    delayMicroseconds(per);
  }
}
