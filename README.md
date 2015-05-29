GPRS Shield
===========

Библиотека для Arduino, позволяющая управлять [GPRS Shield’ом](http://amperka.ru/product/arduino-gprs-shield)
от [Амперки](http://amperka.ru/).

Установка
=========

Скачайте последний релиз библиотеки:

<a class="btn btn-sm btn-primary" href="https://github.com/amperka/gprs-shield/releases/download/v1.0/GPRSShield-1.0.zip">Скачать GPRS Shield v1.0</a>

В Arduino IDE выберите пункт меню «Скетч» → «Импортировать библиотеку» →
«Добавить библиотеку…». В появившемся окне выберите скачаный архив с
библиотекой. Установка завершена.

Пример использования
====================

```cpp
#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
 
GPRS gprs(9600);
 
void setup()
{
  gprs.powerUpDown();
  while (!gprs.init())
    delay(1000);

  gprs.sendSMS("+74990001122", "Hello SMS from Amperka!");
}
 
void loop()
{
}
```

Больше примеров — [в статье на Амперка / Вики](http://wiki.amperka.ru/%D0%BF%D1%80%D0%BE%D0%B4%D1%83%D0%BA%D1%82%D1%8B:gprs-shield).
