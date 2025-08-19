#include <NewPing.h>

#define SONAR_NUM     3     // Número de sensores
#define MAX_DISTANCE  400   // Distância máxima em cm (ajuste conforme necessidade)
#define PING_INTERVAL 33    // Intervalo entre pings em ms (mínimo ~29 ms para evitar cross-talk)

NewPing sonar[SONAR_NUM] = {
  NewPing(13, 12, MAX_DISTANCE),  // Sensor 1
  NewPing(11, 10, MAX_DISTANCE),  // Sensor 2
  NewPing( 9,  8, MAX_DISTANCE)  // Sensor 3
  
};

unsigned int distancia[SONAR_NUM];

void setup() {
  Serial.begin(600);
  Serial.println("Iniciando leitura de 3 sensores ultra-sônicos com NewPing");
}

void loop() {
  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    delay(PING_INTERVAL);
    distancia[i] = sonar[i].ping_cm(); // ou ping_median(iteracoes) se quiser filtro
    Serial.print("Sensor ");
    Serial.print(i + 1);
    Serial.print(": ");
    if (distancia[i] == 0) {
      Serial.println("sem leitura ou acima do alcance");
    } else {
      Serial.print(distancia[i]);
      Serial.println(" cm");
    }
  }
  Serial.println("---");
  delay(200); // Ajuste conforme necessidade
}
