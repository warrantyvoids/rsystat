#include "OperatingSystems.h"

#include <sstream>

class LinuxOperatingSystem : public OperatingSystem {
  public:
    LinuxOperatingSystem(Connection& conn, std::string name) :
       OperatingSystem(name), m_conn(conn) {
    }
    
    std::array<double,3> getLoad() override {
      ExecChannel exec = m_conn.exec("cat /proc/loadavg");
      std::istream& in = exec.getStdOut();
      std::array<double,3> loads;
      for(double & load : loads) {
        in >> load;
      }
      return loads;
    }
    
    std::size_t getNUsers() override {
      return 0;
    }
    
    std::vector<std::string> getUsers() override {
      return std::vector<std::string>();
    }
    
  private:
    Connection& m_conn;
};

class FreeBSDOperatingSystem : public OperatingSystem {
  public:
    FreeBSDOperatingSystem(Connection& conn, std::string name) :
       OperatingSystem(name), m_conn(conn) {
    }
    
    std::array<double,3> getLoad() override {
      ExecChannel exec = m_conn.exec("sysctl -n vm.loadavg");
      std::istream& in = exec.getStdOut();
      in.ignore();
      
      std::array<double,3> loads;
      for (double & load : loads) {
        in >> load;
      }
      return loads;
    }
    
    std::size_t getNUsers() override {
      return 0;
    }
    
    std::vector<std::string> getUsers() override {
      return std::vector<std::string>();
    }
  private:
    Connection& m_conn;
};

OperatingSystem* createOS(Connection& conn) {
  ExecChannel exec = conn.exec("uname");
  std::string os;
  std::istream& in = exec.getStdOut();
  in >> os;
  
  if (os == "Linux") {
    return new LinuxOperatingSystem(conn, os);
  } else if (os == "FreeBSD") {
    return new FreeBSDOperatingSystem(conn, os);
  } else {
    throw "Poep";
  }
}
