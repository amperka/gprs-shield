/*
 * GPRS_Shield_Arduino.cpp
 * A library for SeeedStudio seeeduino GPRS shield 
 *  
 * Copyright (c) 2015 seeed technology inc.
 * Website    : www.seeed.cc
 * Author     : lawliet zou
 * Create Time: April 2015
 * Change Log :
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include "GPRS_Shield_Arduino.h"
#include "SoftwareSerial.h"

GPRS* GPRS::inst;



GPRS::GPRS(uint8_t pkPin, uint8_t stPin, uint32_t baudRate)
{
  _stPin = stPin;
  _pkPin = pkPin;

  inst = this;
  sim900_init(&SERIAL_PORT_HARDWARE);
  SERIAL_PORT_HARDWARE.begin(baudRate);
}



GPRS::GPRS(uint8_t pkPin, uint8_t stPin, uint8_t rx, uint8_t tx, uint32_t baudRate)
{  
  _stPin = stPin;
  _pkPin = pkPin;

  inst = this;
  SoftwareSerial* gprsserial = new SoftwareSerial(rx, tx);
  stream = gprsserial ;
  sim900_init(stream);
  gprsserial ->begin(baudRate);    
}



///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                               POWER                                     ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

void GPRS::powerOn(void) {         
  if(!digitalRead(_stPin)) {      // Если питание не подано,
    powerUpDown();                // то выполним стандартную последовательность сигналов 
  }      
}



void GPRS::powerOff(void) {       // Желательно корректно отключать питание, для сохранения переменных
  long t1 = millis();             // в памяти и для разрегистрации в сети мобильного оператора.
  sim900_send_cmd("AT+CPOWD=1\r\n"); // Даем команду на выключение
  while (millis()-t1 < 8000) {       // и в течение ХХХХ милисекунд джем
    if(!digitalRead(_stPin)) {       // произошло ли выключение.
      return;                        // Выходим, если отключилось быстрее.
    }
  }
  powerUpDown();                     // Иначе передёрним питание 
}



void GPRS::powerUpDown(void) {     // The same sequence is used for switching on and to power off
  pinMode(_pkPin, OUTPUT);
  digitalWrite(_pkPin, LOW);
  delay(1000);
  digitalWrite(_pkPin, HIGH);
  delay(2000);
  digitalWrite(_pkPin, LOW);
  delay(3000);
}



bool GPRS::isPowerOn(void)
{
  return digitalRead(_stPin);
}



///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                              INITIALIZATION                             ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

unsigned char GPRS::init(void)
{
  unsigned char rc;
  if(0 == (rc = initialSetting())) {
    delay(5000);      // Пауза для завершения всех инициализационных процессов
  }
  return rc;
}



unsigned char GPRS::init(char* ipv4Buf) 
{
  unsigned char rc;
  if(0 == (rc = initialSetting())) {
    if(1 == joinGprs(ipv4Buf)) {
      rc = 0;
    } else {
      rc = 9;      // нет GPRS, ip-адрес не получен
    }
  }
  return rc;
}



unsigned char GPRS::init(char* ipv4Buf,
                   const char* apn, 
                   const char* lgn, 
                   const char* pwd) 
{
  unsigned char rc;
  if(0 == (rc = initialSetting())) {
    if(1 == joinGprs(ipv4Buf, apn, lgn, pwd)) {
      rc = 0;
    } else {
      rc = 10;      // нет GPRS, ip-адрес не получен
    }
  }
  return rc;
}



unsigned char GPRS::initialSetting(void)
{
  unsigned char rc;
  long t1;
  if(!sim900_check_with_cmd("AT\r\n",OK,CMD))                    return 1;
                     // а есть ли вообще модуль и способен ли он отвечать?
  if(!sim900_check_with_cmd("ATE0\r\n",OK,CMD))                  return 2;
                     // отключили эхо-ответ, тем самым уменьшив объем буферов
                     // для анализа ответа от модуля.
  if(!sim900_check_with_cmd("AT+CFUN=1\r\n",OK,CMD))             return 3;
  if(!checkSIMStatus())                                          return 4;
  if (!sim900_check_with_cmd("AT+CNMI?\r\n", "+CNMI: 2,0,2,1,1\r\nOK\r\n",CMD)) {
    if (!sim900_check_with_cmd("AT+CNMI=2,0,2,1,1\r\n",OK,CMD))  return 5;
                     // Установили режим, при котором SMS не лезут сразу в 
                     // serial, а сохраняются в памяти. Это удобно для последующего
                     // чтения и разбора SMS
  }
  if (!sim900_check_with_cmd("AT+CMGF?\r\n", "+CMGF: 1\r\nOK\r\n",CMD)) {
    if (!sim900_check_with_cmd("AT+CMGF=1\r\n",OK,CMD))          return 6;
  }
  if (!sim900_check_with_cmd("AT+CLIP=1\r\n",OK,CMD))            return 7;
  return 0;
}



///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                               TOOLS                                     ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

char* GPRS::getImei(char* imei) 
{
  //                   --> CRLF       =  2
  //999999999999999    --> 15 + CRLF  = 17
  //                   --> CRLF       =  2
  //OK                 --> 2 + CRLF   =  4
  char tbuf[25];
  sim900_clean_buffer(tbuf,sizeof(tbuf));
  char *p, *s;
  int   i = 0;
  sim900_flush_serial();
  sim900_send_cmd("AT+GSN\r\n");
  sim900_read_buffer(tbuf,19,DEFAULT_TIMEOUT);
  if(NULL != ( s = strstr(tbuf,"\r\n")+2 )) {
    if(NULL != ( p = strstr(s,"\r\n"))){
      p = s ;
      while((*p != '\r')&&(i < 15)) {
          imei[i++] = *(p++);
      }
    }
  }
  imei[i] = '\0';
  sim900_wait_for_resp(OK, CMD);
  return imei;
}



  char* GPRS::getDateTime(char* buffer)
  {
    //AT+CCLK?                       --> 8 + CRLF = 10
    //+CCLK: "14/11/13,21:14:41+04"  --> 29+ CRLF = 31
    //                               --> CRLF     =  2
    //OK                             -->          =  4
  
    byte i = 0;
    char gprsBuffer[40];
    char *p,*s;
    sim900_flush_serial();
    sim900_send_cmd("AT+CCLK?\r\n");
    sim900_clean_buffer(gprsBuffer,40);
    sim900_read_buffer(gprsBuffer,32,DEFAULT_TIMEOUT);
    if(NULL != ( s = strstr(gprsBuffer,"+CCLK:"))) {
      s = strstr((char *)(s),"\"");
      s = s + 1;  //We are in the first date-time character 
      p = strstr((char *)(s),"\"")-6; //p is last character minutes
      if (NULL != s) {
        i = 0;
        while (s < p) {
          buffer[i++] = *(s++);
        }
        buffer[i] = '\0';            
      }
    }  
    sim900_wait_for_resp(OK, CMD);
    return buffer;
  }
  


unsigned char GPRS::syncNtp (const char* ntpServer) //Синхронизфция времени в модеме с NTP сервером
{ /*
  AT+CNTP="pool.ntp.org",3,1,0
  AT+CNTP

  Коды завершения:
  1 - удачная синхронизация
  11 - нет GPRS
  12 - не установились параметры для синнхронизации
  61 - ошибка сетевого соединения
  62 - ошибка DNS
  63 - ошибка соединения
  64 - превышено время ожидания
  65 - ошибка сервера
  66 - операция недоступна
  */
   
  char *p,*s;
  char tmpBuf[56];
  unsigned char rc;
  sim900_clean_buffer(tmpBuf,sizeof(tmpBuf));
  if (1 == (rc = joinGprs(tmpBuf))) {      // Есть GPRS подключение или удалось подключиться
    sim900_send_cmd("AT+CNTP=\"");         // устанавливаем параметры синхронизации
    sim900_send_cmd(ntpServer);
    sim900_send_cmd("\",3,1,0\r\n");
    if (sim900_wait_for_resp(OK,CMD)) {     // Параметры установились успешно
      sim900_check_with_cmd("AT+CNTP\r\n"   ,OK,DATA);   // собственно синхронизация.  
      sim900_wait_for_resp ("+CNTP: ",  DATA,19,15000);  // ждем ответ об успешности 
      delay(10);
      sim900_clean_buffer(tmpBuf,sizeof(tmpBuf));
      sim900_read_buffer(tmpBuf,4);                      // читаем код завершения
      rc = atoi(tmpBuf);

    } else {                  // Не установились параметры инициализации
      rc = 12;
    }
  } else {
    rc = 11;                             // Если не удалось подключиться по GPRS
  }
  return rc;
}



