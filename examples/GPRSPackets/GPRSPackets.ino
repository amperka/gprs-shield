/*
 * Program wake shield up, send UDP packet, and put shield to sleep.
 * Wait for 1 minute and repeat
 * 
 * lib
 * https://github.com/amperka/gprs-shield/blob/master/GPRS_Shield_Arduino.h
 * https://github.com/amperka/gprs-shield/blob/master/GPRS_Shield_Arduino.cpp
 * 
 * Yarka Ilin
 * 2019
 */

// библиотека для работы с GPRS устройством
#include <GPRS_Shield_Arduino.h>
 
// библиотека для эмуляции Serial порта
// она нужна для работы библиотеки GPRS_Shield_Arduino
#include <SoftwareSerial.h>

#include <math.h>

#define TERMIST_B 4300 
#define VIN 5.0

// создаём объект mySerial и передаём номера управляющих пинов RX и TX
SoftwareSerial mySerial(10, 11);

// создаём объект класса GPRS и передаём в него объект mySerial 
//GPRS gprs(mySerial);
// можно указать дополнительные параметры — пины PK и ST
// по умолчанию: PK = 2, ST = 3
GPRS gprs(mySerial, 2, 3);

char buf[490];
int i = 0;
char str[] = "STRU;datapacket;_st01 xx.x xx.x xxx.x";
char prefix[] = "STRU;datapacket;";
double Temp = 20.2;
char sTemp[] = "20.2";

char IP[] = "217.197.198.226";
int port = 30005;

void send_message(char * _IP, int _port, char * msg);
char * get_temp();

void setup(){
  ;
}
 
void loop(){
  // sensors
  dtostrf(Temp, 4, 1, get_temp());
  sprintf(str, "%s_st01 %s 34.3 101.5", prefix, sTemp);
  
  // connection
  send_message(IP, port, str);

  // wait (minimum 6 sec)
  delay(60000);
}

void send_message(char * _IP, int _port, char * msg){
  //
  mySerial.begin(9600);
  // power Up GPRS module
  gprs.powerOn();
  // initialize GPRS module including SIM card check & signal strength
  while(!gprs.init()){
    delay(1000);
  }  
  // Connect the GPRS module to the network
  if(gprs.join("internet.beeline.ru", "beeline", "beeline", 2 * DEFAULT_TIMEOUT)){
    // Open a tcp/udp connection
    if (gprs.connect(UDP, _IP, _port, 2 * DEFAULT_TIMEOUT)){
      // Send packet
      gprs.send(msg);
      // Close a tcp connection
      gprs.close();
      delay(500);
    }
    // Disconnect the GPRS module from the network
    gprs.disconnect();
  }
  // wait a while
  delay(1000);
  // power down GPRS module
  gprs.powerOff();
  //
  mySerial.end();
  return;
}

char * get_temp(){
  double voltage = analogRead(A0) * VIN / 1024.0;
  double r1 = voltage / (VIN - voltage);
  //Temp = 1./( 1./(TERMIST_B)*log(r1)+1./(25. + 273.) ) - 273;
  Temp += 0.1;
  dtostrf(Temp, 4, 1, sTemp);
  sprintf(str, "%s_st01 %s 34.3 101.5", prefix, sTemp);
  return sTemp;
}
