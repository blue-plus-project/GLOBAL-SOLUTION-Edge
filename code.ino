#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <Wire.h>
#include <EEPROM.h>
#include "DHT.h"

#define LOG_OPTION 1
#define SERIAL_OPTION 1
#define UTC_OFFSET -3

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 RTC;

const int maxRecords = 100;
const int recordSize = 8;
int startAddress = 0;
int endAddress = maxRecords * recordSize;
int currentAddress = 0;

// Configurações da média móvel
const int N = 10; // Número de leituras para a média móvel
float temperatureReadings[N]; // Array para armazenar as últimas N leituras
int currentReadingIndex = 0; // Índice atual no array de leituras
int numReadings = 0; // Número atual de leituras (até atingir N)

// Função para calcular a média móvel
float calculateMovingAverage() {
  float sum = 0;
  for (int i = 0; i < numReadings; i++) {
    sum += temperatureReadings[i];
  }
  return sum / numReadings;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  dht.begin();
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  RTC.begin();
  RTC.adjust(DateTime(F(__DATE__), F(__TIME__)));
  EEPROM.begin();

  for (int i = 0; i < N; i++) {
    temperatureReadings[i] = 0;
  }
}

void loop() {
  DateTime now = RTC.now(); // Obtém a data e hora atual do RTC

  int offsetSeconds = UTC_OFFSET * 3600;
  now = now.unixtime() + offsetSeconds;

  DateTime adjustedTime = DateTime(now);

  float temperature = dht.readTemperature();
  if (isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  temperatureReadings[currentReadingIndex] = temperature;
  currentReadingIndex = (currentReadingIndex + 1) % N;
  if (numReadings < N) {
    numReadings++;
  }

  // Calcula a média móvel
  float averageTemperature = calculateMovingAverage();

  lcd.setCursor(0, 0);
  lcd.print(adjustedTime.day() < 10 ? "0" : "");
  lcd.print(adjustedTime.day());
  lcd.print("/");
  lcd.print(adjustedTime.month() < 10 ? "0" : "");
  lcd.print(adjustedTime.month());
  lcd.print("/");
  lcd.print(adjustedTime.year() % 2000);

  lcd.setCursor(10, 0);
  lcd.print("Temp:");

  lcd.setCursor(0, 1);
  lcd.print(adjustedTime.hour() < 10 ? "0" : "");
  lcd.print(adjustedTime.hour());
  lcd.print(":");
  lcd.print(adjustedTime.minute() < 10 ? "0" : "");
  lcd.print(adjustedTime.minute());
  lcd.print(":");
  lcd.print(adjustedTime.second() < 10 ? "0" : "");
  lcd.print(adjustedTime.second());

  lcd.setCursor(10, 1);
  lcd.print(averageTemperature, 1); // Exibir a média móvel em vez da leitura instantânea
  lcd.print((char)223); // Símbolo de grau
  lcd.print("C");

  if (SERIAL_OPTION) {
    Serial.print(adjustedTime.day());
    Serial.print("/");
    Serial.print(adjustedTime.month());
    Serial.print("/");
    Serial.print(adjustedTime.year());
    Serial.print(" ");
    Serial.print(adjustedTime.hour() < 10 ? "0" : "");
    Serial.print(adjustedTime.hour());
    Serial.print(":");
    Serial.print(adjustedTime.minute() < 10 ? "0" : "");
    Serial.print(adjustedTime.minute());
    Serial.print(":");
    Serial.print(adjustedTime.second() < 10 ? "0" : "");
    Serial.print(adjustedTime.second());
    Serial.print(" - Temp: ");
    Serial.print(temperature, 1);
    Serial.println(" C");
  }

  delay(1000);
}
