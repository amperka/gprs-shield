/*  Пример получения времени с внутренних часов gprs-модуля.
*/

#include <GPRS_Shield_Arduino.h>

#define PIN_PK         8           // Контакт включения GPRS модуля
#define PIN_ST         9           // контакт состояния GPRG модуля
#define BAUDRATE  115200           // частота обмена данными

GPRS   gprsModul(PIN_PK, PIN_ST, BAUDRATE); // создаём объект

//const char apn[]       = "internet.mts.ru"; // Для билайна: "home.beeline.ru", МТС: "internet.mts.ru" megafon: "internet"
//const char lgn[]       = "mts";             // Логин=Пароль MTS: "mts" megafon: "gdata"
//const char balanceReq[]= "#100#";           // USSD номер проверки балланса
const char ntpService[]= "pool.ntp.org";      // Сервер NTP-синхронизации



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
  sim900_clean_buffer(tmpBuf, sizeof(tmpBuf));
  rc = gprsModul.init(tmpBuf);
  Serial.print  ("rc=");
  Serial.print  (int(rc));
  //
  sim900_clean_buffer(tmpBuf, sizeof(tmpBuf));
  Serial.print  (";    ntpSync...");
  rc = gprsModul.syncNtp(ntpService);
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
