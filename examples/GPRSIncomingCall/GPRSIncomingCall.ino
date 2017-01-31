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
    // процесс повторяется в цикле,
    // пока не появится ответ от GPRS устройства
    delay(1000);
    Serial.print("GPRS Init error\r\n");
  }
  // вывод об удачной инициализации GPRS Shield
  Serial.println("GPRS init success");
  // сообщаем об ожидании звонка
  Serial.println("Wait to call ");
}
 
void loop()
{
  // ожидаем звонка
  if (gprs.ifcallNow()) {
    // если поступает входящий звонок
    // выводим сообщение
    Serial.println("Incoming call");
    // по истечении 5 секунд берём трубку
    delay(5000);
    gprs.answer();
    // выводим сообщение о начале разговора
    delay(1000);
    Serial.println("Call a conversation");
    while (!gprs.ifcallEnd()) {
      // ждём пока месть абонент не положит трубку
    }
    // выводим сообщение о конце разговора
    Serial.println("Call over");
  }
}
