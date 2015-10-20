// библиотека для работы с GPRS устройством
#include <GPRS_Shield_Arduino.h>
 
// библиотека для эмуляции Serial порта
// она нужна для работы библиотеки GPRS_Shield_Arduino
#include <SoftwareSerial.h>
 
// номер на который будем звонить
#define PHONE_NUMBER  "+79263995140"
 
// создаём объект класса GPRS
// с помощью него будем давать команды GPRS шилду
GPRS gprs();
 
void setup()
{
  // включаем GPRS шилд
  gprs.powerUpDown();
  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600); 
  while (!Serial) {
    // ждём, пока не откроется монитор последовательного порта
    // для того, чтобы отследить все события в программе
  }
  // проверяем есть ли связь с GPRS устройством
  while (!gprs.init()) {
    // если связи нет, ждём 1 секунду
    // и выводим сообщение об ошибке
    // процесс повторяется в цикле
    // пока не появится ответ от GPRS устройства
    delay(1000);
    Serial.print("Init error\r\n");
  }
  // вывод об удачной инициализации GPRS Shield
  Serial.println("GPRS init success");
  // сообщаем о наборе номера
  Serial.print("Start to call ");
  Serial.print(PHONE_NUMBER);
  // звоним по указанному номеру
  gprs.callUp(PHONE_NUMBER);
}
 
void loop()
{
}
