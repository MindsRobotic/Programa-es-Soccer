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

// --- Velocidade ---
int velocidadeFrente = 150;
int velocidadeGiro = 100;

// --- Bússola ---
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);
float headingZero = 0, angleZ = 0;

// --- Ultrassom ---
#define TRIG_PIN 12
#define ECHO_PIN 13
int distanciaMinima = 15; // cm até a parede

// --- Refletância traseira ---
#define REF_TRA_A A2
#define REF_TRA_B A3

bool emRecuo = false;

// ------------------- Funções -------------------
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

long medirDistancia() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duracao = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  long distancia = duracao * 0.034 / 2;
  return distancia;
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
  headingZero = event.orientation.x; // gol adversário

  pinMode(M1_ENA, OUTPUT); pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
  pinMode(M2_ENB, OUTPUT); pinMode(M2_IN3, OUTPUT); pinMode(M2_IN4, OUTPUT);
  pinMode(M3_ENA, OUTPUT); pinMode(M3_IN1, OUTPUT); pinMode(M3_IN2, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(REF_TRA_A, INPUT_PULLUP);
  pinMode(REF_TRA_B, INPUT_PULLUP);
}

// ------------------- Loop -------------------
void loop() {
  // --- Leitura IR ---
  InfraredResult InfraredBall = InfraredSeeker::ReadAC();
  ballDirection = InfraredBall.Direction;

  // --- Leitura refletância traseira ---
  bool linhaDetectada = (digitalRead(REF_TRA_A) == LOW) && (digitalRead(REF_TRA_B) == LOW);

  // --- Se linha detectada, ativa modo recuo ---
  if (linhaDetectada) {
    emRecuo = true;
  }

  // --- Modo recuo ---
  if (emRecuo) {
    if (medirDistancia() > distanciaMinima) {
      // Continua recuando até chegar na parede
      Deslocar(-100, -100, 0); // ré suave
    } else {
      // Parou na distância segura
      Deslocar(0, 0, 0);
      emRecuo = false; // volta ao modo normal
    }
    return; // não executa o resto do loop enquanto recua
  }

  // --- Seguir a bola normalmente ---
  if (ballDirection < 5) {
    // Bola à esquerda → girar para esquerda
    Deslocar(-velocidadeGiro, velocidadeGiro, 0);
  } 
  else if (ballDirection > 5) {
    // Bola à direita → girar para direita
    Deslocar(velocidadeGiro, -velocidadeGiro, 0);
  } 
  else {
    // Bola à frente → avançar
    Deslocar(velocidadeFrente, velocidadeFrente, 0);
  }
}
