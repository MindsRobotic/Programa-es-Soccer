#define M1_ENA 2
#define M1_IN1 3
#define M1_IN2 4
#define M2_ENB 7
#define M2_IN3 6
#define M2_IN4 5
#define M3_ENA 8
#define M3_IN1 9
#define M3_IN2 10


void setup() {
  pinMode(M1_ENA, OUTPUT); pinMode(M1_IN1, OUTPUT); pinMode(M1_IN2, OUTPUT);
  pinMode(M2_ENB, OUTPUT); pinMode(M2_IN3, OUTPUT); pinMode(M2_IN4, OUTPUT);
  pinMode(M3_ENA, OUTPUT); pinMode(M3_IN1, OUTPUT); pinMode(M3_IN2, OUTPUT);
}

void loop() {
  // ---- Direita ----
  digitalWrite(M2_IN3, LOW); digitalWrite(M2_IN4, HIGH);   // M1 frente
  analogWrite(M2_ENB, 255);

}
