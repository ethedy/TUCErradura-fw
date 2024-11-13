#include <vector>

#ifndef WIFI_SELECTOR_H
#define WIFI_SELECTOR_H

#include <cstdlib>
#include <string>
#include <tuple>

class WifiSelector 
{
private:
  std::vector<std::tuple<std::string, std::string>> redes_aprendidas;

public:
  WifiSelector();


};


#endif