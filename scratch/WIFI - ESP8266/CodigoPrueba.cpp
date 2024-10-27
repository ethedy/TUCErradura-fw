#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FS.h>

ESP8266WebServer server(80);

// Pin del LED
const int Puerta = 2;
String registeredUser;
String registeredPass;

void setup() {
  Serial.begin(115200);
  delay(10);

  WiFi.mode(WIFI_AP_STA);
  // Creamos una instancia de la clase WiFiManager
  WiFiManager wiFiManager;
  //Resetea la configuración
  wiFiManager.resetSettings();

  if (!wiFiManager.autoConnect("AutoConnectAP", "12345678")) {
    Serial.println("Fallo en la conexión");
    delay(3000);
    ESP.restart(); // Reiniciar si la conexión falla
  } else {
    Serial.println();
    Serial.print("Conectado a:\t");
    Serial.println(WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());
    Serial.println("Mac Address: /t");
    Serial.println(WiFi.macAddress());
  }

  //Definir Pin para verificar solicitud HTTP
  // Inicializa SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Error al montar SPIFFS");
    return;
  }

  // Leer las credenciales del archivo
  readCredentials();
  pinMode(Puerta, OUTPUT);
  digitalWrite(Puerta, LOW);

  // Maneja las solicitudes HTTP
    server.on("/door/open", HTTP_POST,  []() {
    server.send(200, "text/plain", "Datos recibidos");
  }, []() {
    // Recupera los datos del POST
    if (server.hasArg("user") && server.hasArg("pass")) {
     String user = server.arg("user");
     String pass = server.arg("pass");
      
     server.send(200, "text/plain", "Datos procesados");
      
        //Validar datos de request
       if (user == registeredUser && pass == registeredPass) {
            server.send(200, "text/plain", "Acceso Correcto");
            digitalWrite(Puerta, LOW);
            delay(5000); //espera 5seg la puerta abierta 
            digitalWrite(Puerta, HIGH);
        }
        else{
          server.send(200, "text/plain", "Acceso Denegado");
        }
  }
   else{
        server.send(200, "text/plain", "Datos incorrectos");
        });
        }
    server.on("/get_mac",[](){
    String mac = WiFi.macAddress();
    server.send(200,"text/plain",mac);    
   }
  server.begin();
}
void loop()
{
  server.handleClient();

}
// Función para leer las credenciales desde el archivo
void readCredentials() {
  File file = SPIFFS.open("/credentials.txt", "r");
  if (!file) {
    Serial.println("Error al abrir el archivo");
    return;
  }

  // Lee las líneas del archivo
  registeredUser = file.readStringUntil('\n');
  registeredPass = file.readStringUntil('\n');

  Serial.println("Usuario registrado: " + registeredUser);
  Serial.println("Contraseña registrada: " + registeredPass);

  file.close();
}