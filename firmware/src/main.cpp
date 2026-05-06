#include <Arduino.h>
#include <ArduinoJson.h>

TaskHandle_t SecurityMonitorTask;

void runSecurityMonitor(void * parameter) {
  for(;;) {
    int challenge = random(0, 255);
    StaticJsonDocument<128> doc_out;
    doc_out["challenge"] = challenge;

    unsigned long start_time = micros();
    serializeJson(doc_out, Serial);
    Serial.println(); 

    String incoming_data = "";
    while(micros() - start_time < 50000) {
      if(Serial.available()) {
        incoming_data = Serial.readStringUntil('\n');
        break;
      }
    }

    unsigned long latency = micros() - start_time;

    if(incoming_data.length() > 0) {
      StaticJsonDocument<256> doc_in;
      DeserializationError error = deserializeJson(doc_in, incoming_data);
      if(!error) {
        int ver_ticks = doc_in["ticks"];
        Serial.printf("Chal: %d, Ticks: %d, Latency(us): %lu\n", challenge, ver_ticks, latency);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void setup() {
  Serial.begin(921600);
  while(!Serial); 
  xTaskCreatePinnedToCore(runSecurityMonitor, "SecurityTask", 4096, NULL, 24, &SecurityMonitorTask, 1);
}

void loop() {
  vTaskDelete(NULL); 
}