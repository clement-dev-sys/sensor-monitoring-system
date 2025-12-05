#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// ===== CONFIG W5500 =====
#define ETH_CS    5
#define ETH_MOSI  23
#define ETH_MISO  19
#define ETH_SCK   18
#define ETH_RST   4

// ===== CONFIG RÉSEAU =====
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 69, 2);
IPAddress gateway(192, 168, 69, 1);
IPAddress subnet(255, 255, 255, 0);

// ===== CONFIG MQTT =====
IPAddress mqttServer(192, 168, 69, 1);
const int mqttPort = 1883;
const char* mqttTopic = "esp32/data";

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

// ===== DÉLAI =====
unsigned long previousMillis = 0;
const long interval = 20000;  // 20 secondes

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32 Publisher MQTT ===");
  
  pinMode(ETH_RST, OUTPUT);
  digitalWrite(ETH_RST, LOW);
  delay(100);
  digitalWrite(ETH_RST, HIGH);
  delay(200);
  
  SPI.begin(ETH_SCK, ETH_MISO, ETH_MOSI, ETH_CS);
  Ethernet.init(ETH_CS);
  
  Serial.println("Connexion Ethernet...");
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  delay(2000);
  
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("ERREUR: W5500 non détecté!");
    while (true) delay(1);
  }
  
  Serial.print("IP ESP32 : ");
  Serial.println(Ethernet.localIP());
  
  mqttClient.setServer(mqttServer, mqttPort);
  
  Serial.println("Prêt à envoyer des données toutes les 20s\n");
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connexion au broker MQTT...");
    
    if (mqttClient.connect("ESP32_Publisher")) {
      Serial.println(" OK !");
    } else {
      Serial.print(" Échec (code ");
      Serial.print(mqttClient.state());
      Serial.println("). Nouvelle tentative dans 5s");
      delay(5000);
    }
  }
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();
  
  // Envoyer données toutes les 20 secondes
  unsigned long currentMillis = millis();
  
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    int nombre1 = random(0, 10);
    int nombre2 = random(10, 20);
    int nombre3 = random(20, 30);
    int nombre4 = random(30, 40);
    
    char message[100];
    snprintf(message, sizeof(message), "%d,%d,%d,%d", nombre1, nombre2, nombre3, nombre4);
    
    Serial.print("Envoi: ");
    Serial.println(message);
    
    if (mqttClient.publish(mqttTopic, message)) {
      Serial.println("Envoyé");
    } else {
      Serial.println("Échec");
    }
    
    Serial.println();
  }
}
