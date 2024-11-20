
#include "services.h"

//  Como se me ocurre retornar una referencia a una variable local?? Estoy loco???
//
UserSession
SecurityServices::login_user(const String& user_name, const String& pass) 
{
  UserSession result;

  if ((user_name == "brian" && pass == "1234") || 
      (user_name == "lautaro" && pass == "1234") || 
      (user_name == "kike" && pass == "1234")) 
  {
    result = {
      .User = user_name,
      .PassHash = "1234ABCD",
      .Rol = (user_name == "brian") ? "admin" : "user",
      .Token = user_name + pass,
      .Status = "OK",
      .StatusCode = 0
    };
  }
  else 
  {
    result = {
      .Status = "Las credenciales solicitadas son invalidas",
      .StatusCode = 100
    };
  }
  return result;
}


