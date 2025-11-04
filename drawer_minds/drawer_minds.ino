#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// --- Configurações do DHT ---
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// --- Configurações do OLED ---
#define SCREEN_WIDTH 128 // largura do display em pixels
#define SCREEN_HEIGHT 64 // altura do display em pixels
#define OLED_RESET -1    // alguns módulos não usam reset físico
#define SCREEN_ADDRESS 0x3C // endereço padrão do OLED (pode ser 0x3C ou 0x3D)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  Serial.begin(9600);
  dht.begin();

  // Inicializa o display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Falha ao inicializar o OLED!"));
    for (;;); // trava aqui se não encontrar o display
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println("Iniciando...");
  display.display();
  delay(2000);
}

void loop() {
  delay(2000);

  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature();

  if (isnan(umidade)) {
    display.clearDisplay();
    display.setCursor(0, 20);
    display.println("Falha leitura!");
    display.display();
    Serial.println("Falha ao ler o sensor DHT!");
    return;
  }

  // Mostrar no Serial Monitor
  Serial.print("Umidade: ");
  Serial.print(umidade);
  Serial.println("%");
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  // Mostrar no OLED
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Sensor DHT11");
  display.setCursor(0, 20);
  display.print("Umidade: ");
  display.print(umidade);
  display.println("%");
  display.setCursor(0, 35);
  display.print("Temp: ");
  display.print(temperatura);
  display.println(" C");

  // Condição de manuseio
  float limitarUmidade = 60.0;
  display.setCursor(0, 50);
  if (umidade <= limitarUmidade) {
    display.println("Pronto p/ manusear!");
  }

  display.display();
}
