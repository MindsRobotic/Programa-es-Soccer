// HC-SR04 - Arduino Mega
const uint8_t TRIG_PIN = 7;   // TRIG -> D7 (pino de saída)
const uint8_t ECHO_PIN = 6;   // ECHO -> D6 (pino de entrada)
const unsigned long PULSE_TIMEOUT = 30000UL; // 30 ms

void setup() {
  Serial.begin(9600);       // mantém 9600 se você preferir
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  delay(100);
  Serial.println("HC-SR04 inicializado (Mega)");
}

void loop() {
  // Gera pulso TRIG (>=10 us)
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Lê duração do pulso HIGH no ECHO (microsegundos)
  unsigned long duracao = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT);

  if (duracao == 0) {
    // fallback manual para diagnóstico
    Serial.println("Sem leitura com pulseIn (duracao=0). Tentando fallback manual...");
    duracao = measurePulseManual(ECHO_PIN, PULSE_TIMEOUT);
    if (duracao == 0) {
      Serial.println("Fallback manual também retornou 0 -> sem leitura");
      Serial.print("Estado atual do ECHO (digitalRead): ");
      Serial.println(digitalRead(ECHO_PIN));
    } else {
      printDistance(duracao);
    }
  } else {
    printDistance(duracao);
  }

  delay(200); // intervalo entre leituras
}

unsigned long measurePulseManual(uint8_t pin, unsigned long timeoutMicros) {
  unsigned long start = micros();
  // espera subida
  while (digitalRead(pin) == LOW) {
    if (micros() - start > timeoutMicros) return 0;
  }
  unsigned long tRise = micros();
  // espera descida
  while (digitalRead(pin) == HIGH) {
    if (micros() - tRise > timeoutMicros) return 0;
  }
  unsigned long tFall = micros();
  return tFall - tRise;
}

void printDistance(unsigned long duracao) {
  float distancia = (duracao * 0.0343) / 2.0; // cm
  Serial.print("Duracao (us): ");
  Serial.print(duracao);
  Serial.print("  -> Distancia: ");
  Serial.print(distancia, 2);
  Serial.println(" cm");
}

