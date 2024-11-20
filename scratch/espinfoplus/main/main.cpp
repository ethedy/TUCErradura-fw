
#include "string"
#include <newlib.h>
#include <stdio.h>

#include "sdkconfig.h"

extern "C" {

  void app_main(void)
  {
    //  std::cout << "Hola, Mundo!!" << std::endl;
    
    int x = 0b10101010;       //  version C++14
    float pepe = 0x1B4p9;     //  version C++17

    std::string mensaje = u8"Hola, Mundo!! Sera C++17?????";

    printf("%s\n", mensaje.c_str());
    printf("float: %f ; int: %i\n", pepe, x);
    printf("%s\n", CONFIG_IDF_TARGET);
  }

}