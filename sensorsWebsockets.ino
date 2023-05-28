#include "Focos.h"
#include <WiFiMulti.h>

WiFiMulti wifiMulti;

// Este objeto es donde se asignaran los valores de los sensores.

// En este String es donde se guardara el objeto anterior serializado.

String dataJson;
// Estas variables contienen los datos de los sensores
int humoValue;

// Estas variables contienen el estado del foco.
int flamaFoco;
int humoFoco;
int gasFoco;
int mqttFoco;

bool states[3];

Focos f;
void setup()
{
  Serial.begin(9600);

  pinMode(focoMqtt, OUTPUT);

  // WifiMulti aun no esta probado.
  wifiMulti.addAP("E6CA82", "L21503735312143");
  // wifiMulti.addAP("MEGACABLE_2.4G_B820", "R4Z5T5y7k8F5p7N5a2a2");
  wifiMulti.addAP("pruebaesp32", "universidad098");

  Serial.print("Conectando a la red WiFi...");

  connectToWiFi();

  // Asignar servidor y asignar la funcion de callback para MQTT.
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Setup de los sensores.
  f.luxoSetup();
  f.humoSetup();
  f.flamaSetup();
}

void loop()
{
  f.dataJson="";
  mqttListen();
  f.luxoListen();

  delay(100);
  f.flamaListen();

  f.humoListen();
  delay(100);

  sendIoT(dataJson);
  delay(100);
  newStates = f.getStates();
  delay(100);
  f.setStates(states[0],states[1],states[2]);

  delay(1000);

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Conexión WiFi perdida. Intentando reconectar...");
    connectToWiFi();
  }
}

void connectToWiFi()
{
  while (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.print("Conectando a ");
    Serial.println(WiFi.SSID());

    delay(1000);
  }

  Serial.println("Conectado a WiFi");
  Serial.print("Dirección IP: ");
  Serial.println(WiFi.localIP());
}