unsigned char GPRS::readBalance(const char* moneyRequestBuf,
                                      char* moneyBalanceBuf, 
                                      int   bufLen,
                                      int&  moneyBalanceInt)
{
  //AT+CUSD=1,"#100#"                                    = 19
  //                                                     =  2
  //                                                     =  2
  //OK                                          --> CRLF =  4
  //                                            --> CRLF =  2  
  //+CUSD: 0,"Balance:45,05r,Limit:0,01r ",64            = 41
  //                                                итого= 70
  byte i = 0;
  char gprsBuffer[70];
  char *p, *s;
  unsigned char rc;
  sim900_flush_serial();
  sim900_send_cmd("AT+CUSD=1,\"");
  sim900_send_cmd(moneyRequestBuf);
  sim900_check_with_cmd("\"\r\n",OK,DATA);
  sim900_clean_buffer(gprsBuffer,sizeof(gprsBuffer));
  sim900_read_buffer(gprsBuffer,sizeof(gprsBuffer)-1);
  //Serial.print(gprsBuffer);

  if(NULL != ( s = strstr(gprsBuffer,"CUSD:"))) {
    s = strstr((char *)(s),"\"");
    s = s + 1;                                     // We are in the first character 
    p = strstr((char *)(s),"r") + 1;
    if (NULL != s) {
      i = 0;
      while ((s < p) && (i < (bufLen -1))) {
        moneyBalanceBuf[i++] = *(s++);
      }
      moneyBalanceBuf[i] = '\0';            
    }     // Ответ получен. Теперь попробуем из него вытащить цифры баланса
    
    if(NULL != (s = strstr(moneyBalanceBuf,"Balance:"))) {       // Для МТС
      s = s + 8;
    } else {
      if(NULL != (s = strstr(moneyBalanceBuf,"balans"))) {       // Для Beeline
        s = s + 6;
      }
    }
    if(NULL != s) {
      p = strstr((char *)(s),"r");
      if (NULL != s) {
        i = 0;
        while (s < p) {
          gprsBuffer[i++] = *(s++);
        }
        gprsBuffer[i] = '\0';            
      }
      moneyBalanceInt = atoi(gprsBuffer);
      rc = 0;     // результат получен
    } else {
      rc = 2;    // в ответе нет баланса
    } 
  } else {
    rc = 4;      // не получен предсказуемый ответ
    i = 0;
    int n = min(sizeof(gprsBuffer), bufLen);
    while (i < n) {
      moneyBalanceBuf[i] = gprsBuffer[i];
      i += 1;
    }
    moneyBalanceBuf[i] = '\0';            

  }
  //sim900_wait_for_resp(OK, CMD);
  sim900_flush_serial();
  return rc;
}



