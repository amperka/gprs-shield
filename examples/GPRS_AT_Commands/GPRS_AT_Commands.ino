// буфер PC Serial
char bufferPC_Serial[64];
// буфер GPRS Serial
char bufferGPRS_Serial[64];
 
int i = 0;
int j = 0;
 
void setup()
{
  // включаем GPRS-шилд
  gprs_OnOff();
 
  // открываем последовательный порт для мониторинга действий в программе
  Serial.begin(9600);
 
  // открываем последовательный порт
  // для связи с GPRS-устройством со скоростью 9600 бод
  Serial1.begin(9600);
 
  while (!Serial) {
    // ждём, пока не откроется монитор последовательного порта
    // для того, чтобы отследить все события в программе
  }
 
  // пока не установится связь с GPRS-устройством, будем крутиться в теле функции
  gprsTest();
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
  i = 0;
  // если появились данные с компьютера
  if (Serial.available() > 0) {
    while (Serial.available() > 0) {
      // пока идёт передача данных,
      // записываем каждый байт в символьный массив
      bufferPC_Serial[i++]=(Serial.read());
    }
    // добавляем символ конца строки
    bufferPC_Serial[i] = '\0';
    // записываем данные в GPRS Shield
    Serial1.println(bufferPC_Serial);
    Serial.println("");
    // очищаем буфер PC Serial
    clearBufferPC_Serial();
  }
}
 
void serialGPRSread()
{
    j = 0;
    // если появились данные с GPRS Shield
    if (Serial1.available() > 0) {
      while (Serial1.available() > 0) {
        // пока идёт передача данных,
        // записываем каждый байт в символьный массив
        bufferGPRS_Serial[j++]=(Serial1.read());
    }
    // добавляем символ конца строки
    bufferGPRS_Serial[j] = '\0';
    // выводим полученные данные с GPRS Shield в Serial-порт
    Serial.write(bufferGPRS_Serial);
    // очищаем буфер GPRS Serial
    clearBufferGPRS_Serial();
  }
}
 
void clearBufferPC_Serial()
{
  for (int t = 0; t < i; t++) {
    // очищаем буфер,
    // присваивая всем индексам массива значение 0
    bufferPC_Serial[t] = 0;
  }
}
 
void clearBufferGPRS_Serial()
{
  for (int t = 0; t < j; t++) {
    // очищаем буфер,
    // присваивая всем индексам массива значение 0
    bufferGPRS_Serial[t] = 0;
  }
}
 
void gprsTest()
{
  // бесконечный цикл
  while (1) {
    // ждём 1 секунду
    delay(1000);
    j = 0;
    // посылаем в GPRS Shield АТ-команду "AT"
    Serial1.println("AT");
    // если появились данные с GPRS Shield
    if (Serial1.available() > 0) {
      while (Serial1.available() > 0) {
        // пока идёт передача данных,
        // записываем каждый байт в символьный массив
        bufferGPRS_Serial[j++] = Serial1.read();
      }
      // добавляем символ конца строки
      bufferGPRS_Serial[j] = '\0';
      // посылаем АТ-команду "AT"; если GPRS Shield исправен,
      // он должен вернуть команду "AT";
      // сравниваем всё что находиться в буфере GPRS Shield
      // со строкой "AT\r\n\r\nOK\r\n"
      if (strcmp(bufferGPRS_Serial, "AT\r\n\r\nOK\r\n") == 0) {
        // если всё верно выводим в Serial порт "State OK"
        // и выходим из бесконечного цикла
        Serial.println("State OK");
        break;
      } else {
        // если строки разные, значит произошла ошибка
        // выводим сообщение об ошибке и продолжаем цикл
        Serial.println("Init ERROR");
      }
    }
    // очищаем буфер GPRS Serial
    clearBufferGPRS_Serial();
  }
}
 
void gprs_OnOff()
{
  // настраиваем пин №2 в режим выхода
  pinMode(2, OUTPUT);
  // проверяем состояние 3 пина
  if (digitalRead(3) != HIGH) {
    // если на нём «низкий уровень»
    // подаём на пин 2 «высокий уровень»
    digitalWrite(2, HIGH);
    // ждём 3 секунды
    delay(3000);
  }
  // подаём на пин 2 «низкий уровень»
  digitalWrite(2, LOW);
}
