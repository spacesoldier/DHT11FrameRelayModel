#include "DHT.h"
#include "SerialCommand.h"

#define SETUP_MODE 0
#define TRANSMIT_MODE 1
uint8_t deviceMode = TRANSMIT_MODE;

// датчик влажности и температуры
#define DHTPIN 2
DHT dht(DHTPIN, DHT22);

// вызов функции по расписанию
void checkTime(int period, unsigned long& old_time, void callback(void)){
  // если с момента вызова прошло заданное время
  if (millis() - old_time > period){
    // вызываем функцию
    callback();
    // запоминаем момент вызова
    old_time = millis();
  }
}

// ------------\/\/-- DHT 22 --\/\/----------------------------------
// период опроса датчика температуры и влажности
int checkDhtPeriod = 2000;
// момент последнего опроса датчика температуры и влажности
unsigned long old_time_checkDHT = 0;

// инициализация датчика температуры и влажности
void initAtmosphereSensor(){
    dht.begin(); 
}

// влажность
float humid = 0.0;
// температура
float temp = 0.0;

// Опрос датчика температуры и влажности
void getAtmosphereParams(){
  //Считываем влажность
    float new_humid = dht.readHumidity();

    // Считываем температуру
    float new_temp = dht.readTemperature();

    // Проверка корректности полученных данных
    if (!isnan(new_humid) || !isnan(new_temp)) {
    humid = new_humid;
        temp = new_temp;
    }
}
// --------------^^^ DHT 22 ^^^ -------------------------------------


// ----------\/\/-- FRAME RELAY --\/\/-------------------------------
// период отправки данных
int sendDataPeriod = 2000;
// момент последней отправки данных
unsigned long old_time_sendData = 0;


// структура - адрес отправки пакета
struct FrameAddress {  
   unsigned short DLCI0  : 6;    // 0..31  (6 bits)  
   unsigned short C_R  : 1;    // 0..1 (1 bit)   
   unsigned short EA0    : 1;    // 0..1 (1 bit) 
   unsigned short DLCI1  : 4;    // 0..15 (4 bits)  
   unsigned short FECN   : 1;    // 0..1 (1 bit)
   unsigned short BECN   : 1;    // 0..1 (1 bit)
   unsigned short DE     : 1;    // 0..1 (1 bit)
   unsigned short EA1    : 1;    // 0..1 (1 bit)
}; 


// байт, который обозначает начало и конец кадра
uint8_t frameStartEnd = 0x7E; // 0b01111110
// Data Link Connection Identifier — идентификатор виртуального канала (PVC)
uint8_t DLCI0 = 20;
uint8_t DLCI1 = 22;

// адрес отправки пакета, заполнение структуры данными по умолчанию
struct frameAddr : FrameAddress { frameAddr() { 
    DLCI0 = DLCI0;
    C_R = 0;
    EA0 = 1;
    DLCI1 = DLCI1;
    FECN = 0;
    BECN = 0;
    DE = 0;
    EA1 = 1;
} } frameAddr;


// отправка данных с датчика по протоколу Frame Relay
void sendFrameRelay(){
  if (deviceMode == TRANSMIT_MODE){
    // TODO: concat a byte array and write it to serial
  }
}




// ----------^^^ FRAME RELAY ^^^-------------------------------------

// ----------\/\/-- COMMANDER --\/\/---------------------------------
int checkATCommandPeriod = 100;
unsigned long old_time_checkAT = 0;

//https://github.com/scogswell/ArduinoSerialCommand/blob/master/examples/SerialCommandHardwareOnlyExample/SerialCommandHardwareOnlyExample.ino

// настройка командного интерфейса
void setupSerialCommands(){
  
}

void readATCommand(){
  
}

// ----------^^^ COMMANDER ^^^---------------------------------------



void setup()
{
  Serial.begin(9600);
  initAtmosphereSensor();
  setupSerialCommands();
}

void loop()
{
    // вызов командного интерфейса
    checkTime(checkATCommandPeriod, old_time_checkAT, readATCommand);
    // опрос датчика температуры и влажности
    checkTime(checkDhtPeriod, old_time_checkDHT, getAtmosphereParams);
    // отправка данных с датчика по протоколу Frame Relay
    checkTime(sendDataPeriod, old_time_sendData, sendFrameRelay);
}