bool GPRS::checkSIMStatus(void)
{
  char gprsBuffer[32];
  int count = 0;
  sim900_clean_buffer(gprsBuffer,32);
  while(count < 3) {
    sim900_send_cmd("AT+CPIN?\r\n");
    sim900_read_buffer(gprsBuffer,32,DEFAULT_TIMEOUT);
    if((NULL != strstr(gprsBuffer,"+CPIN: READY"))) {
      break;
    }
    count++;
    delay(300);
  }
  if(count == 3) {
    return false;
  }
  return true;
}



unsigned char GPRS::getSignalStrength(void) 
{
  //AT+CSQ: 00,00     --> 13 + CRLF = 15
  //                  --> CRLF     = 2
  //OK                --> 2 + CRLF = 4

  byte result = 99;
  char gprsBuffer[21];
  sim900_clean_buffer(gprsBuffer,21);
  char *s;
  sim900_send_cmd("AT+CSQ\r\n");
  sim900_read_buffer(gprsBuffer,21,DEFAULT_TIMEOUT);
  if(NULL != ( s = strstr(gprsBuffer,"+CSQ: "))) {
    result = atoi(s+6);
    sim900_wait_for_resp("OK\r\n", CMD);        
  }  
  return result;
}



///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                                SMS                                      ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

