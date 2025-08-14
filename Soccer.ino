#include <Wire.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_Sensor.h>
#include <HTInfraredSeeker.h>

// --- Pinos dos motores ---
const int M1_IN1 = 7; // Esquerda
const int M1_IN2 = 6;
const int M2_IN1 = 5; // Trás
const int M2_IN2 = 4;
const int M3_IN1 = 3; // Direita
const int M3_IN2 = 2;

// --- BNO055 ---
#define BNO_ADDRESS 0x28
Adafruit_BNO055 bno = Adafruit_BNO055(55, BNO_ADDRESS);

// --- Variáveis IR ---
int direcao;
int intensidade;

// --- Velocidade ---
int velocidadeFrente = 255;
int velocidadeGiro = 200;

// --- PID ---
float Kp, Ki, Kd;
float erroAnterior = 0;
float integral = 0;
const float MAX_CORRECAO = 50; // Limite de correção para não virar demais

// --- Buffer de direção para suavizar leitura ---
#define BUFFER_SIZE 3
int bufferDirecao[BUFFER_SIZE];
int bufferIndex = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(100000);

  if (!bno.begin()) {
    Serial.println("Erro ao inicializar BNO055!");
    while (1);
  }
  bno.setExtCrystalUse(true);

  InfraredSeeker::Initialize();

  pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
  pinMode(M2_IN1, OUTPUT); pinMode(M2_IN2, OUTPUT);
  pinMode(M3_IN1, OUTPUT); pinMode(M3_IN2, OUTPUT);

  // Inicializa buffer com 5 (centro)
  for (int i=0; i<BUFFER_SIZE; i++) bufferDirecao[i] = 5;

  Serial.println("Sistema iniciado. Seguindo a bola...");
}

void parar() {
  analogWrite(M1_IN1, 0); analogWrite(M1_IN2, 0);
  analogWrite(M2_IN1, 0); analogWrite(M2_IN2, 0);
  analogWrite(M3_IN1, 0); analogWrite(M3_IN2, 0);
}

void frenteComPID(int erro, int velocidade) {
  integral += erro;
  float derivada = erro - erroAnterior;
  erroAnterior = erro;

  float correcao = Kp*erro + Ki*integral + Kd*derivada;
  correcao = constrain(correcao, -MAX_CORRECAO, MAX_CORRECAO);

  int velEsq = constrain(velocidade - correcao, 0, 255);
  int velDir = constrain(velocidade + correcao, 0, 255);

  analogWrite(M1_IN1, velEsq); analogWrite(M1_IN2, 0);
  analogWrite(M2_IN1, 0); analogWrite(M2_IN2, 0);
  analogWrite(M3_IN1, velDir); analogWrite(M3_IN2, 0);
}

void girarEsquerda(int vel) {
  analogWrite(M1_IN1, 0); analogWrite(M1_IN2, vel);
  analogWrite(M2_IN1, 0); analogWrite(M2_IN2, vel);
  analogWrite(M3_IN1, vel); analogWrite(M3_IN2, 0);
}

void girarDireita(int vel) {
  analogWrite(M1_IN1, vel); analogWrite(M1_IN2, 0);
  analogWrite(M2_IN1, vel); analogWrite(M2_IN2, 0);
  analogWrite(M3_IN1, 0); analogWrite(M3_IN2, vel);
}

// --- PID adaptativo ---
float ganhoProporcional(int intensidade) {
  return map(intensidade, 0, 255, 1, 3);
}

float ganhoIntegral(int intensidade) {
  return map(intensidade, 0, 255, 0, 0.05);
}

float ganhoDerivativo(int intensidade) {
  return map(intensidade, 0, 255, 0.1, 1.0);
}

// --- Suaviza leitura usando média móvel ---
int mediaDirecao(int nova) {
  bufferDirecao[bufferIndex] = nova;
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;

  int soma = 0;
  for (int i=0; i<BUFFER_SIZE; i++) soma += bufferDirecao[i];
  return soma / BUFFER_SIZE;
}

void loop() {
  // --- Leitura IR ---
  InfraredResult bola = InfraredSeeker::ReadAC();
  direcao = bola.Direction;
  intensidade = bola.Strength;

  // --- Leitura bússola ---
  sensors_event_t event;
  bno.getEvent(&event);
  float heading = event.orientation.x;

  // --- PID adaptativo ---
  Kp = ganhoProporcional(intensidade);
  Ki = ganhoIntegral(intensidade);
  Kd = ganhoDerivativo(intensidade);

  // --- Média móvel da direção ---
  int dirSuave = mediaDirecao(direcao);

  // --- Debug completo ---
  Serial.print("Bússola: "); Serial.print(heading);
  Serial.print("° | Direção: "); Serial.print(direcao);
  Serial.print(" | Dir Suave: "); Serial.print(dirSuave);
  Serial.print(" | Intensidade: "); Serial.print(intensidade);
  Serial.print(" | Kp: "); Serial.print(Kp);
  Serial.print(" | Ki: "); Serial.print(Ki);
  Serial.print(" | Kd: "); Serial.println(Kd);

  // --- Verifica se a bola está visível ---
  if (direcao == 0 || intensidade < 5) {
    parar();
    integral = 0;
    erroAnterior = 0;
    return;
  }

  // --- Ajuste de velocidade ---
  if (intensidade >= 200) {
    velocidadeFrente = 120;
    velocidadeGiro = 100;
  } else {
    velocidadeFrente = 255;
    velocidadeGiro = 200;
  }

  // --- Movimento ---
  if (dirSuave >= 4 && dirSuave <= 6) {
    int erro = 5 - dirSuave;
    frenteComPID(erro, velocidadeFrente);
  } else if (dirSuave < 4) {
    girarEsquerda(velocidadeGiro);
    integral = 0; erroAnterior = 0;
  } else if (dirSuave > 6) {
    girarDireita(velocidadeGiro);
    integral = 0; erroAnterior = 0;
  }

  delay(50);
}
