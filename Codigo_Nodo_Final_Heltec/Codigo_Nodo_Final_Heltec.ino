// !!!! Codigo para el Heltec Wireless Stick !!!!
#include "Sensores_Proyecto.h"
#include <TickTwo.h>
#include <TTN_esp32.h>
#include "TTN_CayenneLPP.h"

//Objeto TTN para el envio de datos
TTN_esp32 ttn ;
//Objeto LPP para el almacenamiento de los datos
TTN_CayenneLPP lpp;

const char* devAddr = "XXXXXXXXXX"; // Change to TTN Device Address
const char* nwkSKey = "XXXXXXXXXX"; // Change to TTN Network Session Key
const char* appSKey = "XXXXXXXXXX"; // Change to TTN Application Session Key

//DEFINICION DE LOS VALORES DE REFERENCIA PARA LOS SENSORES
const int LIGHT_REFERENCE = 400;

const float TEMPERATURE_MIN_REFERENCE = 10.0;
const float TEMPERATURE_MAX_REFERENCE = 30.0;

const int HUMIDITY_MIN_REFERENCE = 10;
const int HUMIDITY_MAX_REFERENCE = 90;

const int FLAMMABLE_MAX_REFERENCE = 200;

const int TOXIC_MAX_REFERENCE = 900;

//ASIGNACION DE PINES
#define PIN_DHT22_1 23
#define PIN_DHT11_1 17
#define PIN_DHT11_2 13
#define PIN_MQ135_1 15
#define PIN_MQ2_1 36
#define PIN_RELAY_1 25
#define PIN_RELAY_2 12
// ! Sensor BH1750 va conectado: SCL ---> 22 | SDA ---> 21 !


// DEFINICION DE LOS OBJETOS NODO PARA CADA VARIABLE A MONITORIZAR
NodeSensor NODO_TEMPERATURE(NodeSensor::VARIABLE_TEMPERATURE);
NodeSensor NODO_HUMIDITY(NodeSensor::VARIABLE_HUMIDITY);
NodeSensor NODO_TOXIC(NodeSensor::VARIABLE_TOXIC);
NodeSensor NODO_FLAMMABLE(NodeSensor::VARIABLE_FLAMMABLE);
NodeSensor NODO_LIGHT(NodeSensor::VARIABLE_LIGHT);

//DEFINICION DE LAS FUNCIONES PARA LEER DATOS DESDE LOS SENSORES Y OBTENER UN PROMEDIO
void Rutina_Temperatura();
void Rutina_Humedad();
void Rutina_Gases_Inflamables();
void Rutina_Gases_Toxicos();
void Rutina_Iluminacion();
void Rutina_Elemento_Control();
void message(const uint8_t* payload, size_t size, int rssi);

//DEFINICION DE FUNCION PARA EL ENVIO DEL PAYLOAD HACIA TTN
void SendPayload();

//DEFINICION DE LOS TICKTWOS QUE EJECUTARAN LA LECTURA DE LOS SENSORES EN UN TIEMPO DETERMINADO
TickTwo RUTINA_TEMP(Rutina_Temperatura, 15000); //En micros segundos
TickTwo RUTINA_HUM(Rutina_Humedad, 40000); //En micros segundos
TickTwo RUTINA_MQ135(Rutina_Gases_Toxicos, 50000); //En micros segundos
TickTwo RUTINA_MQ2(Rutina_Gases_Inflamables, 6000); //En micros segundos
TickTwo RUTINA_LUZ(Rutina_Iluminacion, 20000); //En micros segundos

//TickTwo para el envio de la informacion
TickTwo RUTINA_ENVIO_TTN(SendPayload, 126000);



//DEFINICION DE LOS ELEMENTRO DE CONTROL
Control_Element RELAY_1(PIN_RELAY_1);
Control_Element RELAY_2(PIN_RELAY_2);


//DEFINICION DE LOS PINES Y TIPO DE SENSOR
//----------Para sensores DHT22----------
Sensor Sensor_DHT22_1(PIN_DHT22_1, Sensor::DHT_22);

//----------Para sensores DHT11----------
Sensor Sensor_DHT11_1(PIN_DHT11_1,Sensor::DHT_11);
Sensor Sensor_DHT11_2(PIN_DHT11_2,Sensor::DHT_11);

//----------Para sensores MQ_135----------
Sensor SensorGasMQ135_1(PIN_MQ135_1,Sensor::MQ_135);

//----------Para sensores MQ_2----------
Sensor SensorGasMQ2_1(PIN_MQ2_1,Sensor::MQ_2);

//----------Para sensores BH1750----------
Sensor SensorBH1750_1(Sensor::BH_1735);

