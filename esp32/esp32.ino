#include <SPI.h>
#include <Ethernet.h>

#define ETH_CS    5
#define ETH_MOSI  23
#define ETH_MISO  19
#define ETH_SCK   18
#define ETH_RST   4

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 69, 2);
IPAddress gateway(192, 168, 69, 1);
IPAddress subnet(255, 255, 255, 0);

EthernetServer server(80);

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== Test ESP32 + W5500 ===");
  
  pinMode(ETH_RST, OUTPUT);
  digitalWrite(ETH_RST, LOW);
  delay(100);
  digitalWrite(ETH_RST, HIGH);
  delay(200);
  
  SPI.begin(ETH_SCK, ETH_MISO, ETH_MOSI, ETH_CS);
  Ethernet.init(ETH_CS);
  
  Serial.println("Démarrage de la connexion Ethernet...");
  Ethernet.begin(mac, ip, gateway, gateway, subnet);
  
  delay(2000);
  
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("ERREUR: Module W5500 non détecté!");
    Serial.println("Vérifiez les connexions SPI");
    while (true) delay(1);
  }
  
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("ATTENTION: Câble Ethernet non connecté");
  }
  
  Serial.println("\n--- Configuration réseau ---");
  Serial.print("Adresse IP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("Masque: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("Passerelle: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("DNS: ");
  Serial.println(Ethernet.dnsServerIP());
  
  server.begin();
  Serial.println("\nServeur HTTP démarré!");
  Serial.println("Testez depuis votre PC avec:");
  Serial.print("  curl http://");
  Serial.println(Ethernet.localIP());
  Serial.print("  ou ouvrez http://");
  Serial.print(Ethernet.localIP());
  Serial.println(" dans un navigateur");
  Serial.println("\n=============================\n");
}

void loop() {
  static bool wasConnected = true;
  bool isConnected = (Ethernet.linkStatus() == LinkON);
  
  if (isConnected != wasConnected) {
    wasConnected = isConnected;
    if (isConnected) {
      Serial.println("Câble Ethernet connecté!");
    } else {
      Serial.println("Câble Ethernet déconnecté!");
    }
  }
  
  EthernetClient client = server.available();
  
  if (client) {
    Serial.println("\n>>> Nouveau client connecté");
    boolean currentLineIsBlank = true;
    String request = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        
        if (c == '\n' && currentLineIsBlank) {
          Serial.println("Requête reçue:");
          Serial.println(request);
          
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html; charset=utf-8");
          client.println("Connection: close");
          client.println();
          
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          client.println("<head><title>ESP32 + W5500</title></head>");
          client.println("<body>");
          client.println("<h1>✓ Connexion réussie!</h1>");
          client.println("<p>Votre ESP32 avec module W5500 fonctionne correctement.</p>");
          client.println("<hr>");
          client.println("<p><strong>Informations:</strong></p>");
          client.println("<ul>");
          client.print("<li>IP: ");
          client.print(Ethernet.localIP());
          client.println("</li>");
          client.print("<li>Uptime: ");
          client.print(millis() / 1000);
          client.println(" secondes</li>");
          client.println("</ul>");
          client.println("</body>");
          client.println("</html>");
          
          break;
        }
        
        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    
    delay(1);
    client.stop();
    Serial.println("<<< Client déconnecté\n");
  }
}