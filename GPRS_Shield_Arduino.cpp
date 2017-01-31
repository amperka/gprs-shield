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
#include <HardwareSerial.h>
#include "GPRS_Shield_Arduino.h"

GPRS* GPRS::inst;

GPRS::GPRS(Stream& serial, uint8_t pkPin, uint8_t stPin)
{
  _stPin = stPin;
  _pkPin = pkPin;

  inst = this;

  stream = &serial;
  sim900_init(stream);
}

bool GPRS::init(void)
{
  if (!sim900_check_with_cmd("AT\r\n","OK\r\n",CMD))
    return false;


  if (!sim900_check_with_cmd("AT+CFUN=1\r\n","OK\r\n",CMD))
    return false;


  if (!checkSIMStatus())
    return false;

  if (!sim900_check_with_cmd("AT+CNMI?\r\n", "+CNMI: 2,2,0,0,0\r\nOK\r\n", CMD)) {
    if (!sim900_check_with_cmd("AT+CNMI=2,2,0,0,0\r\n","OK\r\n", CMD)) {
      return false;
    }
  }

  if (!sim900_check_with_cmd("AT+CMGF?\r\n", "+CMGF: 1\r\nOK\r\n", CMD)) {
    if (!sim900_check_with_cmd("AT+CMGF=1\r\n","OK\r\n", CMD)) {
      return false;
    }
  }

  if (!sim900_check_with_cmd("AT+CLIP=1\r\n","OK\r\n", CMD)) {
      return false;
  }
  return true;
}

bool GPRS::checkPowerUp(void) {
    return sim900_check_with_cmd("AT\r\n","OK\r\n", CMD);
}

// power Up GPRS Shield
void GPRS::powerOn() {
	delay(1000);
    pinMode(_pkPin, OUTPUT);
    if (!digitalRead(_stPin)) {
        digitalWrite(_pkPin, HIGH);
        delay(3000);
    }
    digitalWrite(_pkPin, LOW);
    delay(3000);
}

