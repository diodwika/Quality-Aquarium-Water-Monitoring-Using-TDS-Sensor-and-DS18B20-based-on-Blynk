// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "your template ID"
#define BLYNK_TEMPLATE_NAME "your template name"
#define BLYNK_AUTH_TOKEN "your device token"

// Uncomment this line if you're using a DS18B20 temperature sensor
#define DS18B20_SENSOR_PIN 18  // Ganti dengan pin GPIO yang Anda gunakan

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "your wifi ssid";
char pass[] = "your wifi passwaord";

BlynkTimer timer;

#define TdsSensorPin 34  // Ganti dengan pin GPIO yang Anda gunakan
float vRef = 5.0;  // Tegangan referensi ADC (V)

const int ledPin1 = 25;  // Pin untuk LED 1
const int ledPin2 = 26;  // Pin untuk LED 2

// Sensor DS18B20
OneWire oneWire(DS18B20_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
float temperature = 0.0;
float factor = 0.0;

void readAndSendTdsData() {
  int rawValue = analogRead(TdsSensorPin);  // Baca nilai analog dari sensor TDS
  
  // Konversi nilai analog ke tegangan (V)
  float voltage = rawValue * (vRef / 1023.0);
  
  // Konversi tegangan ke nilai TDS (contoh konversi sederhana)
  float tdsValue = voltage * 100;  // Contoh konversi sederhana (harap sesuaikan sesuai karakteristik sensor)
  
  Serial.print("TDS Value (ppm): ");
  Serial.println(tdsValue, 2);
  
  Blynk.virtualWrite(V0, tdsValue);  // Mengirim nilai TDS ke server Blynk

  // Mengirim notifikasi ke V1 berdasarkan nilai TDS
  if (tdsValue > 600) {
    Blynk.virtualWrite(V1, "Air Kotor");
  } else {
    Blynk.virtualWrite(V1, "Air Bersih");
  }
}

void readAndSendTemperature() {
  sensors.requestTemperatures();  // Mengirim perintah untuk membaca suhu
  temperature = sensors.getTempCByIndex(0) - 8;  // Mendapatkan suhu dalam derajat Celsius

  Serial.print("Temperature: ");
  Serial.println(temperature, 2);

  Blynk.virtualWrite(V2, temperature);  // Mengirim nilai suhu ke server Blynk

  if (temperature < 25) {
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
    Blynk.virtualWrite(V3, HIGH);  // Mengirim status LED 1 ke V3
    Blynk.virtualWrite(V4, LOW);   // Mengirim status LED 2 ke V4
  } else if (temperature > 30) {
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, HIGH);
    Blynk.virtualWrite(V3, LOW);   // Mengirim status LED 1 ke V3
    Blynk.virtualWrite(V4, HIGH);  // Mengirim status LED 2 ke V4
  } else {
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, LOW);
    Blynk.virtualWrite(V3, LOW);   // Mengirim status LED 1 ke V3
    Blynk.virtualWrite(V4, LOW);   // Mengirim status LED 2 ke V4
  }
}

void setup() {
  pinMode(25, OUTPUT);  // Set pin 25 as output for future use
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  // Setup timer untuk pemanggilan fungsi setiap detik
  timer.setInterval(1000L, readAndSendTdsData);
  timer.setInterval(1000L, readAndSendTemperature);  // Ganti interval sesuai kebutuhan
}

void loop() {
  Blynk.run();
  timer.run();
}
