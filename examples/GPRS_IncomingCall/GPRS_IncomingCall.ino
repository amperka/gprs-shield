// библиотека для работы с GPRS устройством
#include <GPRS_Shield_Arduino.h>
 
// библиотека для эмуляции Serial порта
// она нужна для работы библиотеки GPRS_Shield_Arduino
#include <SoftwareSerial.h>
 
// создаём объект класса GPRS
// с помощью него будем давать команды GPRS шилду
GPRS gprs;
 
void setup()
{
  //настраиваем пин №13 в режим выхода,
  pinMode(13, OUTPUT);
  // подаём на пин 13 «низкий уровень»
  digitalWrite(13, LOW);
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
    // процесс повторяется в цикле,
    // пока не появится ответ от GPRS устройства
    delay(1000);
    Serial.print("Init error\r\n");
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
    // если поступает входящий звонок,
    // подаём на пин 13 «высокий уровень», чтобы
    // зажечь встроенный на Iskra светодиод
    digitalWrite(13, HIGH);
    // выводим сообщение о входящем вызове
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
    // гасим светодиод
    digitalWrite(13, LOW);
  }
}
