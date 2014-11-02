#include "Configuration.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <iostream>

void Configuration::readConfig(std::ifstream& file) {
  while (!file.eof()) {
    std::string line;
    
    std::getline(file, line);
    if (file.eof())
      break;
      
    if (line.size() == 0 || line[0] == '#')
      continue;
          
    std::istringstream linestream{line};
    
    std::string hostname;
    linestream >> hostname;
    
    m_hosts.push_back(hostname);
    
  }
  
}

Configuration::Configuration() {
  std::string homedir = std::string( std::getenv( "HOME" ) );
  homedir += "/.rsystat.conf";
  std::ifstream localConfig { homedir };
  if (localConfig) {
    readConfig(localConfig);
    return;
  }
  std::ifstream globalConfig { "/usr/local/etc/rsystat.conf" };
  if (globalConfig) {
    readConfig(globalConfig);
    return;
  }
  std::cerr << "No configuration file found!" << std::endl;
  std::exit(1);
}

Configuration::Configuration(std::string fname) {
  std::ifstream config { fname };
  if (!config) {
    std::cerr << "Configuration file '" << fname << "' not found!" << std::endl;
    std::exit(1);
  }
  readConfig(config);
}

