#include <ThingerESP32.h>

#define USERNAME "dwikadio"
#define DEVICE_ID "ESP32_TDS"
#define DEVICE_CREDENTIAL "diodwika_TDS" // ganti dengan credential device Thinger.io Anda
#define SSID "ARMTECH"
#define SSID_PASSWORD "inverse6"

#define TdsSensorPin 27
#define VREF 5.0      // analog reference voltage(Volt) of the ADC
#define SCOUNT  30           // sum of sample point
int analogBuffer[SCOUNT];    // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, temperature = 25;

ThingerESP32 thing(USERNAME, DEVICE_ID, DEVICE_CREDENTIAL);

void setup()
{
    Serial.begin(115200);
    pinMode(TdsSensorPin, INPUT);

    thing.add_wifi(SSID, SSID_PASSWORD);

    // Set the resource for sending ultrasonic distance to Thinger.io
    thing["TDS_Value"] >> [](pson& out){
      out = readTdsSensor();
    };
}

void loop() {
  thing.handle();
  delay(1000); // Update every 1 second
}

long readTdsSensor() {
    static unsigned long analogSampleTimepoint = millis();
    if (millis() - analogSampleTimepoint > 40U) //every 40 milliseconds, read the analog value from the ADC
    {
        analogSampleTimepoint = millis();
        analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin); //read the analog value and store into the buffer
        analogBufferIndex++;
        if (analogBufferIndex == SCOUNT)
            analogBufferIndex = 0;
    }

    static unsigned long printTimepoint = millis();
    if (millis() - printTimepoint > 800U)
    {
        printTimepoint = millis();
        for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
            analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
        averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 4096.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
        float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);                 // temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
        float compensationVolatge = averageVoltage / compensationCoefficient;            // temperature compensation
        tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; // convert voltage value to TDS value

        return tdsValue;
    }

    return 0; // or any default value if TDS value cannot be calculated
}



