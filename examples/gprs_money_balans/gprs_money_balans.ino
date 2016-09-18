#include <GPRS_Shield_Arduino.h>

#define PIN_PK         8           // Контакт включения GPRS модуля
#define PIN_ST         9           // контакт состояния GPRG модуля
#define BAUDRATE  115200           // частота обмена данными

GPRS gprsModul(PIN_PK, PIN_ST, BAUDRATE);   // создаём объект типа GPRS
int  moneyBalanceInt;              // Переменная в которой хранится текущий денежный баланс
char moneyBalanceBuf[32];          // Строка ответа о текущем балансе



//const char apn[]       = "internet.mts.ru"; // Для билайна: "home.beeline.ru", МТС: "internet.mts.ru" megafon: "internet"
//const char lgn[]       = "mts";             // Логин=Пароль MTS: "mts" megafon: "gdata"
const char balanceReq[]= "#100#";             // USSD номер проверки балланса



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
  Serial.print  (";   readBalance");
  rc = gprsModul.readBalance(balanceReq, moneyBalanceBuf, sizeof(moneyBalanceBuf)-1, moneyBalanceInt);
  Serial.print  (" rc=");
  Serial.print  (int(rc));
  //
  Serial.print  (";  ");
  Serial.print  (moneyBalanceBuf);
  Serial.print  (";  ");
  Serial.print  (moneyBalanceInt);
  Serial.println(";");
  // 
  gprsModul.powerOff();
  delay(15*1000);
}