bool GPRS::sendSMS(char *number, char *data)
{
  // Set message mode to ASCII
  if(!sim900_check_with_cmd("AT+CMGF=1\r\n", "OK\r\n", CMD)) {
    return false;
  }

  delay(500);
  sim900_send_cmd("AT+CMGS=\"");
  sim900_send_cmd(number);

  if(!sim900_check_with_cmd("\"\r\n",">",CMD)) {
      return false;
  }

  delay(1000);
  sim900_send_cmd(data);
  delay(500);
  sim900_send_End_Mark();
  return true;
}

char GPRS::isSMSunread()
{
  char gprsBuffer[48];  //48 is enough to see +CMGL:
  char *s;    

  //List of all UNREAD SMS and DON'T change the SMS UNREAD STATUS
  sim900_send_cmd(F("AT+CMGL=\"REC UNREAD\",1\r\n"));
    /*If you want to change SMS status to READ you will need to send:
          AT+CMGL=\"REC UNREAD\"\r\n
      This command will list all UNREAD SMS and change all of them to READ
      
     If there is not SMS, response is (30 chars)
         AT+CMGL="REC UNREAD",1  --> 22 + 2
                                 --> 2
         OK                      --> 2 + 2

     If there is SMS, response is like (>64 chars)
         AT+CMGL="REC UNREAD",1
         +CMGL: 9,"REC UNREAD","XXXXXXXXX","","14/10/16,21:40:08+08"
         Here SMS text.
         OK  
         
         or

         AT+CMGL="REC UNREAD",1
         +CMGL: 9,"REC UNREAD","XXXXXXXXX","","14/10/16,21:40:08+08"
         Here SMS text.
         +CMGL: 10,"REC UNREAD","YYYYYYYYY","","14/10/16,21:40:08+08"
         Here second SMS        
         OK           
    */

         sim900_clean_buffer(gprsBuffer,31); 
         sim900_read_buffer(gprsBuffer,30,DEFAULT_TIMEOUT); 
    //Serial.print("Buffer isSMSunread: ");Serial.println(gprsBuffer);

         if(NULL != ( s = strstr(gprsBuffer,"OK"))) {
        //In 30 bytes "doesn't" fit whole +CMGL: response, if recieve only "OK"
        //    means you don't have any UNREAD SMS
          delay(50);
          return 0;
        } else {
        //More buffer to read
        //We are going to flush serial data until OK is recieved
          sim900_wait_for_resp("OK\r\n", CMD);        
        //sim900_flush_serial();
        //We have to call command again
          sim900_send_cmd("AT+CMGL=\"REC UNREAD\",1\r\n");
          sim900_clean_buffer(gprsBuffer,48); 
          sim900_read_buffer(gprsBuffer,47,DEFAULT_TIMEOUT);
    //Serial.print("Buffer isSMSunread 2: ");Serial.println(gprsBuffer);       
          if(NULL != ( s = strstr(gprsBuffer,"+CMGL:"))) {
            //There is at least one UNREAD SMS, get index/position
            s = strstr(gprsBuffer,":");
            if (s != NULL) {
                //We are going to flush serial data until OK is recieved
              sim900_wait_for_resp("OK\r\n", CMD);
              return atoi(s+1);
            }
          } else {
            return -1; 

          }
        } 
        return -1;
      }

void GPRS::readSMS(char *message, char *phone, char *datetime)  
{
  /* Response is like:
  +CMT: "+79772941911","","15/12/15,01:51:24+12"
  +CMGR: "REC READ","XXXXXXXXXXX","","14/10/09,17:30:17+08"
  SMS text here
  */

  int i = 0;
  int j = 0;

  char gprsBuffer[80 + 160];

  sim900_clean_buffer(gprsBuffer,sizeof(gprsBuffer));
  sim900_read_buffer(gprsBuffer,sizeof(gprsBuffer));
  
  int len = strlen(gprsBuffer);

  if(gprsBuffer[i]=='\"') {
    i++;
    j = 0;
    while(gprsBuffer[i]!='\"') {
      phone[j++] = gprsBuffer[i++];
    }
    phone[j] = '\0';
    i++;
  }

  if(gprsBuffer[i]==',')
    i++;

  if(gprsBuffer[i]=='\"') {
    i++;
    while(gprsBuffer[i]!='\"') {
      i++;
    }
    i++;
  }

  if(gprsBuffer[i]==',')
    i++;

  if(gprsBuffer[i]=='\"') {
    i++;
    j = 0;
    while(gprsBuffer[i]!='\"') {
      datetime[j++] = gprsBuffer[i++];
    }
    datetime[j] = '\0';
    i++;
  }

  if(gprsBuffer[i]=='\r')
    i++;

  if(gprsBuffer[i]=='\n')
    i++;

  j = 0;
  while(i < len - 2)
    message[j++] = gprsBuffer[i++];
  
  message[j] = '\0';
}

