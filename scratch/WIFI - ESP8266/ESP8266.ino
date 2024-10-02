#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> 


// Sustituir con datos de vuestra red
const char* ssid     = "SSID";
const char* password = "PASSWORD";

ESP8266WebServer server(80);

IPAddress ip(192,168,100,79);     
IPAddress gateway(192,168,100,1);   
IPAddress subnet(255,255,255,0);  

// Pin del LED
const int Puerta = 2;

void setup()
{
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid, password);
  Serial.print("Conectando a:\t");
  Serial.println(ssid); 

  // Esperar a que nos conectemos
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(200);
  Serial.print('.');
  }
  
  WiFi.setAutoReconnect(true);

  // Mostrar mensaje de exito y dirección IP asignada
  Serial.println();
  Serial.print("Conectado a:\t");
  Serial.println(WiFi.SSID()); 
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  pinMode(Puerta, OUTPUT);
  digitalWrite(Puerta, LOW);

  // Maneja las solicitudes HTTP
    server.on("/door/open", []() {
      digitalWrite(Puerta, LOW);
      server.send(200, "text/plain", "Puerta Abierta");
  });

    server.on("/door/closed", []() {
      digitalWrite(Puerta, HIGH);
      server.send(200, "text/plain", "Puerta Cerrada");
  });

  server.begin();
}
void loop() 
{
  server.handleClient();
}