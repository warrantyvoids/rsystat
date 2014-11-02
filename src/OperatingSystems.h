#ifndef OPERATINGSYSTEMS_H
#define OPERATINGSYSTEMS_H

#include <vector>
#include <string>
#include <array>
#include <cstdlib>

#include "Connection.h"

class OperatingSystem {
  public:
    OperatingSystem(std::string name) : m_name(name) { }
    virtual ~OperatingSystem() {};
    
    OperatingSystem(const OperatingSystem& other) = delete;
    OperatingSystem(OperatingSystem&& other) = delete;
    OperatingSystem& operator=(const OperatingSystem& other) = delete;
    OperatingSystem& operator=(OperatingSystem&& other) = delete;
    
    virtual std::array<double,3> getLoad() = 0;
    virtual std::size_t getNUsers() = 0;
    virtual std::vector<std::string> getUsers() = 0;
    std::string getName() { return m_name; }
  private:
    std::string m_name;
};

class Connection;
OperatingSystem* createOS(Connection& conn);

#endif