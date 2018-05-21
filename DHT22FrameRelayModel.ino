#include "DHT.h"
#include "SerialCommand.h"
using namespace std;

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

SerialCommand SCmd;

// настройка командного интерфейса
void setupSerialCommands(){
  SCmd.addCommand("AT",AT_on);              // Переход в командный режим
  SCmd.addCommand("AT+Q",AT_off);           // Выход из командного режима
  SCmd.addCommand("AT+PARAM",AT_param);     // Установка параметров передачи
  SCmd.addCommand("AT+LIST",AT_list);      // Отображение текущих параметров
  SCmd.addDefaultHandler(unrecognized);     // Обработчик неизвестной команды
}

// Переход в командный режим
void AT_on(){
  char *arg;
  arg = SCmd.next();
  if (arg != NULL)  
  {
    Serial.println(arg);
  } else {
    if (deviceMode == TRANSMIT_MODE){
      deviceMode = SETUP_MODE;
      Serial.println("OK");
    } else {
      Serial.println("IN SETUP");
    }
  }
}

// Выход из командного режима
void AT_off(){
  if (deviceMode == SETUP_MODE){
    deviceMode = TRANSMIT_MODE;
    Serial.println("READY");
  } else {
    Serial.println("IN TRANSMIT");
  }
}

void AT_list(){
    Serial.print("DLCI0="); 
    Serial.println(frameAddr.DLCI0);
    Serial.print("C_R="); 
    Serial.println(frameAddr.C_R);
    Serial.print("EA0="); 
    Serial.println(frameAddr.EA0);    
    Serial.print("DLCI1="); 
    Serial.println(frameAddr.DLCI1);
    Serial.print("FECN="); 
    Serial.println(frameAddr.FECN);
    Serial.print("BECN="); 
    Serial.println(frameAddr.BECN);
    Serial.print("DE="); 
    Serial.println(frameAddr.DE);
    Serial.print("EA1=");
    Serial.println(frameAddr.EA1);
}

// установка параметров
void AT_param(){
  if (deviceMode == TRANSMIT_MODE){
      Serial.println("IN TRANSMIT");
  } else {
      char *param_name;
      param_name = SCmd.next();
      if (param_name != NULL)  
      {
        String pname(param_name);
        char *param_val = SCmd.next();
        if (param_val != NULL){
          if (pname == String("DLCI0")){
                int new_DLCI0 = atoi(param_val);
                if (new_DLCI0 >= 0 && new_DLCI0<=31){
                  frameAddr.DLCI0 = new_DLCI0;
                  Serial.print("DLCI0=");
                  Serial.println(new_DLCI0);
                } else {
                  Serial.println("WRONG DLCI0 [0..31]");
                }
          } else if (pname == String("C_R")){
                int new_C_R = atoi(param_val);
                if (new_C_R >= 0 && new_C_R<=1){
                  frameAddr.C_R = new_C_R;
                  Serial.print("C_R=");
                  Serial.println(new_C_R);
                } else {
                  Serial.println("WRONG C_R [0..1]");
                }
          } else if (pname == String("EA0")){
                int new_EA0 = atoi(param_val);
                if (new_EA0 >= 0 && new_EA0<=1){
                  frameAddr.EA0 = new_EA0;
                  Serial.print("EA0=");
                  Serial.println(new_EA0);
                } else {
                  Serial.println("WRONG EA0 [0..1]");
                }
          } else if (pname == String("DLCI1")){
                int new_DLCI1 = atoi(param_val);
                if (new_DLCI1 >= 0 && new_DLCI1<=15){
                  frameAddr.DLCI1 = new_DLCI1;
                  Serial.print("DLCI1=");
                  Serial.println(new_DLCI1);
                } else {
                  Serial.println("WRONG DLCI0 [0..15]");
                }
          } else if (pname == String("FECN")){
                int new_FECN = atoi(param_val);
                if (new_FECN >= 0 && new_FECN<=1){
                  frameAddr.FECN = new_FECN;
                  Serial.print("FECN=");
                  Serial.println(new_FECN);
                } else {
                  Serial.println("WRONG FECN [0..1]");
                }
          } else if (pname == String("BECN")){
                int new_BECN = atoi(param_val);
                if (new_BECN >= 0 && new_BECN<=1){
                  frameAddr.BECN = new_BECN;
                  Serial.print("BECN=");
                  Serial.println(new_BECN);
                } else {
                  Serial.println("WRONG BECN [0..1]");
                }
          } else if (pname == String("DE")){
                int new_DE = atoi(param_val);
                if (new_DE >= 0 && new_DE<=1){
                  frameAddr.DE = new_DE;
                  Serial.print("DE=");
                  Serial.println(new_DE);
                } else {
                  Serial.println("WRONG DE [0..1]");
                }
          } else if (pname == String("EA1")){
                int new_EA1 = atoi(param_val);
                if (new_EA1 >= 0 && new_EA1<=1){
                  frameAddr.EA1 = new_EA1;
                  Serial.print("EA1=");
                  Serial.println(new_EA1);
                } else {
                  Serial.println("WRONG EA1 [0..1]");
                }
          }
        } else {
          Serial.println("NO VALUE"); 
        }
      } else {
        Serial.println("NO PARAM");
      }     
  }

}


// Обработчик неизвестной команды
void readATCommand(){
  SCmd.readSerial();
}

// незнакомая команда
void unrecognized()
{
  Serial.println("WAT?"); 
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
