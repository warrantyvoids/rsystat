#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

class Configuration {
  public:
    Configuration();
    Configuration(std::string filename);
    
    const std::vector<std::string>& getHosts() const { return m_hosts; }
  private:
  
    void readConfig(std::ifstream& file);
  
    std::vector<std::string> m_hosts;
};

#endif