// power Off GPRS Shield
void GPRS::powerOff() {
    pinMode(_pkPin, OUTPUT);
    if (digitalRead(_stPin)) {
        digitalWrite(_pkPin, HIGH);
        delay(3000);
    }
    digitalWrite(_pkPin, LOW);
    delay(3000);
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

  bool GPRS::getDateTime(char *buffer)
  {
  //AT+CCLK?						--> 8 + CRLF = 10
  //+CCLK: "14/11/13,21:14:41+04"   --> 29+ CRLF = 31
  //								--> CRLF     =  2
  //OK

    byte i = 0;
    char gprsBuffer[46];
    char *p,*s;
    sim900_send_cmd("AT+CCLK?\r\n");
    sim900_clean_buffer(gprsBuffer,43);
    sim900_read_buffer(gprsBuffer,43,DEFAULT_TIMEOUT);
    if(NULL != ( s = strstr(gprsBuffer,"+CCLK:"))) {
      s = strstr((char *)(s),"\"");
        s = s + 1;  //We are in the first phone number character
        p = strstr((char *)(s),"\""); //p is last character """
        if (NULL != s) {
          i = 0;
          while (s < p) {
            buffer[i++] = *(s++);
          }
          buffer[i] = '\0';
        }
        //We are going to flush serial data until OK is recieved
        return sim900_wait_for_resp("OK\r\n", CMD);
      }
      return false;
    }

    byte GPRS::getSignalStrength() {
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

//Here is where we ask for APN configuration, with F() so we can save MEMORY
//bool GPRS::join(const __FlashStringHelper *apn, const __FlashStringHelper *userName, const __FlashStringHelper *passWord)
    bool GPRS::join(char* apn, char* userName, char* passWord, int timeout)
    {
     byte i;
     char *p, *s;
     char ipAddr[32];
/*    if(!sim900_check_with_cmd("AT+CIPSHUT\r\n","SHUT OK\r\n", CMD)) {
      Serial.write("Error = 1\r\n");
    return false;
    }
    delay(1000);
*/
    sim900_send_cmd("AT+CIPSHUT\r\n");
    delay(500);
    //Select multiple connection
    //sim900_check_with_cmd("AT+CIPMUX=1\r\n","OK",DEFAULT_TIMEOUT,CMD);

    //set APN. OLD VERSION
    //snprintf(cmd,sizeof(cmd),"AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n",_apn,_userName,_passWord);
    //sim900_check_with_cmd(cmd, "OK\r\n", DEFAULT_TIMEOUT,CMD);
    sim900_send_cmd("AT+CSTT=\"");
    sim900_send_cmd(apn);
    sim900_send_cmd("\",\"");
    sim900_send_cmd(userName);
    sim900_send_cmd("\",\"");
    sim900_send_cmd(passWord);
    sim900_send_cmd("\"\r\n");
    delay(500);
    //Brings up wireless connection

    sim900_send_cmd("AT+CIICR\r\n");
    delay(4000);
    sim900_wait_for_resp("OK\r\n", CMD);
    delay(500);
//    sim900_check_with_cmd("AT+CIICR\r\n","OK\r\n", CMD);


    //Get local IP address
    sim900_send_cmd("AT+CIFSR\r\n");
    delay(500);
    sim900_clean_buffer(ipAddr,32);
    sim900_read_buffer(ipAddr,32,DEFAULT_TIMEOUT);

	//Response:
	//AT+CIFSR\r\n       -->  8 + 2
	//\r\n				 -->  0 + 2
	//10.160.57.120\r\n  --> 15 + 2 (max)   : TOTAL: 29
	//Response error:
	//AT+CIFSR\r\n
	//\r\n
	//ERROR\r\n
    if (NULL != strstr(ipAddr,"ERROR")) {
      Serial.write("Error = 2\r\n");
      return false;
    }
    s = ipAddr + 12;
    p = strstr((char *)(s),"\r\n"); //p is last character \r\n
    if (NULL != s) {
      i = 0;
      while (s < p) {
        ip_string[i++] = *(s++);
      }
      ip_string[i] = '\0';
    }
    _ip = str_to_ip(ip_string);
    if(_ip != 0) {

      return true;
    }
    Serial.write("Error = 3\r\n");
    return false;
  }

  void GPRS::disconnect()
  {
    sim900_send_cmd("AT+CIPSHUT\r\n");
  }

  bool GPRS::connect(Protocol ptl,const char * host, int port, int timeout)
  {
    //char cmd[64];
   char num[4];
   char resp[96];

    //sim900_clean_buffer(cmd,64);
   if(ptl == TCP) {
    sim900_send_cmd("AT+CIPSTART=\"TCP\",\"");
    sim900_send_cmd(host);
    sim900_send_cmd("\",");
    itoa(port, num, 10);
    sim900_send_cmd(num);
    sim900_send_cmd("\r\n");
//        sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%d\r\n",host, port);
  } else if(ptl == UDP) {
    sim900_send_cmd("AT+CIPSTART=\"UDP\",\"");
    sim900_send_cmd(host);
    sim900_send_cmd("\",");
    itoa(port, num, 10);
    sim900_send_cmd(num);
    sim900_send_cmd("\r\n");

	//        sprintf(cmd, "AT+CIPSTART=\"UDP\",\"%s\",%d\r\n",host, port);
  } else {
    return false;
  }

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
} else if(ptl == UDP) {
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

  bool GPRS::is_connected(void)
  {
    char resp[96];
    sim900_send_cmd("AT+CIPSTATUS\r\n");
    sim900_read_buffer(resp,sizeof(resp),DEFAULT_TIMEOUT);
    if(NULL != strstr(resp,"CONNECTED")) {
        //+CIPSTATUS: 1,0,"TCP","216.52.233.120","80","CONNECTED"
      return true;
    } else {
        //+CIPSTATUS: 1,0,"TCP","216.52.233.120","80","CLOSED"
        //+CIPSTATUS: 0,,"","","","INITIAL"
      return false;
    }
  }

  bool GPRS::close()
  {
    // if not connected, return
    if (!is_connected()) {
      return true;
    }
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

        int GPRS::recv(char* buf, int len)
        {
          sim900_clean_buffer(buf,len);
    sim900_read_buffer(buf,len);   //Ya he llamado a la funcion con la longitud del buffer - 1 y luego le estoy añadiendo el 0
    return strlen(buf);
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

char* GPRS::getIPAddress()
{
  //I have already a buffer with ip_string: snprintf(ip_string, sizeof(ip_string), "%d.%d.%d.%d", (_ip>>24)&0xff,(_ip>>16)&0xff,(_ip>>8)&0xff,_ip&0xff);
  return ip_string;
}

unsigned long GPRS::getIPnumber()
{
  return _ip;
}

void setST(uint8_t stPin)
{

}
void setPK(uint8_t pkPin)
{

}

/* NOT USED bool GPRS::gethostbyname(const char* host, uint32_t* ip)
{
    uint32_t addr = str_to_ip(host);
    char buf[17];
    //snprintf(buf, sizeof(buf), "%d.%d.%d.%d", (addr>>24)&0xff, (addr>>16)&0xff, (addr>>8)&0xff, addr&0xff);
    if (strcmp(buf, host) == 0) {
        *ip = addr;
        return true;
    }
    return false;
}
*/
