#include <Wire.h>
#include <BH1750.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Rangos para sensor de Flama
#define sensorMin 80
#define sensorMax 4095

// PINES sensor-ESP32
#define inputHumo 34
#define inputFlama 35

// PINES ESP32-foco
#define focoLuxometro 33
#define focoHumo 25
#define focoFlama 26
#define focoMqtt 27

BH1750 lightMeter;
bool ledState = false;

// Configuracion del servidor MQTT (Mosquitto Broker publico)
const char *mqtt_server = "test.mosquitto.org";
String palabra;
const int mqtt_port = 1883;

// Este cliente es de la libreria MultiWifi.h
WiFiClient espClient;
// Este cliente es para el MQTT
PubSubClient client(espClient);
DynamicJsonDocument doc(400);
JsonArray jsonArray = doc.to<JsonArray>();
// Configuracion Mqtt
class Focos
{
private:
  JsonObject luxData = doc.createNestedObject();
  JsonObject flamaData = doc.createNestedObject();
  JsonObject humoData = doc.createNestedObject();

public:
  String dataJson;
  // Mqtt
  void mqttSetup();
  void mqttListen();
  void reconnectMqtt();
  bool mqttFocoEstado;

  //  Luxometro
  void luxoSetup();
  void luxoListen();
  uint16_t lux;
  bool luxFocoEstado;
  // Humo
  void humoSetup();
  void humoListen();
  int valorHumo;
  bool humoFocoEstado;
  // Flama
  void flamaSetup();
  void flamaListen();
  int valorFlama;
  int range;
  bool flamaFocoEstado;
  void getStates();
  bool setStates(bool, bool, bool);
  bool states[3];
};


bool* Focos::getStates(){
  return this->states;
}

/*
void Focos::setStates(bool st1, bool st2, bool st3){
  pinMode(focoLuxometro, st1);
  pinMode(focoFlama, st2);
  pinMode(focoHumo, bool st3);
}
*/

void callback(char *topic, byte *payload, unsigned int length)
{
  String mensaje;
  String topicString = String(topic);

  /*
    Serial.print("Mensaje recibido [");
    Serial.print(topic);
    Serial.print("] ");
    */
  for (int i = 0; i < length; i++)
  {
    mensaje += (char)payload[i];
  }
  

  if (topicString == "focoEsli/api/foco1")
  {
    Serial.println(mensaje);    
  }  
  else if (topicString == "focoEsli/api/foco2")
  {
    Serial.println("foco2");
  }
  else if (topicString == "focoEsli/api/foco3")
  {
    Serial.println("foco3");
  }
  else if (topicString == "focoEsli/api/foco4")
  {
    Serial.println("foco4");
  }
  else if (topicString == "focoEsli/foco5")
  {
    Serial.println("foco5");
  }
  mensaje = "";
  /*
  for (int i = 0; i < length; i++)
    {
      Serial.print((char)payload[i]);
    }


  ledState = !ledState;
  digitalWrite(focoMqtt, ledState);\
  */
}


void reconnectMqtt()
{
  while (!client.connected())
  {
    Serial.print("Conectando al servidor MQTT...");

    // Intenta conectarse
    if (client.connect(mqtt_server))
    {
      Serial.println("conectado");

      // Suscribirse a un tema
      client.subscribe("focoEsli/api/#");
    }
    else
    {
      Serial.print("fallo, rc=");
      Serial.print(client.state());
      Serial.println(" esperando 5 segundos antes de volver a intentar");
      // Espera 5 segundos antes de intentar nuevamente
      delay(5000);
    }
  }
}

void mqttListen()
{
  if (!client.connected())
  {
    reconnectMqtt();
  }
  client.loop();
}

// FLAMA
void Focos::flamaListen()
{

  this->valorFlama = analogRead(inputFlama);
  Serial.println(this->valorFlama);

  this->range = map(this->valorFlama, sensorMax, sensorMin, 0, 3);
  Serial.println(this->range);
  // Serial.println(sensorReading);
  // La sensibilidad del sensor esta muy alta, hay que bajarla y cambiar los valores del range.
  switch (this->range)
  {
  case 0:
    Serial.println("No hay fuego");
    this->flamaFocoEstado = false;
    //digitalWrite(focoFlama, this->flamaFocoEstado);
    this->states[1]=this->flamaFocoEstado;
    break;
  case 1 || 2:
    Serial.println("Fuego lejano");
    this->flamaFocoEstado = false;
    //digitalWrite(focoFlama, this->flamaFocoEstado);
    states[1]=this->flamaFocoEstado;
    break;
  case 3:
    Serial.println("Fuego");
    this->flamaFocoEstado = true;
    //digitalWrite(focoFlama, this->flamaFocoEstado);
    this->states[1]=this->flamaFocoEstado;
    break;
  }
  this->flamaData["name"] = "flama";
  this->flamaData["value"] = this->range;
  this->flamaData["state"] = this->flamaFocoEstado;
}
void Focos::flamaSetup()
{
  pinMode(focoFlama, OUTPUT);
}

// Humo
void Focos::humoSetup()
{
  pinMode(inputHumo, INPUT);
  pinMode(focoHumo, OUTPUT);
}
void Focos::humoListen()
{

  this->valorHumo = analogRead(inputHumo);
  Serial.println(this->valorHumo);
  if (this->valorHumo > 1000)
  {
    humoFocoEstado = true;
    //digitalWrite(focoHumo, humoFocoEstado);
    states[2]=this->flamaFocoEstado;
    Serial.println("Humo detectado");
  }
  else
  {
    humoFocoEstado = false;
    //digitalWrite(focoHumo, humoFocoEstado);
    states[2]=this->flamaFocoEstado;
    Serial.println("Humo no detectado");
  }
  this->humoData["name"] = "humo";
  this->humoData["value"] = this->valorHumo;
  this->humoData["state"] = this->humoFocoEstado;
}

// Luxometro
void Focos::luxoSetup()
{

  Wire.begin();
  if (lightMeter.begin())
  {
    Serial.println("Conectado");
  }
  else
  {
    Serial.println("No conectado");
  }
  pinMode(focoLuxometro, OUTPUT);
}
void Focos::luxoListen()
{
  this->lux = lightMeter.readLightLevel();
  if (this->lux > 100)
  {
    Serial.print("Luminosidad: ");
    Serial.print(this->lux);
    Serial.println(" lux");
    luxFocoEstado = false;
   // digitalWrite(focoLuxometro, luxFocoEstado);
   this->states[0]=this->flamaFocoEstado;
  }
  else
  {
    Serial.print("Luminosidad: ");
    Serial.print(this->lux);
    Serial.println(" lux");
    this->luxFocoEstado = true;
   // digitalWrite(focoLuxometro, this->luxFocoEstado);
   this->states[0]=this->flamaFocoEstado;
  }

  this->luxData["name"] = "lux";
  this->luxData["value"] = this->lux;
  this->luxData["state"] = this->luxFocoEstado;
}

void  sendIoT(String dataJson){
    serializeJson(jsonArray, dataJson);
    client.publish("focoEsli/datos", dataJson.c_str());
}
