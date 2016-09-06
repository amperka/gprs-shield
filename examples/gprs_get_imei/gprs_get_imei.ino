#include <GPRS_Shield_Arduino.h>

#define PIN_PK         8           // Контакт включения GPRS модуля
#define PIN_ST         9           // контакт состояния GPRG модуля
#define BAUDRATE  115200           // частота обмена данными

GPRS   gprsModul(PIN_PK, PIN_ST, BAUDRATE);   // создаём объекты:

void setup() 
{
  Serial.begin(BAUDRATE);
  while (!Serial) {
    // ждем откроется монитор последовательного порта
  } 
  Serial.println("Serial OK!");
  Serial.print  ("Power... ");
  gprsModul.powerOn();
  Serial.println(" On");
}



void loop() {
  unsigned char rc;
  char tmpBuf[64];
  int  dataLen;
  //
  Serial.print  ("init...   ");
  rc = gprsModul.init();
  Serial.print  ("rc=");
  Serial.print  (int(rc));
  //
  Serial.print("  imei=");
  sim900_clean_buffer(tmpBuf, sizeof(tmpBuf));
  gprsModul.getImei(tmpBuf);
  Serial.print(tmpBuf);
  Serial.println(";");
  //
  delay(5*1000);
}
