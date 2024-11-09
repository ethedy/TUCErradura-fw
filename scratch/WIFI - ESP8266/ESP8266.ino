#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <vector>  // Correcto para usar std::vector
#include <SHA256.h>  // Librería para SHA256

ESP8266WebServer server(80);

unsigned long sessionLimit = 900000; // 15 minutos de tiempo para session activa
unsigned long lastLoginTime = 0;  // Para gestionar la expiración de la sesión
const int Puerta = 2; // Pin del LED

struct Session {
  String User;
  String PassHash; // Usaremos el hash de la contraseña
  String Rol; // "admin" o "user"
  String Token;     // Token de sesión único para cada usuario
};

std::vector<Session> UsuariosLogueados;  // Vector para almacenar los usuarios logueados

// Función para generar un token único
String generarToken() {
  String token = String(millis(), HEX) + String(random(0, 255), HEX);  // Usamos el tiempo y un valor aleatorio como token
  return token;
}

// Función para generar el hash SHA256 de la contraseña
String generarHash(const String& pass) {
  uint8_t hash[SHA256_SIZE];
  SHA256.begin();
  SHA256.print(pass);
  SHA256.end(hash);
  
  // Convertimos el hash a un string hexadecimal
  String hashStr = "";
  for (int i = 0; i < SHA256_SIZE; i++) {
    hashStr += String(hash[i], HEX);
  }
  return hashStr;
}

// Verificar usuario con hash de contraseña
bool verificarUsuario(const String& User, const String& Pass, Session& usuario) {
  for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
    if (UsuariosLogueados[i].User == User) {
      // Comparamos el hash de la contraseña
      if (UsuariosLogueados[i].PassHash == generarHash(Pass)) {
        usuario = UsuariosLogueados[i]; // Devolvemos la información del usuario encontrado
        return true;
      }
    }
  }
  return false;
}

// Función para manejar la solicitud de login
void handleLogin() {
  if (server.hasArg("user") && server.hasArg("pass")) {
    String User = server.arg("user");
    String Pass = server.arg("pass");

    Session usuario;
    if (verificarUsuario(User, Pass, usuario)) {
      // Generamos un nuevo token para este usuario y lo añadimos
      usuario.Token = generarToken();
      // Si es válido, lo agregamos al vector de usuarios logueados
      UsuariosLogueados.push_back(usuario);
      lastLoginTime = millis(); // Marcar el tiempo del último login
       server.send(200, "text/plain", "Login exitoso. Token: " + usuario.Token);
    } else {
      // Si no es válido, mostramos un mensaje de error
      server.send(401, "text/plain", "Error: Usuario o contraseña incorrectos.");
    }
  } else {
    server.send(400, "text/plain", "Error: Parámetros de usuario y contraseña faltantes.");
  }
}

// Lógica para abrir la puerta
void handleOpenDoor() {
  if (server.hasArg("user") && server.hasArg("token")) {
    String User = server.arg("user");  // Recuperamos el usuario desde los parámetros del POST
    String Token = server.arg("token"); // Recuperamos el token desde los parámetros del POST

    bool userLoggedIn = false;
    for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
      if (UsuariosLogueados[i].User == User && UsuariosLogueados[i].Token == Token) {
        userLoggedIn = true;  // Si encontramos el usuario con el token correcto
        break;
      }
    }

    if (userLoggedIn) {
      // El usuario está logueado, proceder a abrir la puerta
      digitalWrite(Puerta, LOW);  // Abre la puerta
      server.send(200, "text/plain", "Puerta abierta exitosamente.");
      delay(5000);  // Mantiene la puerta abierta durante 5 segundos
      digitalWrite(Puerta, HIGH);  // Cierra la puerta
    } else {
      // El usuario no está logueado, mostramos un mensaje de error
      server.send(401, "text/plain", "Error: Usuario no autenticado.");
    }
  } else {
    // Si no se recibe el parámetro 'user' o 'token', mostramos un mensaje de error
    server.send(400, "text/plain", "Error: Parámetros 'user' o 'token' faltantes.");
  }
}

// Función para agregar un nuevo usuario
void handleAddUser() {
  if (server.hasArg("user") && server.hasArg("pass") && server.hasArg("rol")) {
    String newUser = server.arg("user");
    String newPass = server.arg("pass");
    String newRol = server.arg("rol");

    // Verificar si el usuario ya existe
    for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
      if (UsuariosLogueados[i].User == newUser) {
        server.send(400, "text/plain", "Error: El usuario ya existe.");
        return;
      }
    }

    // Si el usuario no existe, agregamos el nuevo usuario
    Session newSession;
    newSession.User = newUser;
    newSession.PassHash = generarHash(newPass); // Guardamos el hash de la contraseña
    newSession.Rol = newRol;

    UsuariosLogueados.push_back(newSession);
    server.send(200, "text/plain", "Usuario agregado exitosamente.");
  } else {
    server.send(400, "text/plain", "Error: Parámetros faltantes (user, pass, rol).");
  }
}

// Función para eliminar un usuario
void handleDeleteUser() {
  if (server.hasArg("user")) {
    String userToDelete = server.arg("user");

    // Buscar el usuario en el vector
    for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
      if (UsuariosLogueados[i].User == userToDelete) {
        // Eliminar el usuario encontrado
        UsuariosLogueados.erase(UsuariosLogueados.begin() + i);
        server.send(200, "text/plain", "Usuario eliminado exitosamente.");
        return;
      }
    }

    // Si el usuario no fue encontrado
    server.send(404, "text/plain", "Error: Usuario no encontrado.");
  } else {
    server.send(400, "text/plain", "Error: Parámetro 'user' faltante.");
  }
}

// Función para mostrar todos los usuarios logueados
void mostrarUsuariosLogueados() {
  Serial.println("Usuarios logueados:");
  for (size_t i = 0; i < UsuariosLogueados.size(); i++) {
    Serial.print("Usuario: ");
    Serial.print(UsuariosLogueados[i].User);
    Serial.print(", Rol: ");
    Serial.println(UsuariosLogueados[i].Rol);
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  WiFi.mode(WIFI_AP_STA);
  WiFiManager wiFiManager;
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

  server.on("/get_mac", []() {
    String mac = WiFi.macAddress();
    server.send(200, "text/plain", mac);
  });

  // Definir Pin para verificar solicitud HTTP
  pinMode(Puerta, OUTPUT);
  digitalWrite(Puerta, LOW);

  // Endpoint para login
  server.on("/login", HTTP_POST, handleLogin);

  // Endpoint para Abrir la Puerta
  server.on("/door/open", handleOpenDoor);

  // Endpoint para agregar un nuevo usuario
  server.on("/add_user", HTTP_POST, handleAddUser);

  // Endpoint para borrar un usuario
  server.on("/delete_user", HTTP_POST, handleDeleteUser);

  // Mensaje de error
  server.onNotFound([]() {
    server.send(404, "text/plain", "Ruta no encontrada");
  });

  server.begin();
}

 void loop() {
  server.handleClient(); // Mantenemos el servidor funcionando

  // Mostrar los usuarios logueados cada cierto tiempo
  mostrarUsuariosLogueados();

  // Verificar si la sesión ha expirado (15 minutos)
  if (millis() - lastLoginTime > sessionLimit) {
    UsuariosLogueados.clear();  // Limpiar los usuarios logueados después de 15 minutos
    Serial.println("Sesión expirada, usuarios desconectados.");
  }

  delay(10000);  // Muestra los usuarios cada 10 segundos
}



