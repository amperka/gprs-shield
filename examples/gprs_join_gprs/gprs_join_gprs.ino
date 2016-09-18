/*  
    Библиотека содержит две версии функции joinGprs для подключения к сети через
    мобильного оператора. Одна версия с APN параметрами, другая без них.
    Экспериментально я выявил, что подключение происходит успешно при любых 
    значениях параметров. Возможно, что это только в "домашней" сети и не на 
    всех тарифных планах. Я проверял МТС, Beeline и Мегафон в Москве.
    При удачном подключении к GPRS, модуль должен получит IP адрес. 
    Функция joinGprs возвращает его в первом параметре. 
    Сама функция возвращает статус GPRS соединения:
      0 - соединение устанавливается
      1 - соединение установлено
      2 - соединение закрывается
      3 - нет соединения
      10 - состояние не распознано
      ipv4Buf возвращает в формате xxx.xxx.xxx.xxx

*/

#include <GPRS_Shield_Arduino.h>

#define PIN_PK         8             // Контакт включения GPRS модуля
#define PIN_ST         9             // контакт состояния GPRG модуля
#define BAUDRATE  115200             // частота обмена данными

const char apn[]       = "internet"; // Для билайна: "home.beeline.ru", МТС: "internet.mts.ru" megafon: "internet"
const char lgn[]       = "mts";      // Логин  MTS: "mts" megafon: "gdata"
const char pwd[]       = "mts";      // Пароль MTS: "mts" megafon: "gdata"

GPRS   gprsModul(PIN_PK, PIN_ST, BAUDRATE); // создаём объект



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
  char tmpBuf[16];
  //
  Serial.print  ("Power... ");
  gprsModul.powerOn();
  Serial.print  (" On");
  //
  Serial.print  (";   init... ");
  rc = gprsModul.init();                          // Инициализация без подключения к сети
  Serial.print  ("rc=");
  Serial.print  (int(rc));
  //
  sim900_clean_buffer(tmpBuf, sizeof(tmpBuf));    // предварительная очистка буфера для IP адреса
  Serial.print  (";   joinGprs... ");
  rc = gprsModul.joinGprs(tmpBuf);                // Инициализация без APN параметров
  //rc = gprsModul.joinGprs(tmpBuf, apn,lgn,pwd); // Инициализация с APN параметрами
  Serial.print  ("rc=");
  Serial.print  (int(rc));
  Serial.print  (";   IP-address = ");
  Serial.print  (tmpBuf);
  Serial.println(";");
  // 
  gprsModul.powerOff();
  delay(15*1000);
}
