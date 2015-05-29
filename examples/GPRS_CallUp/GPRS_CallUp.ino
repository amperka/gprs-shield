/*
GPRS Call Up 

This sketch is used to test seeeduino GPRS_Shield's callUp function.
to make it work, you should insert SIM card to Seeeduino GPRS 
and replace the phoneNumber,enjoy it!

create on 2015/05/14, version: 1.0
by lawliet.zou(lawliet.zou@gmail.com)
*/
#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>

#define PIN_TX    7
#define PIN_RX    8
#define BAUDRATE  9600
#define PHONE_NUMBER  "+79671547389"

GPRS gprsTest(BAUDRATE);//RX,TX,PWR,BaudRate

void setup() {
  gprs_OnOff();
  Serial.begin(9600);
  while(!gprsTest.init()) { //gprs init
      delay(1000);
      Serial.print("init error\r\n");
  }
  Serial.println("gprs init success");
  Serial.println("start to call ...");
  gprsTest.callUp(PHONE_NUMBER);
}

void loop() {
  //nothing to do
}

void gprs_OnOff()
{
  pinMode(2, OUTPUT);
  if(digitalRead(3)!=1)
  {
    digitalWrite(2, HIGH);    // РџРѕРґР°РµРј High РЅР° РїРёРЅ 2
    delay(3000);              // РЅР° 3 СЃРµРєСѓРЅРґС‹  
  }
  digitalWrite(2, LOW);     // Рё РѕС‚РїСѓСЃРєР°РµРј РІ Low.

}
