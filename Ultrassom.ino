// Pinos dos motores
const int M1_IN1 = 7; // Esquerda
const int M1_IN2 = 6;
const int M2_IN1 = 5; // Trás
const int M2_IN2 = 4;
const int M3_IN1 = 3; // Direita
const int M3_IN2 = 2;

// ... (Aqui vai o código do sensor ultrassônico que já passamos)

// Setup motor
void setup() {
  Serial.begin(9600);
  // Sensores setup (como antes)
  for (int i = 0; i < 3; i++) {
    pinMode(sensores[i].trigPin, OUTPUT);
    pinMode(sensores[i].echoPin, INPUT);
    sensores[i].tempoAnterior = millis();
    sensores[i].distanciaAnterior = distanciaFiltrada(sensores[i].trigPin, sensores[i].echoPin);
  }
  
  // Motores como saída
  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);
  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);
  pinMode(M3_IN1, OUTPUT);
  pinMode(M3_IN2, OUTPUT);

  // Parar motores inicialmente
  pararMotores();
}

// Funções para controlar motores

void pararMotores() {
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, LOW);
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, LOW);
  digitalWrite(M3_IN1, LOW);
  digitalWrite(M3_IN2, LOW);
}

void seguirEmFrente() {
  // Motor 1 (esquerda) pra frente
  digitalWrite(M1_IN1, HIGH);
  digitalWrite(M1_IN2, LOW);

  // Motor 2 (trás) pra frente
  digitalWrite(M2_IN1, HIGH);
  digitalWrite(M2_IN2, LOW);

  // Motor 3 (direita) pra frente
  digitalWrite(M3_IN1, HIGH);
  digitalWrite(M3_IN2, LOW);

  Serial.println("Seguindo em frente");
}

void desviarParaDireita() {
  // Motor esquerda para frente
  digitalWrite(M1_IN1, HIGH);
  digitalWrite(M1_IN2, LOW);

  // Motor trás para trás (gira o robô)
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, HIGH);

  // Motor direita para trás
  digitalWrite(M3_IN1, LOW);
  digitalWrite(M3_IN2, HIGH);

  Serial.println("Desviando para direita");
}

void desviarParaEsquerda() {
  // Motor esquerda para trás
  digitalWrite(M1_IN1, LOW);
  digitalWrite(M1_IN2, HIGH);

  // Motor trás para trás (gira o robô)
  digitalWrite(M2_IN1, LOW);
  digitalWrite(M2_IN2, HIGH);

  // Motor direita para frente
  digitalWrite(M3_IN1, HIGH);
  digitalWrite(M3_IN2, LOW);

  Serial.println("Desviando para esquerda");
}

// No loop principal, você escolhe qual desvio usar, por exemplo:
// if (desviar) desviarParaDireita(); else seguirEmFrente();