void setup() {
  Serial.begin(115200);
  Serial.println(F("Iniciado"));
  ttn.begin();
  ttn.onMessage(message); 
  ttn.personalize(devAddr, nwkSKey, appSKey);
  ttn.showStatus();
  RUTINA_TEMP.start();
  RUTINA_HUM.start();
  RUTINA_MQ135.start();
  RUTINA_MQ2.start();
  RUTINA_LUZ.start();
  RUTINA_ENVIO_TTN.start();
}

void loop() {
  RUTINA_TEMP.update();
  RUTINA_HUM.update();
  RUTINA_MQ135.update();
  RUTINA_MQ2.update();
  RUTINA_LUZ.update();
  RUTINA_ENVIO_TTN.update();
  if(RUTINA_ENVIO_TTN.counter() == 4){
    //Serial.println(F("Cambio del intervalo"));
    RUTINA_ENVIO_TTN.interval(600000);
    RUTINA_TEMP.interval(180000);
    RUTINA_HUM.interval(240000);
    RUTINA_MQ135.interval(270000);
    RUTINA_MQ2.interval(300000);
    RUTINA_LUZ.interval(300000);
  } 
}

void Rutina_Temperatura(){
  Sensor_DHT22_1.readValueSensor();
  if(!NODO_TEMPERATURE.addValue(Sensor_DHT22_1.getValueSensor(Sensor::DHT_22_TEMPERATURE))){
      Serial.println(F("- Error de lectura de temperatura en el sensor DHT22_1"));
  }
}

void Rutina_Humedad(){
  Sensor_DHT22_1.readValueSensor();
  if(!NODO_HUMIDITY.addValue(Sensor_DHT22_1.getValueSensor(Sensor::DHT_22_HUMIDITY))){
    Serial.println(F("- Error en la lectura de la humedad en el sensor DHT22_1"));
  }
}


void Rutina_Gases_Inflamables(){
  SensorGasMQ2_1.readValueSensor();
  if(!NODO_FLAMMABLE.addValue(SensorGasMQ2_1.getValueSensor())){
    Serial.println(F("- Error en la lectura de gases inflamables en el sensor MQ_2_1"));
  }
}

void Rutina_Gases_Toxicos(){
  SensorGasMQ135_1.readValueSensor();
  if(!NODO_TOXIC.addValue(SensorGasMQ135_1.getValueSensor())){
    Serial.println(F("- Error en la lectura de gases inflamables en el sensor MQ135_1"));
  }
}

void Rutina_Iluminacion(){
  SensorBH1750_1.readValueSensor();
  if(!NODO_LIGHT.addValue(SensorBH1750_1.getValueSensor())){
    Serial.println(F("- Error en la lectura de luminosidad del sensor BH_1735_1"));
  }
}

void SendPayload(){
    Serial.print(F("+ Promedio de temperatura: " ));
    NODO_TEMPERATURE.CalculateAvarageValue();
    Serial.println(NODO_TEMPERATURE.getAvarage());
    NODO_TEMPERATURE.resetCounterAvg();
    lpp.addTemperature(1, NODO_TEMPERATURE.getAvarage());
    /*---------------------------*/
    Serial.print(F("+ Promedio de humedad: "));
    NODO_HUMIDITY.CalculateAvarageValue();
    Serial.println(NODO_HUMIDITY.getAvarage());
    NODO_HUMIDITY.resetCounterAvg();
    lpp.addRelativeHumidity(1,int(NODO_HUMIDITY.getAvarage())/2);
    /*---------------------------*/
    Serial.print(F("+ Promedio del valor de luminosidad: "));
    NODO_LIGHT.CalculateAvarageValue();
    Serial.println(NODO_LIGHT.getAvarage());
    NODO_LIGHT.resetCounterAvg();
    lpp.addLuminosity(1, NODO_LIGHT.getAvarage());
    /*---------------------------*/
    Serial.print(F("+ Promedio de valor de deteccion de gases toxicos: "));
    NODO_TOXIC.CalculateAvarageValue();
    Serial.println(NODO_TOXIC.getAvarage());
    NODO_TOXIC.resetCounterAvg();
    lpp.addGasToxic(1, NODO_TOXIC.getAvarage());
    /*---------------------------*/
    Serial.print(F("+ Promedio de valor de deteccion de gases inflamables: "));
    NODO_FLAMMABLE.CalculateAvarageValue();
    Serial.println(NODO_FLAMMABLE.getAvarage());
    NODO_FLAMMABLE.resetCounterAvg();
    lpp.addGasFlamable(1, NODO_FLAMMABLE.getAvarage());
    /*---------------------------*/
    if (ttn.sendBytes(lpp.getBuffer(), lpp.getSize())){
    Serial.printf("TTN_CayenneLPP: %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", 
                  lpp.getBuffer()[0], lpp.getBuffer()[1],
                  lpp.getBuffer()[2], lpp.getBuffer()[3],lpp.getBuffer()[4],lpp.getBuffer()[5],
                  lpp.getBuffer()[6], lpp.getBuffer()[7], lpp.getBuffer()[8], lpp.getBuffer()[9]);
    Serial.println();
    }
    lpp.reset();
  
}

