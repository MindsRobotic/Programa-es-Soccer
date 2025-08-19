#include <Wire.h>              // Biblioteca I2C
#include <HTInfraredSeeker.h>  // Biblioteca do HiTechnic IR Seeker

void setup() {
  Serial.begin(9600);                       // Inicia comunicação serial
  Serial.println("Dir\tStrength");          // Cabeçalho para leitura
  InfraredSeeker::Initialize();             // Inicializa (sem parâmetro nesta lib)
}

void loop() {
  // Faz a leitura no modo AC (1200 Hz da bola de soccer)
  InfraredResult ir = InfraredSeeker::ReadAC();

  if (ir.Direction != 0) {                  // Se detectou a bola
    Serial.print(ir.Direction);             // Direção 1..9 (5 = frente)
    Serial.print('\t');
    Serial.println(ir.Strength);            // Intensidade 0..255
  } else {
    Serial.println("Sem sinal da bola");
  }

  delay(100); // Atualiza a cada 100 ms
}
