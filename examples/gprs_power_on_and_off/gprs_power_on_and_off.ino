#include <GPRS_Shield_Arduino.h>

#define PIN_PK         8           // Контакт включения GPRS модуля
#define PIN_ST         9           // контакт состояния GPRG модуля
#define BAUDRATE  115200           // частота обмена данными

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
    t2 = millis();
    if (gprs.isPowerOn()) {
      Serial.print(" !!! --- не выключилось --- !!! ");
    } else {
      Serial.print(" Ok. ");
    }
  } else {
    Serial.print ("Включаем.");
    t1 = millis();
    gprs.powerOn();
    t2 = millis();
    if (!gprs.isPowerOn()) {
      Serial.print(" !!! --- не включилось --- !!!");
    } else {
      Serial.print(" Ok. ");
    }
  }
  Serial.print(t2-t1);     // Печатаем, сколько времени (милисекунд)
  Serial.println(" ms;");  // ушло на включение или выклюсчение
}
