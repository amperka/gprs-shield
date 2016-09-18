/*  
    Библиотека содержит три версии функции init для начальной инициализации
    модуля. Первая версия - без параметров. Она рекомендуется, когда 
    не планируете выход в интернет. То есть, будете пользоваться только SMS 
    и голосовым вызовом.
    Если же интернет нужен, то можно выполнить подключение и получение IP адреса
    сразу. Для этого вторая и третья версии функции. Параметры те же, что и 
    в функции joinGprs.
    Третья версия не только подключается к сети, но и сохраняет APN параметры.
    Пока модуль не будет выключен, эти параметры использоваться по умолчанию.
    
    При удачном подключении к GPRS, модуль должен получит IP адрес. 
    Функция возвращает его в параметре. IP адрес выдаётся мобильным
    оператором. В общем случае, требуется указывать APN, логин и пароль. 
    Если их не указать, всё равно предоставляется IP адрес, если услуга
    доступна. Скорее всего, используется значение по умолчанию.
    Проверял для МТС, Beeline и Мегафон.
    Однако, можно не испытывать судьбу и использовать версию функции init со
    всеми параметрами. Можете пробовать разные функции, поочередно закометировав 
    соответствующую строку.
*/

#include <GPRS_Shield_Arduino.h>

#define PIN_PK         8             // Контакт включения GPRS модуля
#define PIN_ST         9             // контакт состояния GPRG модуля
#define BAUDRATE  115200             // частота обмена данными

const char apn[]       = "internet"; // Для билайна: "home.beeline.ru", МТС: "internet.mts.ru" megafon: "internet"
const char lgn[]       = "mts";      // Логин  MTS: "mts" megafon: "gdata"
const char pwd[]       = "mts";      // Пароль MTS: "mts" megafon: "gdata"

GPRS   gprsModul(PIN_PK, PIN_ST, BAUDRATE); // создаём объект



void setup() 
{
  Serial.begin(BAUDRATE);
  while (!Serial) {
    // ждем, пока откроется монитор последовательного порта
  } 
  Serial.println("Serial OK!");
}



void loop()
{
  unsigned char rc;
  char tmpBuf[16];
  //
  Serial.print  ("Power... ");
  gprsModul.powerOn();
  Serial.print  (" On");
  //
  sim900_clean_buffer(tmpBuf, sizeof(tmpBuf));  // предварительная очистка буфера для IP адреса
  Serial.print  (";   init... ");
  rc = gprsModul.init(tmpBuf);                  // Инициализация без APN параметров
//rc = gprsModul.init(tmpBuf, apn, lgn, pwd);   // Инициализация с APN параметрами
  Serial.print  ("rc=");
  Serial.print  (int(rc));
  //
  Serial.print  (";   IP-address = ");
  Serial.print  (tmpBuf);
  Serial.println(";");
  // 
  gprsModul.powerOff();
  delay(15*1000);
}