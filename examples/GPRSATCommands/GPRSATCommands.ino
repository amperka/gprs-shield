// библиотека для работы с GPRS устройством
#include <GPRS_Shield_Arduino.h>
 
// создаём объект класса GPRS и передаём в него объект Serial1 
GPRS gprs(Serial1);
// можно указать дополнительные параметры — пины PK и ST
// по умолчанию: PK = 2, ST = 3
// GPRS gprs(Serial1, 2, 3);
 
void setup()
{ 
  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600);
  // ждём, пока не откроется монитор последовательного порта
  // для того, чтобы отследить все события в программе
  while (!Serial) {
  }
  Serial.print("Serial init OK\r\n");
  // открываем Serial-соединение с GPRS Shield
  Serial1.begin(9600);
  // включаем GPRS шилд
  gprs.powerOn();
  // проверяем есть ли связь с GPRS устройством
  while (!gprs.init()) {
    // если связи нет, ждём 1 секунду
    // и выводим сообщение об ошибке
    // процесс повторяется в цикле
    // пока не появится ответ от GPRS устройства
    Serial.print("GPRS Init error\r\n");
    delay(3000);
  }
  // выводим сообщение об удачной инициализации GPRS Shield
  Serial.println("GPRS init success");
}
 
void loop()
{
  // считываем данные с компьютера и записываем их в GPRS Shield
  serialPCread();
  // считываем данные с GPRS Shield и выводим их в Serial-порт
  serialGPRSread();
}
 
void serialPCread()
{
  if (Serial.available() > 0) {
    // если приходят данные по USB
    while (Serial.available() > 0) {
      // записываем их в GPRS Shield
      Serial1.write(Serial.read());
    }
  }
}
 
void serialGPRSread()
{
    if (Serial1.available() > 0) {
      // если приходят данные с GPRS Shield
      while (Serial1.available() > 0) {
        // передаём их в USB
        Serial.write(Serial1.read());
    }
  }
}
