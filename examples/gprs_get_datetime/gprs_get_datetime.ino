/*  Пример получения времени с внутренних часов gprs-модуля.
    Если в модуле не установлена батарейка, то после каждого выключения
    часы сбрасываются в начальное значение.
    Для установки правильного текущего времени можно синхронизироваться
    по серверам службы NTP. Для этого в библиотеке есть функция. 
    Пример её использования смотри в скатче
    gprs_ntp_datetime
*/

#include <GPRS_Shield_Arduino.h>

#define PIN_PK         8           // Контакт включения GPRS модуля
#define PIN_ST         9           // контакт состояния GPRG модуля
#define BAUDRATE  115200           // частота обмена данными

GPRS   gprsModul(PIN_PK, PIN_ST, BAUDRATE);   // создаём объект

void setup() 
{
  Serial.begin(BAUDRATE);
  while (!Serial) {
    // ждем, пока откроется монитор последовательного порта
  } 
  Serial.println("Serial OK!");
}



void loop()
{
  unsigned char rc;
  char tmpBuf[64];
  //
  Serial.print  ("Power... ");
  gprsModul.powerOn();
  Serial.print  (" On");
  //
  Serial.print  (";   init... ");
  rc = gprsModul.init();
  Serial.print  ("rc=");
  Serial.print  (int(rc));
  //
  sim900_clean_buffer(tmpBuf, sizeof(tmpBuf));
  Serial.print  (";   dateTime = ");
  Serial.print  (gprsModul.getDateTime(tmpBuf));
  Serial.println(";");
  // 
  gprsModul.powerOff();
  delay(15*1000);
}