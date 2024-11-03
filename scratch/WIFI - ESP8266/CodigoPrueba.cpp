#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <FS.h>

ESP8266WebServer server(80);

// Pin del LED
const int Puerta = 2;
String registeredUser;
String registeredPass;
String adminUser = "admin";  // Usuario administrador predeterminado
String adminPass = "admin123"; // Contraseña del administrador predeterminado
bool isLoggedIn = false;

void setup() {
  Serial.begin(115200);
  delay(10);
  
  WiFi.mode(WIFI_AP_STA);
  // Creamos una instancia de la clase WiFiManager
  WiFiManager wiFiManager;
  //Resetea la configuración
  wiFiManager.resetSettings();

  if (!wiFiManager.autoConnect("AutoConnectAP", "12345678")) {
    Serial.println("Fallo en la conexión, reiniciando...");
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

  // Inicializa SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Error al montar SPIFFS");
    return;
    }

    // Leer las credenciales del archivo
    readCredentials();
    //Definir Pin para verificar solicitud HTTP
    pinMode(Puerta, OUTPUT);
    digitalWrite(Puerta, LOW);

  server.on("/get_mac", []() {
    String mac = WiFi.macAddress();
    server.send(200,"text/plain",mac);
    });

    // Endpoint para login
    server.on("/login", HTTP_POST, []() {
      server.send(200, "text/plain", "Datos recibidos");
      }, []() {
        // Recupera los datos del POST
        if (server.hasArg("user") && server.hasArg("pass")) {
          String user = server.arg("user");
          String pass = server.arg("pass");
          if (user == adminUser && pass == adminPass) {
            void FuncionesAdmin();
            isLoggedIn = true;  // Establece la sesión como iniciada
            server.send(200,"text/plain", "Login Admin Exitoso");
                }
          // Validar el usuario y la contraseña
          if (user == registeredUser && pass == registeredPass) {
            isLoggedIn = true;  // Establece la sesión como iniciada
            server.send(200, "text/plain", "Login exitoso");
            } else {
              server.send(401, "text/plain", "Credenciales incorrectas");
              }
              } else {
                server.send(400, "text/plain", "Datos incorrectos");
                }
                });

    // Endpoint para abrir la puerta
    server.on("/door/open", HTTP_POST, []() {
      if (isLoggedIn) {
        server.send(200, "text/plain", "Datos recibidos");
        } else {
          server.send(403, "text/plain", "Acceso denegado. Debe iniciar sesión.");
          }
          }, []() {
            if (isLoggedIn) {
              // Recupera los datos del POST
              server.send(200, "text/plain", "Datos procesados");
              // Lógica para abrir la puerta
              digitalWrite(Puerta, LOW);
              server.send(200, "text/plain", "Puerta cerrada");
              delay(5000); // espera 5 seg la puerta abierta 
              digitalWrite(Puerta, HIGH);
              server.send(200, "text/plain", "Puerta abierta");
              } else {
                server.send(403, "text/plain", "Acceso denegado. Debe iniciar sesión.");
                }
                });
                
 server.begin();

    // Endpoint para agregar un nuevo usuario
    server.on("/add_user", HTTP_POST, []() {
      server.send(200, "text/plain", "Datos recibidos");
      }, []() {
        if (server.hasArg("user") && server.hasArg("pass")) {
          String user = server.arg("user");
          String pass = server.arg("pass");
          // Escribir las credenciales en el archivo
          File file = SPIFFS.open("/credentials.txt", "w"); //W: Escribe
          if (!file) {
            server.send(500, "text/plain", "Error al abrir el archivo");
            return;
            }
            file.println(user); // Guarda el nuevo usuario
            file.println(pass); // Guarda la nueva contraseña
            file.close();
            registeredUser = user; // Actualiza la variable global
            registeredPass = pass; // Actualiza la variable global
            server.send(200, "text/plain", "Usuario agregado correctamente");
            } else {
              server.send(400, "text/plain", "Datos incompletos");
              }
              });
    
    // Endpoint para borrar un usuario
    server.on("/delete_user", HTTP_POST, []() {
      server.send(200, "text/plain", "Datos recibidos");
      }, []() {
        if (server.hasArg("user")) {
          String userToDelete = server.arg("user");
          // Leer las credenciales actuales
          File file = SPIFFS.open("/credentials.txt", "r");
          if (!file) {
            server.send(500, "text/plain", "Error al abrir el archivo");
            return;
            }
            String currentUser = file.readStringUntil('\n');
            String currentPass = file.readStringUntil('\n');
            file.close();
            // Si el usuario a eliminar coincide con el registrado, lo borramos
            if (userToDelete == currentUser) {
              File newFile = SPIFFS.open("/credentials.txt", "w");
              if (!newFile) {
                server.send(500, "text/plain", "Error al abrir el archivo para escribir");
                return;
                }
                // Aquí se puede optar por dejar el archivo vacío o manejar la lógica
                newFile.println(""); // O simplemente no escribir nada
                newFile.close();
                registeredUser = ""; // Limpiar la variable global
                registeredPass = ""; // Limpiar la variable global
                server.send(200, "text/plain", "Usuario eliminado correctamente");
                } else {
                  server.send(404, "text/plain", "Usuario no encontrado");
                  }
                  } else {
                    server.send(400, "text/plain", "Datos incompletos");
                    }
                    });
}
void loop(){
  server.handleClient();
  }
