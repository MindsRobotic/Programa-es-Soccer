//Ponte 1

// Motor 1
#define M1_ENA 2
#define M1_IN1 3
#define M1_IN2 4

// Motor 2
#define M2_ENB 7
#define M2_IN3 5
#define M2_IN4 6 //.

//Ponte 2

// Motor 3
#define M3_ENA 8
#define M3_IN1 9
#define M3_IN2 10

// Motor 4
#define M4_ENB 13
#define M4_IN3 11
#define M4_IN4 12 //.


void setup() {
  // Motor 1
  pinMode(M1_ENA, OUTPUT);
  pinMode(M1_IN1, OUTPUT);
  pinMode(M1_IN2, OUTPUT);

  // Motor 2
  pinMode(M2_ENB, OUTPUT);
  pinMode(M2_IN3, OUTPUT);
  pinMode(M2_IN4, OUTPUT);

  // Motor 3
  pinMode(M3_ENA, OUTPUT);
  pinMode(M3_IN1, OUTPUT);
  pinMode(M3_IN2, OUTPUT);

  // Motor 4
  pinMode(M4_ENB, OUTPUT);
  pinMode(M4_IN3, OUTPUT);
  pinMode(M4_IN4, OUTPUT);
}

 //frente
void moverFrente(int vel) {

  // Motor 1
  digitalWrite(M1_IN1, HIGH);
  digitalWrite(M1_IN2, LOW);
  analogWrite(M1_ENA, vel);

  // Motor 2
  digitalWrite(M2_IN3, HIGH);
  digitalWrite(M2_IN4, LOW);
  analogWrite(M2_ENB, vel);

  // Motor 3
  digitalWrite(M3_IN1, HIGH);
  digitalWrite(M3_IN2, LOW);
  analogWrite(M3_ENA, vel);

  // Motor 4
  digitalWrite(M4_IN3, HIGH);
  digitalWrite(M4_IN4, LOW);
  analogWrite(M4_ENB, vel);
}


  //parar
void pararMotores() {
  analogWrite(M1_ENA, 0);
  analogWrite(M2_ENB, 0);
  analogWrite(M3_ENA, 0);
  analogWrite(M4_ENB, 0);
}

void loop() {
  moverFrente(200);
  delay(200 0);

  pararMotores();
  delay(1000);
}
