#include <GPRS_Shield_Arduino.h>

#define PIN_PK         8           // Контакт включения GPRS модуля
#define PIN_ST         9           // контакт состояния GPRG модуля
#define BAUDRATE  115200           // частота обмена данными

const char balanceReq[]= "#100#";           // USSD номер проверки балланса

GPRS gprsModul(PIN_PK, PIN_ST, BAUDRATE);   // создаём объект типа GPRS
int  moneyBalanceInt;              // Переменная в которой хранится текущий денежный баланс
char moneyBalanceBuf[32];          // Строка ответа о текущем балансе



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



void loop()
{
  unsigned char rc;
  //
  Serial.print  ("init...");
  rc = gprsModul.init();
  Serial.print  ("  rc=");
  Serial.print  (int(rc));
  //
  Serial.print  (";   readBalance rc=");
  rc = gprsModul.readBalance(balanceReq, moneyBalanceBuf, sizeof(moneyBalanceBuf), moneyBalanceInt);
  Serial.print  ( int(rc) );
  Serial.print  (";   BalanceBuf=");
  Serial.print  (moneyBalanceBuf);
  Serial.print  (";   moneyInt=");
  Serial.print  (moneyBalanceInt);
  Serial.println(";");
  //
  delay(5*1000);
}