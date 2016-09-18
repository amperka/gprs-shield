#include <GPRS_Shield_Arduino.h>


#define PIN_PK  8
#define PIN_ST  9
#define PIN_RX 10
#define PIN_TX 11
#define BAUDRATE 115200

char  bufferPC_Serial[96];
char  bufferGPRS_Serial[96];
GPRS   gprsModul(PIN_PK, PIN_ST, BAUDRATE);


void setup()
{
   Serial.begin(BAUDRATE);          // открываем последовательный порт для мониторинга действий в программе
   while (!Serial) {                // ждём, пока не откроется монитор последовательного порта
   }
   Serial.println("Serial - OK");
   gprsModul.powerOn();
//   gprsModul.init();                // Ждём инициализацию модема
   sim900_check_with_cmd("ATE1\r\n",OK,CMD);
}


void loop()
{
   serialPCread();         // считываем данные с компьютера и записываем их в GPRS Shield
   gprsSIM900read();       // считываем данные с GPRS Shield и выводим их в Serial-порт
}


void serialPCread()
{
   int i = 0;
   if (Serial.available() > 0) {
      sim900_clean_buffer(bufferPC_Serial,  96);
      while (Serial.available() > 0) {
         bufferPC_Serial[i++]=(Serial.read());
      }
      delay(100);
      bufferPC_Serial[i] = '\0';
      if (strcmp("Q\r\n",bufferPC_Serial) == 0 ) {
         gprsModul.powerOff();
      } else if (strcmp("B\r\n",bufferPC_Serial) == 0 ) {
         gprsModul.powerOn();
         gprsModul.init();
      };
      sim900_send_cmd(bufferPC_Serial);
   }
}


void gprsSIM900read()
{
   int j = sim900_check_readable();
   if (j > 0) 
   {
      sim900_clean_buffer(bufferGPRS_Serial,96);
      sim900_read_buffer(bufferGPRS_Serial, j);
      bufferGPRS_Serial[j] = '\0';
      Serial.print(bufferGPRS_Serial);
   }
}
