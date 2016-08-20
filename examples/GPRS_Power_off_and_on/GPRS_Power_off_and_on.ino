#include <GPRS_Shield_Arduino.h>

#define PIN_PK         8           // Контакт включения GPRS модуля
#define PIN_ST         9           // контакт состояния GPRG модуля

GPRS gprs(PIN_PK, PIN_ST, 115200);

void setup() {
  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(115200);
  while (!Serial) {
    // ждём, пока не откроется монитор последовательного порта
    // для того, чтобы отследить все события в программе
  }
  Serial.println("Serial Ok.");
}

void loop() {
  long t1,t2;
  if (gprs.isPowerOn()) {
    Serial.print ("Выключаем.");
    t1 = millis();
    gprs.powerOff();
    if (gprs.isPowerOn()) {
      Serial.print(" !!! --- не выключилось --- !!! ");
      Serial.println(millis()-t1);
    } else {
      Serial.print("Ok. ");
      Serial.println(millis()-t1);
    }
  } else {
    Serial.print ("Включаем.");
    t1 = millis();
    gprs.powerOn();
    t2 = millis();
    if (!gprs.isPowerOn()) {
      Serial.print("!!! --- не включилось --- !!!");
      Serial.println(t2-t1);
    } else {
      Serial.print(" Ok.");
      Serial.println(t2-t1);
    }
  }
}
