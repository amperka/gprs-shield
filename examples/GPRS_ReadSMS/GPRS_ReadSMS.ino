// библиотека для работы с GPRS устройством
#include <GPRS_Shield_Arduino.h>
 
// библиотека для эмуляции Serial-порта
// она нужна для работы библиотеки GPRS_Shield_Arduino
#include <SoftwareSerial.h>
 
// длина сообщения
#define MESSAGE_LENGTH 160
 
// номер сообщения в памяти сим-карты
int messageIndex = 0;
 
// текст сообщения
char message[MESSAGE_LENGTH];
// номер, с которого пришло сообщение
char phone[16];
// дата отправки сообщения
char datetime[24];
 
// создаём объект класса GPRS. По умолчанию скорость общения с ним 9600 бод
// с помощью него будем давать команды GPRS шилду
GPRS gprs;
 
void setup()
{
  // включаем GPRS-шилд
  gprs.powerUpDown();
  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600);
  while (!Serial) {
    // ждём пока не откроется монитор последовательного порта
    // для того, чтобы отследить все события в программе
  }
  // проверяем, есть ли связь с GPRS-устройством
  while (!gprs.init()) {
    // если связи нет, ждём 1 секунду
    // и выводим сообщение об ошибке;
    // процесс повторяется в цикле,
    // пока не появится ответ от GPRS-устройства
    delay(1000);
    Serial.print("Init error\r\n");
  }
  // выводим сообщение об удачной инициализации GPRS Shield
  Serial.println("GPRS init success");
  Serial.println("Please send SMS message to me!");
}
 
void loop()
{
  // если пришло новое сообщение
  if (gprs.ifSMSNow()) {
    // читаем его
    gprs.readSMS(message, phone, datetime);

    // выводим номер, с которого пришло смс
    Serial.print("From number: ");
    Serial.println(phone);
 
    // выводим дату, когда пришло смс
    Serial.print("Datetime: ");
    Serial.println(datetime);
 
    // выводим текст сообщения
    Serial.print("Recieved Message: ");
    Serial.println(message);
  }
}