bool GPRS::readSMS(int messageIndex, char *message,int length)
{
  int i = 0;
  char gprsBuffer[100];
  char num[4];
  char *p,*s;
  sim900_check_with_cmd("AT+CMGF=1\r\n","OK\r\n",CMD);
  delay(1000);
      sim900_send_cmd("AT+CMGR=");
      itoa(messageIndex, num, 10);
      sim900_send_cmd(num);
      sim900_send_cmd("\r\n");
      sim900_clean_buffer(gprsBuffer,sizeof(gprsBuffer));
      sim900_read_buffer(gprsBuffer,sizeof(gprsBuffer),DEFAULT_TIMEOUT);
      if(NULL != ( s = strstr(gprsBuffer,"+CMGR:"))){
        if(NULL != ( s = strstr(s,"\r\n"))){
          p = s + 2;
          while((*p != '\r')&&(i < length-1)) {
            message[i++] = *(p++);
          }
          message[i] = '\0';
          return true;
        }
      }
      return false;   
}

void GPRS::readSMS()
{
  char gprsBuffer[100];
  sim900_clean_buffer(gprsBuffer,sizeof(gprsBuffer));
  sim900_read_buffer(gprsBuffer,sizeof(gprsBuffer),DEFAULT_TIMEOUT);
  Serial.write(gprsBuffer);
}



bool GPRS::deleteSMS(void)
{       
  return sim900_check_with_cmd("AT+CMGD=1,3\r\n",OK,CMD); // Удалить все, кроме непрочитанных
}



bool GPRS::deleteSMS(int index)
{
    //char cmd[16];
     char num[4];
    //sprintf(cmd,"AT+CMGD=%d\r\n",index);
     sim900_send_cmd("AT+CMGD=");
     itoa(index, num, 10);
     sim900_send_cmd(num);
	//snprintf(cmd,sizeof(cmd),"AT+CMGD=%d\r\n",index);
    //sim900_send_cmd(cmd);
    //return 0;
    // We have to wait OK response
	//return sim900_check_with_cmd(cmd,"OK\r\n",CMD);
     return sim900_check_with_cmd("\r\n","OK\r\n",CMD);	
}

bool GPRS::ifSMSNow(void)
{
  return sim900_check_with_cmd("","+CMT: ",CMD);
}



///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                                RINGS                                    ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

bool GPRS::callUp(char *number)
{
  if(!sim900_check_with_cmd("AT+COLP=1\r\n","OK\r\n",CMD)) {
      return false;
  }
  delay(1000);
  sim900_send_cmd("ATD");
  sim900_send_cmd(number);
  sim900_send_cmd(";\r\n");
  return true;
}

void GPRS::answer(void)
{
  //TO CHECK: ATA doesnt return "OK" ????
  sim900_send_cmd("ATA\r\n");
}

bool GPRS::ifcallNow(void)
{
  return sim900_check_with_cmd("","RING\r\n",CMD);
}

bool GPRS::ifcallEnd(void)
{
  return sim900_check_with_cmd("","NO CARRIER\r\n",CMD);
}

void GPRS::callEnd(void)
{
  sim900_send_cmd("ATH0\r\n");
}