void message(const uint8_t* payload, size_t size, int rssi){
  Serial.println(F("Mensaje recibido: "));
  Serial.print("Recibido " + String(size) + " bytes RSSI= " + String(rssi) + "dB");
  for (int i = 0; i < size; i++)
  {
    Serial.print(" " + String(payload[i])+" ");
    // Serial.write(payload[i]);
  }
  Serial.println();
  if (payload[0] == 1) {
    Serial.println(F("\t+Elemento de control #1 encendido") );
    if(!RELAY_1.getState()) RELAY_1.setOnElement();
  }
  else if (payload[0] == 2){
    Serial.println(F("\t-Elemento de control #1 apagado") );
    RELAY_1.setOffElement();
  }
  if (payload[1] == 1) {
    Serial.println(F("\t+Elemento de control #2 encendido") );
    if(!RELAY_2.getState()) RELAY_2.setOnElement();
  }
  else if (payload[1] == 2){
    Serial.println(F("\t-Elemento de control #2 apagado") );
    RELAY_2.setOffElement();
  }
  Serial.println();
}

void Rutina_Elemento_Control(){
   //TEMPERATURA ALTA
  if (NODO_TEMPERATURE.getAvarage() > TEMPERATURE_MAX_REFERENCE){
    if (!RELAY_1.getState()){
      RELAY_1.setOnElement();
      Serial.println(F("\t - Abanicos encendidos"));
    }/*
    if(Relay_Inflarojos.getState()){
      Relay_Inflarojos.setOffElement();
      Serial.println(F("\t - Luces Inflarojas encendidas"));
    }*/
  }
  
  //TEMPERATURA BAJA
  if(NODO_TEMPERATURE.getAvarage() < TEMPERATURE_MIN_REFERENCE){
    if(RELAY_1.getState()){
      RELAY_1.setOffElement();
      Serial.println(F("\t - Abanicos apagados"));
    }/*
    if(!Relay_Inflarojos.getState()){
      Relay_Inflarojos.setOnElement();
      Serial.println(F("\t - Luces inflarojas encendidas"));
    }*/
  }
  /*
  //HUMEDAD RELATIVA BAJA
  if (NODO_HUMIDITY.getAvarage() <= HUMIDITY_MIN_REFERENCE){
    if (!Relay_Humificador.getState()){
      Relay_Humificador.setOnElement();
      Serial.println(F("\t - Humificador encendido"));
    }
  }

  //HUMEDAD RELATIVA ALTA
  if (NODO_HUMIDITY.getAvarage() >= HUMIDITY_MAX_REFERENCE){
    if(Relay_Humificador.getState()){
      Relay_Humificador.setOffElement();
      Serial.println(F("\t - Humificador apagado"));
    }
    if(!RELAY_1.getState()){
      RELAY_1.setOnElement();
      Serial.println(F("\t - Abanicos encendido"));
    }
    if(!Relay_Inflarojos.getState()){
      Relay_Inflarojos.setOnElement();
      Serial.println(F("\t - Luces inflarojas encendidas"));
    }      
  }

  //NIVEL DE GAS INFLAMABLE ALTO
  if (NODO_FLAMMABLE.getAvarage() > FLAMMABLE_MAX_REFERENCE){
    if(!RELAY_1.getState()){
      RELAY_1.setOnElement();
      Serial.println(F("\t - Abanicos encendidos [FLAMMABLE]"));
    } 
  }
  
  //NIVEL DE GASES TOXICOS ALTO
  if (NODO_TOXIC.getAvarage() > TOXIC_MAX_REFERENCE){
    if(!RELAY_1.getState())
      RELAY_1.setOnElement();
      Serial.println(F("\t - Abanicos encendidos"));
  }

  //ILUMINACION BAJA
  if (NODO_LIGHT.getAvarage() < LIGHT_REFERENCE){
    if (!Relay_Iluminacion.getState()){
      Relay_Iluminacion.setOnElement();
      Serial.println(F("\t - Luces encendidas "));
    }
  }
  else{
    Relay_Iluminacion.setOffElement();
    Serial.println(F("\t - Luces apagadas "));
  }
  */
}