// Función para leer las credenciales desde el archivo
void readCredentials() {
  File file = SPIFFS.open("/credentials.txt", "r"); //r: Lee
  if (!file) {
    // Lee las líneas del archivo
    registeredUser = file.readStringUntil('\n');
    registeredPass = file.readStringUntil('\n');
    Serial.println("Registered user: " + registeredUser);
    Serial.println("Registered password: " + registeredPass);
    file.close();
    } else {
    Serial.println("Error al abrir el archivo");
    return;
  }
}
void FuncionesAdmin() {
  // Endpoint de administración para agregar un nuevo usuario
    server.on("/admin/add_user", HTTP_POST, []() {
        server.send(200, "text/plain", "Datos Recibidos");
        }, []() {
          if (isLoggedIn) { // Se confima que el admin inicio sesión
          if (server.hasArg("user") && server.hasArg("pass")) {
            String user = server.arg("user");
            String pass = server.arg("pass");
            
            //Guardar usuarios y contraseñas
            File file = SPIFFS.open("/credentials.txt", "a"); // modo agregar
            if (file) {
              file.println(user);
              file.println(pass);
              file.close();
              server.send(200, "text/plain", "Usuario Agregado Correctamente.");
              } else {
                server.send(500, "text/plain", "Error al Abrir Archivo.");
                }
                } else {
                  server.send(400, "text/plain", "Datos Incompletos.");
                  }
                  } else {
                    server.send(403, "text/plain", "Acceso Denegado.");
                    }
                    });                  
  // Endpoint de administración para eliminar un usuario
  server.on("/admin/delete_user", HTTP_POST, []() {
    server.send(200, "text/plain", "Datos Recibidos");
    }, []() {
      if (isLoggedIn) { // Se confima que el admin inicio sesión
      if (server.hasArg("user")) {
        String userToDelete = server.arg("user");
        
        // Leer las credenciales actuales
        File file = SPIFFS.open("/credentials.txt", "r");
        if (file) {
          String line;
          String currentUser, currentPass;
          bool userFound = false;
          
          // Crea un archivo temporal para guardar a los usuarios restantes
          File tempFile = SPIFFS.open("/temp_credentials.txt", "w");
          
          while (file.available()) {
            currentUser = file.readStringUntil('\n');
            currentPass = file.readStringUntil('\n');
            
            if (currentUser == userToDelete) {
              userFound = true; // Usuario encontrado y no será escrito en el archivo temporal
              } else {
                // Vuelve a escribir el usuario en el archivo temporal
                tempFile.println(currentUser);
                tempFile.println(currentPass);
                }
                }
                
                file.close();
                tempFile.close();
                
                // Reemplazar el archivo de credenciales antiguo por el nuevo
                SPIFFS.remove("/credentials.txt");
                SPIFFS.rename("/temp_credentials.txt", "/credentials.txt");
                if (userFound) {
                  server.send(200, "text/plain", "Usuario Eliminado Correctamente.");
                  } else {
                    server.send(404, "text/plain", "Usuario No Encontrado.");
                    }
                    } else {
                      server.send(500, "text/plain", "Error al Abrir Datos.");
                      }
                      } else {
                        server.send(400, "text/plain", "Datos Incompletos.");
                        }
                        } else {
                          server.send(403, "text/plain", "Acceso Denegado.");
                          }
                          });
                          }