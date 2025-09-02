#include <Wire.h>
#include <HTInfraredSeeker.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_Sensor.h>

// --- Motores ---
#define M1_ENA 2
#define M1_IN1 3
#define M1_IN2 4
#define M2_ENB 7
#define M2_IN3 5
#define M2_IN4 6
#define M3_ENA 8
#define M3_IN1 9
#define M3_IN2 10

// --- IR ---
int ballDirection;
int ballIntens;

// --- PID IR ---
float kp = 1, ki = 0, kd = 0;
float erroAnteriorIR = 0;

// --- PID Bússola ---
float kp_compass = 0.5, ki_compass = 0, kd_compass = 0.02;
float erroAnteriorCompass = 0, integralCompass = 0;

// --- Velocidade ---
int velocidade = 200;
int velocidadeAtaque = 255;
int limiarBolaPerto = 220;

// --- Pesos fixos ---
float pesoIR_perto = 0.3;
float pesoBussola_longe = 0.5;

// --- Média móvel IR ---
#define BUFFER_SIZE 5
int bufferDirecao[BUFFER_SIZE] = {5,5,5,5,5};
int bufferIndex = 0;

int mediaDirecao(int nova) {
  bufferDirecao[bufferIndex] = nova;
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
  int soma = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) soma += bufferDirecao[i];
  return constrain(soma / BUFFER_SIZE, 0, 9);
}

// --- Bússola ---
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
float headingZero = 0, angleZ = 0;

// ------------------- Funções -------------------
void PID_IR(float erro, int velocidade, float correcaoCompass) {
  float proporcional = kp * erro;
  float integral     = ki * erro;
  float derivativo   = kd * (erro - erroAnteriorIR);
  erroAnteriorIR = erro;

  float correcao = proporcional + integral + derivativo + correcaoCompass;

  int velEsq, velDir, velTras = 0;

  if (erro == 0) {
    velEsq  = constrain(velocidade - correcao, 0, 255);
    velDir  = -constrain(velocidade + correcao, 0, 255);
    velTras = constrain(correcao, 0, 255);
  } else if (erro < 0) {
    velEsq  = -constrain(velocidade - correcao, 0, 255);
    velDir  = -constrain(velocidade + correcao, 0, 255);
    velTras = -constrain(correcao, 0, 255);
  } else {
    velEsq  = constrain(velocidade - correcao, 0, 255);
    velDir  = constrain(velocidade + correcao, 0, 255);
    velTras = constrain(correcao, 0, 255);
  }

  Deslocar(velEsq, velDir, velTras);
}

void Deslocar(int motoresquerdo, int motordireito, int motortras){
  auto drive = [](int pwm, int en, int in1, int in2) {
    if (pwm >= 0) {
      digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
      analogWrite(en, constrain(pwm, 0, 255));
    } else {
      pwm = -pwm;
      digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
      analogWrite(en, constrain(pwm, 0, 255));
    }
  };
  drive(motoresquerdo, M1_ENA, M1_IN1, M1_IN2);
  drive(motordireito, M2_ENB, M2_IN3, M2_IN4);
  drive(motortras, M3_ENA, M3_IN1, M3_IN2);
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);
  Wire.begin();

  InfraredSeeker::Initialize();

  if (!bno.begin()) {
    Serial.println("Erro ao iniciar BNO055!");
    while (1);
  }
  delay(1000);
  bno.setExtCrystalUse(true);

  sensors_event_t event;
  bno.getEvent(&event);
  headingZero = event.orientation.x; // frente no momento da ligação

  pinMode(M1_ENA, OUTPUT); pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
  pinMode(M2_ENB, OUTPUT); pinMode(M2_IN3, OUTPUT); pinMode(M2_IN4, OUTPUT);
  pinMode(M3_ENA, OUTPUT); pinMode(M3_IN1, OUTPUT); pinMode(M3_IN2, OUTPUT);
}

// ------------------- Loop -------------------
void loop() {
  // IR
  InfraredResult InfraredBall = InfraredSeeker::ReadAC();
  ballDirection = mediaDirecao(InfraredBall.Direction);
  ballIntens = InfraredBall.Strength;

  // Bússola
  sensors_event_t event;
  bno.getEvent(&event);
  angleZ = event.orientation.x - headingZero;
  if (angleZ > 180) angleZ -= 360;
  if (angleZ < -180) angleZ += 360;

  static float erroCompassFiltrado = 0;
  float erroCompass = 0 - angleZ;
  erroCompassFiltrado = erroCompassFiltrado * 0.7 + erroCompass * 0.3;

  // PID da bússola + IR
  if (abs(erroCompassFiltrado) < 3) erroCompassFiltrado = 0;
  integralCompass += erroCompassFiltrado;
  float derivCompass = erroCompassFiltrado - erroAnteriorCompass;
  erroAnteriorCompass = erroCompassFiltrado;

  float correcaoCompass = (kp_compass * erroCompassFiltrado +
                           ki_compass * integralCompass +
                           kd_compass * derivCompass);

  // Controle adaptativo fixo
  if (ballIntens >= limiarBolaPerto) {
    int erroIR = (5 - ballDirection) * pesoIR_perto;
    PID_IR(erroIR, velocidade, correcaoCompass);
  } else {
    int erroIR = 5 - ballDirection;
    PID_IR(erroIR, velocidade, correcaoCompass * pesoBussola_longe);
  }
}