bool GPRS::hangup(void)
{
  return sim900_check_with_cmd("ATH\r\n","OK\r\n",CMD);
}

  bool GPRS::disableCLIPring(void)
  {
    return sim900_check_with_cmd("AT+CLIP=0\r\n","OK\r\n",CMD);
  }

  bool GPRS::isCallActive(char *number)
  {
    char gprsBuffer[46];  //46 is enough to see +CPAS: and CLCC:
    char *p, *s;
    int i = 0;

    sim900_send_cmd("AT+CPAS\r\n");
    /*Result code:
        0: ready
        2: unknown
        3: ringing
        4: call in progress
    
      AT+CPAS   --> 7 + 2 = 9 chars
                --> 2 char              
      +CPAS: 3  --> 8 + 2 = 10 chars
                --> 2 char
      OK        --> 2 + 2 = 4 chars
    
      AT+CPAS
      
      +CPAS: 0
      
      OK
    */

      sim900_clean_buffer(gprsBuffer,29);
      sim900_read_buffer(gprsBuffer,27);
    //HACERR cuando haga lo de esperar a OK no me haría falta esto
    //We are going to flush serial data until OK is recieved
      sim900_wait_for_resp("OK\r\n", CMD);    
    //Serial.print("Buffer isCallActive 1: ");Serial.println(gprsBuffer);
      if(NULL != ( s = strstr(gprsBuffer,"+CPAS:"))) {
        s = s + 7;
        if (*s != '0') {
         //There is something "running" (but number 2 that is unknow)
         if (*s != '2') {
           //3 or 4, let's go to check for the number
           sim900_send_cmd("AT+CLCC\r\n");
           /*
           AT+CLCC --> 9
           
           +CLCC: 1,1,4,0,0,"656783741",161,""
           
           OK  

           Without ringing:
           AT+CLCC
           OK              
           */

           sim900_clean_buffer(gprsBuffer,46);
           sim900_read_buffer(gprsBuffer,45);
    //Serial.print("Buffer isCallActive 2: ");Serial.println(gprsBuffer);
           if(NULL != ( s = strstr(gprsBuffer,"+CLCC:"))) {
             //There is at least one CALL ACTIVE, get number
             s = strstr((char *)(s),"\"");
             s = s + 1;  //We are in the first phone number character            
             p = strstr((char *)(s),"\""); //p is last character """
             if (NULL != s) {
              i = 0;
              while (s < p) {
                number[i++] = *(s++);
              }
              number[i] = '\0';            
            }
             //I need to read more buffer
             //We are going to flush serial data until OK is recieved
            return sim900_wait_for_resp("OK\r\n", CMD); 
          }
        }
      }        
    } 
    return false;
  }



///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///                                GPRS                                     ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

unsigned char GPRS::joinGprs(char* ipv4Buf,
                       const char* apn, 
                       const char* lgn, 
                       const char* pwd)
{
  char *p,*s;
  char  tmpBuf[56];
  long t1, t2;
  unsigned char  rc;
  rc = getGprsStatus(tmpBuf);      // Запрос о состоянии GPRS соединения
  if (rc != 1)   {                 // Если не установлено, то установим
    sim900_check_with_cmd("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n",OK,CMD);
    //
    sim900_send_cmd("AT+SAPBR=3,1,\"APN\",\"");
    sim900_send_cmd(apn);
    sim900_check_with_cmd("\"\r\n", OK, CMD);
    //
    sim900_send_cmd("AT+SAPBR=3,1,\"USER\",\"");
    sim900_send_cmd(lgn);
    sim900_check_with_cmd("\"\r\n", OK, CMD);
    //
    sim900_send_cmd("AT+SAPBR=3,1,\"PWD\",\"");
    sim900_send_cmd(pwd);
    sim900_check_with_cmd("\"\r\n", OK, CMD);
    //
    t1 = millis();
    while (1 != (rc = getGprsStatus(ipv4Buf))) {
      t2 = millis();
      if (20*1000 < (t2-t1)) return 20;
      sim900_check_with_cmd("AT+SAPBR=1,1\r\n" ,OK,CMD,1);
    }
    //
    if (rc == 1) {                                        // Если удачно соединились
      sim900_check_with_cmd("AT+SAPBR=5,1\r\n", OK,CMD);  // то запомним параметры
    }
  }
  return rc;
}



unsigned char GPRS::joinGprs(char* ipv4Buf)
{
  char *p,*s;
  long t1, t2;
  unsigned char  rc;
  rc = getGprsStatus(ipv4Buf);                // Запрос о состоянии GPRS соединения
  if (rc != 1)   {                            // Если не установлено, то установим
    sim900_check_with_cmd("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n",OK,CMD);
    t1 = millis();
    while (1 != (rc = getGprsStatus(ipv4Buf))) {
      t2 = millis();
      if (20*1000 < (t2-t1)) return 20;
      sim900_check_with_cmd("AT+SAPBR=1,1\r\n" ,OK,CMD,1);
    }
  }
  return rc;
}



