#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Credenciales del WAIFAI
const char* nombre = "nico l";
const char* password = "nico523146";

// CONFIGS DEL MQTT
const char* mqtt_server = "broker.emqx.io";
const int mqtt_puerto = 1883;
const char* mqtt_topic = "stm/grupo/sensores";

// Coneccion WIFI a ESP32
WiFiClient espClient;
PubSubClient client(espClient);

// Configuración del sensor DHT11
#define DHTPIN 4         // Pin conectado
#define DHTTYPE DHT11    // Tipo de sensor DHT
DHT dht(DHTPIN, DHTTYPE);

// Declaracion de PINs para potenciometro, LEDs y botón
const int potenciometro = 34;
const int ledVerde = 15;
const int ledAmarillo = 18;
const int ledRojo = 5;
const int ledAzul = 21;




void setup() {
  Serial.begin(115200);

  // Configurar pines
  pinMode(potenciometro, INPUT);
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarillo, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  

  // Apagar LEDs al inicio
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAmarillo, LOW);
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledAzul, LOW);

  // Inicializar el sensor DHT
  dht.begin();

  // Conectar al WiFi
  WiFi.begin(nombre, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando al WIFI");
  }
  Serial.println("Conexión WIFI funcional");

  // Conectar al broker MQTT
  client.setServer(mqtt_server, mqtt_puerto);
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado a EMQX");
    } else {
      Serial.print("Falló: Error = ");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void loop() {
  if (!client.connected()) {
    client.connect("ESP32Client");
  }
  client.loop();

  int valorPotenciometro = analogRead(potenciometro);
  float SensorCO2 = (valorPotenciometro / 4095.0) * 100.0;

  float SensorTMP = dht.readTemperature();
  float SensorHMD = dht.readHumidity();

 

 
 

  // Control de LEDs según CO2
  if (SensorCO2 < 10) {
    digitalWrite(ledVerde, HIGH);
    digitalWrite(ledAmarillo, LOW);
    digitalWrite(ledRojo, LOW);
    digitalWrite(ledAzul, LOW);

  } else if (SensorCO2 < 60) {
    digitalWrite(ledAmarillo, HIGH);
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledRojo, LOW);
    digitalWrite(ledAzul, HIGH);
    delay(1000);
    digitalWrite(ledAzul, LOW);
    delay(1000);
    
  } else if(SensorCO2 > 60){
    digitalWrite(ledRojo, HIGH);
    digitalWrite(ledAzul, HIGH);
    digitalWrite(ledVerde, LOW);
    digitalWrite(ledAmarillo, LOW);
    
  }

  String payload = "{";
  payload += "\"CO2\":" + String(SensorCO2, 1) + ",";
  payload += "\"Temperatura\":" + String(SensorTMP, 1) + ",";
  payload += "\"Humedad\":" + String(SensorHMD, 1);
  payload += "}";

  client.publish(mqtt_topic, payload.c_str());
  Serial.println("MQTT OUT: " + payload);

  delay(5000);
}