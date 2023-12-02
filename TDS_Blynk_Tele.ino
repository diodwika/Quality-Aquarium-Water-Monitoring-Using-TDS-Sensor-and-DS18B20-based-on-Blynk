// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID "TMPL6ogMBe7M8"
#define BLYNK_TEMPLATE_NAME "TDS Monitoring"
#define BLYNK_AUTH_TOKEN "8XuHW6-X1XByeqlnf27GKYs-P-Dff2PX"

// Define your Telegram bot token and chat ID
#define TELEGRAM_BOT_TOKEN "6815609273:AAG0ie5RLqsGQku48SMxwhxvDEE5sRXsLJE"
#define TELEGRAM_CHAT_ID "1916454355"

// Uncomment this line if you're using a DS18B20 temperature sensor
#define DS18B20_SENSOR_PIN 18  // Ganti dengan pin GPIO yang Anda gunakan
#define TdsSensorPin 34  // Ganti dengan pin GPIO yang Anda gunakan

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <HTTPClient.h>

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "KAY/O";
char pass[] = "seblakayam";

BlynkTimer timer;

float vRef = 5.0;  // Tegangan referensi ADC (V)

const int ledPin1 = 25;  // Pin untuk LED 1
const int ledPin2 = 26;  // Pin untuk LED 2

// Sensor DS18B20
OneWire oneWire(DS18B20_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
float temperature = 0.0;
float factor = 0.0;

void sendTelegramNotification(String message) {
  HTTPClient http;
  String url = "https://api.telegram.org/bot" + String(TELEGRAM_BOT_TOKEN) +
               "/sendMessage?chat_id=" + String(TELEGRAM_CHAT_ID) +
               "&text=" + message;

  http.begin(url);

  int httpResponseCode = http.GET();
  if (httpResponseCode > 0) {
    Serial.print("Telegram notification sent successfully. Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error sending Telegram notification. Response code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

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
    sendTelegramNotification("MULEH BANG, IWAKMU MEH MATI");
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
  pinMode(ledPin1, OUTPUT);  // Set pin 25 as output for future use
  pinMode(ledPin2, OUTPUT);
  pinMode(TdsSensorPin, INPUT);
  pinMode(DS18B20_SENSOR_PIN, INPUT);
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
