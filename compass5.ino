#include <Wire.h>

// Compass I2C address
#define COMPASS_ADDRESS 0x01

// Motores
const int M1_IN1 = 7; // Esquerda
const int M1_IN2 = 6;
const int M2_IN1 = 5; // Trás
const int M2_IN2 = 4;
const int M3_IN1 = 3; // Direita
const int M3_IN2 = 2;

// Velocidade padrão dos motores
const int vel_M1 = 200;  // Motor 1 (esquerda)
const int vel_M2 = 255;  // Motor 2 (trás)
const int vel_M3 = 255;  // Motor 3 (direita)

float compassOffset = 0;  // Referência inicial
float Kp = 4.0;
float Ki = 0.05;
float Kd = 1.0;

float erroAnterior = 0;
float integral = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();

  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);
  pinMode(M2_IN1, OUTPUT);
  pinMode(M2_IN2, OUTPUT);
  pinMode(M3_IN1, OUTPUT);
  pinMode(M3_IN2, OUTPUT);

  delay(1000); // Tempo para estabilizar
  compassOffset = lerBussola(); // Define o ponto inicial como frente
  pararMotores(); // Inicia parado
}

void loop() {
  float anguloAtual = lerBussola();
  float erro = calcularErro(compassOffset, anguloAtual);

  float correcao = calcularPID(erro);

  if (abs(erro) < 5) {
    // Quando estiver alinhado, anda para frente (sem motor 2)
    analogWrite(M1_IN1, vel_M1);
    analogWrite(M1_IN2, 0);
    
    analogWrite(M2_IN1, 0);
    analogWrite(M2_IN2, 0);
    
    analogWrite(M3_IN1, vel_M3);
    analogWrite(M3_IN2, 0);
  } else if (erro > 0) {
    // Gira sentido horário
    analogWrite(M1_IN1, 0);
    analogWrite(M1_IN2, vel_M1);
    
    analogWrite(M2_IN1, vel_M2);
    analogWrite(M2_IN2, 0);
    
    analogWrite(M3_IN1, 0);
    analogWrite(M3_IN2, vel_M3);
  } else {
    // Gira sentido anti-horário
    analogWrite(M1_IN1, vel_M1);
    analogWrite(M1_IN2, 0);
    
    analogWrite(M2_IN1, 0);
    analogWrite(M2_IN2, vel_M2);
    
    analogWrite(M3_IN1, vel_M3);
    analogWrite(M3_IN2, 0);
  }

  delay(50);
}

float calcularPID(float erro) {
  integral += erro;
  float derivada = erro - erroAnterior;
  erroAnterior = erro;
  return Kp * erro + Ki * integral + Kd * derivada;
}

float calcularErro(float alvo, float atual) {
  float erro = alvo - atual;
  if (erro > 180) erro -= 360;
  if (erro < -180) erro += 360;
  return erro;
}

float lerBussola() {
  Wire.beginTransmission(COMPASS_ADDRESS);
  Wire.write(0x42); // Endereço do registrador de ângulo
  Wire.endTransmission();
  Wire.requestFrom(COMPASS_ADDRESS, 2);
  if (Wire.available() >= 2) {
    byte highByte = Wire.read();
    byte lowByte = Wire.read();
    int heading = (highByte << 8) + lowByte;
    return heading / 10.0; // Valor em graus
  }
  return 0;
}

void pararMotores() {
  analogWrite(M1_IN1, 0);
  analogWrite(M1_IN2, 0);
  analogWrite(M2_IN1, 0);
  analogWrite(M2_IN2, 0);
  analogWrite(M3_IN1, 0);
  analogWrite(M3_IN2, 0);
}
