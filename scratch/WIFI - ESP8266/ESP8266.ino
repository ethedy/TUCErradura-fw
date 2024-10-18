#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h> 
#include <WiFiManager.h>
#include <WiFiClient.h>

ESP8266WebServer server(80); 

// Pin del LED
const int Puerta = 2;

const char* serverIp = "http://tu-servidor.com/guardar_mac"; // Cambia esto por tu servidor

void setup(){
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_AP_STA);
  WiFiManager wiFiManager;
  wiFiManager.resetSettings();
  
  if (!wiFiManager.autoConnect("ESP8266 Config", "12345678")) {
    Serial.println("Fallo en la conexión");
    delay(3000);
    ESP.restart(); // Reiniciar si la conexión falla
  }else{
      Serial.println();
  Serial.print("Conectado a:\t");
  Serial.println(WiFi.SSID()); 
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address:\t");
  Serial.println(WiFi.macAddress());
      }
       // Envía la MAC a tu servidor
    WiFiClient client;
    if (client.connect(serverIp, 80)) {
      client.print(String("POST /guardar_mac HTTP/1.1\r\n") +
                   "Host: " + serverIp + "\r\n" +
                   "Content-Type: application/x-www-form-urlencoded\r\n" +
                   "Connection: close\r\n\r\n" +
                   "mac=" + WiFi.macAddress());
      delay(100); // Espera a que se envíe el mensaje
    }
  }
  


  //Definir Pin para verificar solicitud HTTP
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

    server.on("/get_mac",[](){
      String mac = WiFi.macAddress();
      server.send(200, "text/plain", mac);
    });

  server.begin();
}

void loop() 
{
  server.handleClient();
  
}
