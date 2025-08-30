#include <Wire.h>
#include <HTInfraredSeeker.h>

// --- Motor 1 (Esquerda) ---
#define M1_ENA 2
#define M1_IN1 3
#define M1_IN2 4

// --- Motor 2 (Direita) ---
#define M2_ENB 7
#define M2_IN3 5
#define M2_IN4 6

// --- Motor 3 (Trás) ---
#define M3_ENA 8
#define M3_IN1 9
#define M3_IN2 10

// --- Solenoide (canal livre do L298N) ---
#define SOL_ENB 13
#define SOL_IN3 11
#define SOL_IN4 12

// --- Variáveis do IR ---
int ballDirection;
int ballIntens;

// --- Velocidade base ---
int velocidade = 255;

// --- PID ---
float kp = 1;
float ki = 0;
float kd = 0;
float erroAnterior = 0;
float proporcional, integral, derivativo = 0;


// --- Média móvel para suavizar direção ---
#define BUFFER_SIZE 5
int bufferDirecao[BUFFER_SIZE] = {5,5,5,5,5};
int bufferIndex = 0;

int mediaDirecao(int nova) {
  bufferDirecao[bufferIndex] = nova;
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
  int soma = 0;
  for (int i=0; i<BUFFER_SIZE; i++) soma += bufferDirecao[i];
  int media = soma / BUFFER_SIZE;
  return constrain(media, 0, 9); // garante que nunca passe de 9
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Inicializa IR Seeker
  InfraredSeeker::Initialize();

  // Configura pinos motores
  pinMode(M1_ENA, OUTPUT); pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
  pinMode(M2_ENB, OUTPUT); pinMode(M2_IN3, OUTPUT); pinMode(M2_IN4, OUTPUT);
  pinMode(M3_ENA, OUTPUT); pinMode(M3_IN1, OUTPUT); pinMode(M3_IN2, OUTPUT);

  // Solenoide
  pinMode(SOL_ENB, OUTPUT); pinMode(SOL_IN3, OUTPUT); pinMode(SOL_IN4, OUTPUT);

  Serial.println("Sistema iniciado. Seguindo a bola...");
}

// --- Funções de movimento ---
void parar() {
  analogWrite(M1_ENA, 0);
  analogWrite(M2_ENB, 0);
  analogWrite(M3_ENA, 0);
}

void PID(int erro, int velocidade) {
  proporcional = kp * erro;
  integral = ki * erro;
  derivativo = kd * (erro - erroAnterior);
  erroAnterior = erro;

  float correcao = proporcional + integral + derivativo;
  int velEsq, velDir, velTras = 0;
  if (erro == 0){
    velEsq = constrain(velocidade - correcao, 0, 255);
    velDir = constrain(velocidade + correcao, 0, 255) * -1;
    velTras = constrain(correcao, 0, 255);
  }

  if (erro < 0){
    velEsq = constrain(velocidade - correcao, 0, 255) * -1;
    velDir = constrain(velocidade + correcao, 0, 255) * -1;
    velTras = constrain(correcao, 0, 255) * -1;
  }

  if (erro > 0){
    velEsq = constrain(velocidade - correcao, 0, 255);
    velDir = constrain(velocidade + correcao, 0, 255);
    velTras = constrain(correcao, 0, 255);
  }

 Deslocar(velEsq, velDir, velTras);

}

void Deslocar(int motoresquerdo, int motordireito, int motortras){

  if (motoresquerdo >= 0){
    // Motor 1 (Esquerda)
    digitalWrite(M1_IN1, HIGH); digitalWrite(M1_IN2, LOW);
    analogWrite(M1_ENA, motoresquerdo);
  }

  if (motoresquerdo < 0){
    // Motor 1 (Esquerda)
    motoresquerdo = motoresquerdo * -1;
    digitalWrite(M1_IN1, LOW); digitalWrite(M1_IN2, HIGH);
    analogWrite(M1_ENA, motoresquerdo);
  }

    if (motordireito >= 0){
      // Motor 2 (Direita)     
      digitalWrite(M2_IN3, HIGH); digitalWrite(M2_IN4, LOW);
      analogWrite(M2_ENB, motordireito);
  }

  if (motordireito < 0){
      // Motor 2 (Direita)
      motordireito = motordireito * -1;      
      digitalWrite(M2_IN3, LOW); digitalWrite(M2_IN4, HIGH);
      analogWrite(M2_ENB, motordireito);
  }
  
  if (motortras >= 0){
      // Motor 3 (Traseiro)    
      digitalWrite(M3_IN1, HIGH); digitalWrite(M3_IN2, LOW);
      analogWrite(M3_ENA, motortras);
  }

  if (motortras < 0){
      // Motor 3 (Traseiro)
      motortras = motortras * -1;      
      digitalWrite(M3_IN1, LOW); digitalWrite(M3_IN2, HIGH);
      analogWrite(M3_ENA, motortras);
  }



}

// ------------------- Loop principal -------------------
void loop() {
  // --- Lê bola ---
  InfraredResult InfraredBall = InfraredSeeker::ReadAC(); // Realiza a leitura do sensor IR Seeker
  ballDirection = InfraredBall.Direction; // Armazena a direção na variável
  ballIntens = InfraredBall.Strength; // Armazena a direção na variável

  // --- Mostra valores ---
  Serial.print("Direção: "); Serial.print(ballDirection);
  Serial.print(" | Intensidade: "); Serial.println(ballIntens);

  // --- Controle de movimento com PID ---
  int erro = 5 - ballDirection; // Centro é 5
  PID(erro, velocidade);

millis();
}