unsigned char GPRS::getGprsStatus(char* ipv4Buf)   
{
  //AT+SAPBR=2,1                       1
  //                                   1
  //+SAPBR: 1,3,"xxx.xxx.xxx.xxx"     29
  //                                   1
  //OK                                 3
  //                                   1
  char *p, *s;
  unsigned char  rc;
  char  tmpBuf[31];
  int   i;
  sim900_flush_serial();
  sim900_send_cmd("AT+SAPBR=2,1\r\n");               // Запрос о состоянии GPRS соединения
  sim900_clean_buffer(tmpBuf,sizeof(tmpBuf));
  sim900_read_buffer(tmpBuf,sizeof(tmpBuf),DEFAULT_TIMEOUT); // Считываем ответ
  //Serial.println("<");
  //Serial.println(tmpBuf);
  //Serial.println(">");

  if(NULL != ( s = strstr(tmpBuf,"+SAPBR: "))) {     // находим нужное место
    s += 8;                                          // 
    if(NULL != ( s = strstr((char *)(s),","))) {     // после первой запятой
      rc = *(s+1) - '0';                // для конвертации в цифру отнять код нуля
      s = strstr((char *)(s),"\"")+1;   // Теперь ищем ip-адрес в кавычках
      p = strstr((char *)(s),"\"");     // Закрывающая кавычка
      if ((NULL != p) && (NULL != s)) {
        i = 0;
        while (s < p) {
          ipv4Buf[i++] = *(s++);
        }
        ipv4Buf[i] = '\0';            
      } else {
        rc = 9;                 // не распознан ip-адрес
      }
    } else {
      rc = 10;                  // не распознан код состояния
    }
  } else {
    rc = 10;                    // не распознан код состояния
  }
  delay(10);               // Перед очисткой буфера надо дождаться завер-     
  sim900_flush_serial();   // шения вывода данных.
  return rc;
}



void GPRS::disconnectGprs()
{
  sim900_check_with_cmd("AT+SAPBR=0,1\r\n",OK,CMD);  // отключаемся
}



int GPRS::httpGet(char* url, int&  dataLen)
{
  char  tmpBuf[111];
  char *p, *s;
  int   result;
  sim900_flush_serial();
  sim900_clean_buffer(tmpBuf, sizeof(tmpBuf));
  sim900_check_with_cmd("AT+HTTPINIT\r\n",OK,CMD,33);
  sim900_check_with_cmd("AT+HTTPPARA=\"CID\",1\r\n",OK,CMD);
  sim900_send_cmd("AT+HTTPPARA=\"URL\",\"");
  sim900_send_cmd(url);
  sim900_check_with_cmd("\"\r\n",OK,CMD);
  sim900_check_with_cmd("AT+HTTPACTION=0\r\n",OK,CMD);
  if(!sim900_wait_for_resp ("+HTTPACTION:0," ,CMD))               return 7;
  //   
  sim900_read_buffer(tmpBuf,sizeof(tmpBuf),DEFAULT_TIMEOUT);
  sim900_flush_serial();
  if(NULL != ( s = strstr(tmpBuf,","))) {
    tmpBuf[ s-tmpBuf ] = '\0';
    result = atoi( tmpBuf );
  } else {
    result = 1;    // СтатусКод не распознан
  } 
  //
  if(NULL != ( p = strstr( (char *)(s+1),"\r\n"))) {
    tmpBuf[ p-tmpBuf ] = '\0';
    dataLen = atoi( (char *)(s+1) );
  } else {
    result = 2;    // dataLen не распознан
  } 
  //
  sim900_check_with_cmd("AT+HTTPTERM\r\n",OK,CMD);
  return result;
}



bool GPRS::connect(Protocol ptl, const char * host, int port, int timeout)
{
  //char cmd[64];
  char num[4];
  char resp[96];

  //sim900_clean_buffer(cmd,64);
  if(ptl == TCP) {
    sim900_send_cmd("AT+CIPSTART=\"TCP\",\"");
  } else if (ptl == UDP) {
    sim900_send_cmd("AT+CIPSTART=\"UDP\",\"");
  } else {
    return false;
  }
  sim900_send_cmd(host);
  sim900_send_cmd("\",");
  itoa(port, num, 10);
  sim900_send_cmd(num);
  sim900_send_cmd("\r\n");
	//        sprintf(cmd, "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n",host, port);

  delay(2000);
  //sim900_send_cmd(cmd);
  sim900_read_buffer(resp,96,timeout);

  //Serial.print("Connect resp: "); Serial.println(resp);    
  if(NULL != strstr(resp,"CONNECT")) { //ALREADY CONNECT or CONNECT OK
    return true;
  }
  return false;
}



