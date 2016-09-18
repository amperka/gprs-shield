#include <GPRS_Shield_Arduino.h>

#define PIN_PK         8              // Контакт включения GPRS модуля
#define PIN_ST         9              // контакт состояния GPRG модуля
#define BAUDRATE  115200              // частота обмена данными

char tmpBuf[64];                      // Символьный буфер
int  moneyBalanceInt;                 // Переменная в которой хранится текущий денежный баланс
char moneyBalanceBuf[32];             // Строка ответа о текущем балансе

const char apn[]       = "internet.mts.ru";// Для билайна: "home.beeline.ru", МТС: "internet.mts.ru" megafon: "internet"
const char lgn[]       = "mts";            // Логин=Пароль MTS: "mts" megafon: "gdata"
const char balanceReq[]= "#100#";          // USSD номер проверки балланса
const char ntpService[]= "pool.ntp.org";   // Сервер NTP-синхронизации

GPRS   gprsModul(PIN_PK, PIN_ST, BAUDRATE);



void setup() 
{
  Serial.begin(BAUDRATE);
  while (!Serial) {
    // ждем, пока откроется монитор последовательного порта
  } 
  Serial.println("Serial OK!");
}



void loop() {
  unsigned char rc;
  char tmpBuf[64];
  char urlBuf[]="http://diribus.net/loger.php?ddddddd=fff";
  int  dataLen;
  //
  Serial.print  ("Power...");
  gprsModul.powerOn();
  Serial.print  ("On;  init rc=");
  rc = gprsModul.init(tmpBuf, apn, lgn, lgn);
  Serial.print  (int(rc));
  //
  rc = gprsModul.getGprsStatus(tmpBuf);
  Serial.print  (";  getGprsStatus rc=");
  Serial.print  ( int(rc) );
  Serial.print  (";  ip=");
  Serial.print  (tmpBuf);
  
  int result = gprsModul.httpGet( urlBuf, dataLen );
  Serial.print  (";  Http код возврата =");
  Serial.print  (result);
  Serial.print  ("   Размер страницы =");
  Serial.print  (dataLen);

  Serial.print  (";  dateTime=");
  sim900_clean_buffer(tmpBuf, sizeof(tmpBuf));
  Serial.print(gprsModul.getDateTime(tmpBuf));
  Serial.println();
  gprsModul.powerOff();
  delay(15*1000);
}
