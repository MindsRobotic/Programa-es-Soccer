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

// --- Bússola ---
#define BNO_ADDRESS 0x28
Adafruit_BNO055 bno = Adafruit_BNO055(55, BNO_ADDRESS);

// --- Variáveis do IR ---
int direcao;
int intensidade;

// --- Velocidade base ---
int velocidadeFrente = 255;
int velocidadeGiro = 200;

// --- PID ---
float Kp = 1.0;
float Ki = 0.5;
float Kd = 1.0;
float erroAnterior = 0;
float integral = 0;
const float MAX_CORRECAO = 50;

// --- Média móvel para suavizar direção ---
#define BUFFER_SIZE 5
int bufferDirecao[BUFFER_SIZE] = {5,5,5};
int bufferIndex = 0;

int mediaDirecao(int nova) {
  bufferDirecao[bufferIndex] = nova;
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
  int soma = 0;
  for (int i=0; i<BUFFER_SIZE; i++) soma += bufferDirecao[i];
  return soma / BUFFER_SIZE;
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Inicializa BNO055
  if (!bno.begin()) {
    Serial.println("Erro ao inicializar BNO055!");
    while (1);
  }
  bno.setExtCrystalUse(true);

  // Inicializa IR Seeker
  InfraredSeeker::Initialize();

  // Configura pinos motores
  pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
  pinMode(M2_IN1, OUTPUT); pinMode(M2_IN2, OUTPUT);
  pinMode(M3_IN1, OUTPUT); pinMode(M3_IN2, OUTPUT);

  Serial.println("Sistema iniciado. Seguindo a bola...");
}

// --- Funções de movimento ---
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

  analogWrite(M1_IN1, 0); analogWrite(M1_IN2, velEsq);
  analogWrite(M2_IN1, 0);     analogWrite(M2_IN2, 0);
  analogWrite(M3_IN1, 0); analogWrite(M3_IN2, velDir);
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

// ------------------- Loop principal -------------------
void loop() {
  // --- Lê bola ---
  InfraredResult bola = InfraredSeeker::ReadAC();
  int leituraCorrigida = 10 - bola.Direction; // Corrige inversão esquerda/direita
  direcao = mediaDirecao(leituraCorrigida);  // Suaviza leitura
  intensidade = bola.Strength;

  // --- Lê bússola ---
  sensors_event_t event;
  bno.getEvent(&event);
  float heading = event.orientation.x;

  // --- Mostra valores ---
  Serial.print("Bússola: "); Serial.print(heading);
  Serial.print("° | Direção: "); Serial.print(direcao);
  Serial.print(" | Intensidade: "); Serial.println(intensidade);

  // --- Verifica se a bola está visível ---
  if (direcao == 0 || intensidade < 5) {
    parar();
    integral = 0;      
    erroAnterior = 0;
    return;
  }

  // Ajusta velocidade de acordo com intensidade
  if (intensidade >= 200) {
    velocidadeFrente = 120;
    velocidadeGiro = 100;
  } else {
    velocidadeFrente = 255;
    velocidadeGiro = 200;
  }

  // --- Controle de movimento com PID ---
  if (direcao >= 4 && direcao <= 6) {
    int erro = 5 - direcao; // Centro é 5
    frenteComPID(erro, velocidadeFrente);
  } else if (direcao > 5) {
    girarEsquerda(velocidadeGiro);
    integral = 0; erroAnterior = 0;
  } else if (direcao < 5 ) {
    girarDireita(velocidadeGiro);
    integral = 0; erroAnterior = 0;
  }

  delay(50);
}