//Overload with F() macro to SAVE memory
bool GPRS::connect(Protocol ptl,const __FlashStringHelper *host, const __FlashStringHelper *port, int timeout)
{
  //char cmd[64];
  char resp[96];

  //sim900_clean_buffer(cmd,64);
  if(ptl == TCP) {
    sim900_send_cmd(F("AT+CIPSTART=\"TCP\",\""));   //%s\",%d\r\n",host, port);
  } else if (ptl == UDP) {
    sim900_send_cmd(F("AT+CIPSTART=\"UDP\",\""));   //%s\",%d\r\n",host, port);
  } else {
    return false;
  }
  sim900_send_cmd(host);
  sim900_send_cmd(F("\","));
  sim900_send_cmd(port);
  sim900_send_cmd(F("\r\n"));
  //Serial.print("Connect: "); Serial.println(cmd);
  sim900_read_buffer(resp, 96, timeout);
  //Serial.print("Connect resp: "); Serial.println(resp);    
  if(NULL != strstr(resp,"CONNECT")) { //ALREADY CONNECT or CONNECT OK
    return true;
  }
  return false;
}



bool GPRS::close()
{
  // if not connected, return
  //    if ( getGprsStatus("                ") == 3) {
  //      return true;
  //    }
  return sim900_check_with_cmd("AT+CIPCLOSE\r\n", "CLOSE OK\r\n", CMD);
}

  int GPRS::readable(void)
  {
    return sim900_check_readable();
  }

  int GPRS::wait_readable(int wait_time)
  {
    return sim900_wait_readable(wait_time);
  }

  int GPRS::wait_writeable(int req_size)
  {
    return req_size+1;
  }

int GPRS::send(const char * str, int len)
{
  //char cmd[32];
 char num[4];
 if(len > 0){
      //snprintf(cmd,sizeof(cmd),"AT+CIPSEND=%d\r\n",len);
//sprintf(cmd,"AT+CIPSEND=%d\r\n",len);
  sim900_send_cmd("AT+CIPSEND=");
  itoa(len, num, 10);
  sim900_send_cmd(num);
  if(!sim900_check_with_cmd("\r\n",">",CMD)) {
      //if(!sim900_check_with_cmd(cmd,">",CMD)) {
    return 0;
  }
/*if(0 != sim900_check_with_cmd(str,"SEND OK\r\n", DEFAULT_TIMEOUT * 10 ,DATA)) {
    return 0;
}*/
    delay(500);
    sim900_send_cmd(str);
    delay(500);
    sim900_send_End_Mark();
    if(!sim900_wait_for_resp("SEND OK\r\n", DATA, DEFAULT_TIMEOUT * 10, DEFAULT_INTERCHAR_TIMEOUT * 10)) {
      return 0;
    }        
  }
  return len;
}

        int GPRS::send(const char * str) {
    //char cmd[32];
          int len=strlen(str);
          char num[4];
          if(len > 0){
        //snprintf(cmd,sizeof(cmd),"AT+CIPSEND=%d\r\n",len);
    //sprintf(cmd,"AT+CIPSEND=%d\r\n",len);
            sim900_send_cmd("AT+CIPSEND=");
            itoa(len, num, 10);
            sim900_send_cmd(num);
            if(!sim900_check_with_cmd("\r\n",">",CMD)) {
        //if(!sim900_check_with_cmd(cmd,">",CMD)) {
              return 0;
            }
        /*if(0 != sim900_check_with_cmd(str,"SEND OK\r\n", DEFAULT_TIMEOUT * 10 ,DATA)) {
            return 0;
        }*/
            delay(500);
            sim900_send_cmd(str);
            delay(500);
            sim900_send_End_Mark();
            if(!sim900_wait_for_resp("SEND OK\r\n", DATA, DEFAULT_TIMEOUT * 10, DEFAULT_INTERCHAR_TIMEOUT * 10)) {
              return 0;
            }        
          }
          return len;
        }   



uint32_t GPRS::str_to_ip(const char* str)
{
  uint32_t ip = 0;
  char* p = (char*)str;
  for(int i = 0; i < 4; i++) {
    ip |= atoi(p);
    p = strchr(p, '.');
    if (p == NULL) {
      break;
    }
    ip <<= 8;
    p++;
  }

  return ip;
}
