#include "ArduinoJson.h"
#include "ESP8266WiFi.h"
#include "PubSubClient.h"
#include "DHT.h"


//Variables de conexion
const char* ssid = "STAR-LINK";
const char* password = "1234567890"; 
const char* rabbitmq_server = "fly.rmq.cloudamqp.com"; 
const int rabbitmq_port = 1883;               
const char* rabbitmq_user = "wyymaeak:wyymaeak";
const char* rabbitmq_password = "fDasodKlvvMTnipPrmFcbfHeHJaX07HM";
const char* rabbitmq_queue = "sensor_data"; 



//Variables Sensor
#define DHTPIN 2  
#define DHTTYPE DHT11   
DHT dht(DHTPIN, DHTTYPE);

//Iniciar WiFi
WiFiClient wifiClient;

//Conexion RabbitMQ (mqtt)
PubSubClient client(wifiClient);

void setup() {
  Serial.begin(9600);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando a la Red...");
  }
  Serial.println("Red conectada!");

  client.setServer(rabbitmq_server, rabbitmq_port);
  while (!client.connected()) {
    if (client.connect("ESP8266Client", rabbitmq_user, rabbitmq_password)) {
      Serial.println("Connected to RabbitMQ");
    } else {
      Serial.println("Failed to connect to RabbitMQ");
      delay(2000);
    }
  }
  dht.begin();
}

void loop() {

  // RabbitMQ conexion
  if (!client.connected()) {
    if (client.connect("ESP8266Client", rabbitmq_user, rabbitmq_password)) {
      Serial.println("Connected to RabbitMQ");
    } else {
      Serial.println("Failed to connect to RabbitMQ");
      delay(2000);
    }
  }

  // Leer los datos del sensor DHT11
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Error al leer el sensor DHT11!");
    return;
  }

  // Crear un objeto JSON con los datos del sensor
  StaticJsonDocument<200> jsonDoc;
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;

  // Convertir el objeto JSON en una cadena JSON
  char jsonStr[200];
  serializeJson(jsonDoc, jsonStr);

  Serial.println(client.publish(rabbitmq_queue, jsonStr));
  // Enviar los datos a RabbitMQ
  client.publish(rabbitmq_queue, jsonStr);
  client.subscribe(rabbitmq_queue);
  delay(10000);
}