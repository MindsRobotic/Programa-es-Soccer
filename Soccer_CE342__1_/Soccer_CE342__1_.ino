#include <Wire.h>
#include <HTInfraredSeeker.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_Sensor.h>

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

// --- Solenoide ---
#define SOL_ENB 13
#define SOL_IN3 11
#define SOL_IN4 12

// --- IR ---
int ballDirection;
int ballIntens;

// --- PID (mesmo ganho para IR e bússola, como você vinha usando) ---
float kp = 1;
float ki = 0;
float kd = 0;
float erroAnterior = 0;
float proporcional, integral, derivativo = 0;

// --- Velocidade ---
int velocidade = 255;

// --- Bússola ---
Adafruit_BNO055 bno = Adafruit_BNO055();

// --- Média móvel IR ---
#define BUFFER_SIZE 5
int bufferDirecao[BUFFER_SIZE] = {5,5,5,5,5};
int bufferIndex = 0;

int mediaDirecao(int nova) {
  bufferDirecao[bufferIndex] = nova;
  bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
  int soma = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) soma += bufferDirecao[i];
  int media = soma / BUFFER_SIZE;
  return constrain(media, 0, 9);
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Inicializa IR
  InfraredSeeker::Initialize();

  // Inicializa BNO055
  if (!bno.begin()) {
    Serial.println("Erro ao iniciar BNO055!");
    while (1);
  }
  bno.setExtCrystalUse(true);

  // Motores
  pinMode(M1_ENA, OUTPUT); pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
  pinMode(M2_ENB, OUTPUT); pinMode(M2_IN3, OUTPUT); pinMode(M2_IN4, OUTPUT);
  pinMode(M3_ENA, OUTPUT); pinMode(M3_IN1, OUTPUT); pinMode(M3_IN2, OUTPUT);

  // Solenoide
  pinMode(SOL_ENB, OUTPUT); pinMode(SOL_IN3, OUTPUT); pinMode(SOL_IN4, OUTPUT);

  Serial.println("Sistema iniciado. Seguindo a bola...");
}

// ------------------- Funções -------------------
void parar() {
  analogWrite(M1_ENA, 0);
  analogWrite(M2_ENB, 0);
  analogWrite(M3_ENA, 0);
}


void Bussola(float erroBussola, int velocidade) {
  float correcao_Bussola = kp * erroBussola; // Limitando correção para ficar suave e ñ ir duma vez

  int velEsq, velDir, velTras = 0;

   if (erroBussola == 0) {
    velEsq  = constrain(velocidade - correcao_Bussola, 0, 255);
    velDir  = constrain(velocidade + correcao_Bussola, 0, 255) * -1;
    velTras = constrain(correcao_Bussola, 0, 255);
  }
  if (erroBussola < 3) {
    velEsq  = constrain(velocidade - correcao_Bussola, 0, 255) * -1;
    velDir  = constrain(velocidade + correcao_Bussola, 0, 255) * -1;
    velTras = constrain(correcao_Bussola, 0, 255) * -1;
  }
  if (erroBussola > 3) {
    velEsq  = constrain(velocidade - correcao_Bussola, 0, 255);
    velDir  = constrain(velocidade + correcao_Bussola, 0, 255);
    velTras = constrain(correcao_Bussola, 0, 255);
  }

  Deslocar(velEsq, velDir, velTras);
}
void PID(float erro, int velocidade) {
  proporcional = kp * erro;
  integral     = ki * erro;
  derivativo   = kd * (erro - erroAnterior);
  erroAnterior = erro;

  float correcao = proporcional + integral + derivativo;
  int velEsq, velDir, velTras = 0;

  if (erro == 0) {
    velEsq  = constrain(velocidade - correcao, 0, 255);
    velDir  = constrain(velocidade + correcao, 0, 255) * -1;
    velTras = constrain(correcao, 0, 255);
  }
  if (erro < 0) {
    velEsq  = constrain(velocidade - correcao, 0, 255) * -1;
    velDir  = constrain(velocidade + correcao, 0, 255) * -1;
    velTras = constrain(correcao, 0, 255) * -1;
  }
  if (erro > 0) {
    velEsq  = constrain(velocidade - correcao, 0, 255);
    velDir  = constrain(velocidade + correcao, 0, 255);
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
  // IR
  InfraredResult InfraredBall = InfraredSeeker::ReadAC(); // Realiza a leitura do sensor IR Seeker
  ballDirection = InfraredBall.Direction; // Armazena a direção na variável
  ballIntens = InfraredBall.Strength; // Armazena a direção na variável

  int erro = 5 - ballDirection;  // centro do IR é 5

  // Bússola
  sensors_event_t event;
  bno.getEvent(&event);
  float compassAngle = event.orientation.x; // 0° é o gol adversário
  // erro da bússola para 0°, normalizado para -180..180 para girar pelo menor lado
  float erroBussola = -compassAngle;
  if (erroBussola > 180)  erroBussola -= 360;
  if (erroBussola < -180) erroBussola += 360;

  // Escolha da estratégia
  if (ballIntens > 220) {     // bola perto: usa bússola para apontar pro gol
    Bussola(erroBussola, velocidade);
  } else {                    // bola longe: segue a bola pelo IR
    PID(erro, velocidade);
  }

  // Debug
  Serial.print("Dir IR: "); Serial.print(ballDirection);
  Serial.print(" | Intens: "); Serial.print(ballIntens);
  Serial.print(" | Compass: "); Serial.println(compassAngle);

  delay(50);
}
