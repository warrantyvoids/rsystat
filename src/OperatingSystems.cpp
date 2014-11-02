#include "OperatingSystems.h"

#include <sstream>
#include <limits>

class UnixOperatingSystem : public OperatingSystem {
  public:
    UnixOperatingSystem(Connection& conn, std::string name) 
       : OperatingSystem(name), m_conn(conn) { }
    
    std::size_t getNUsers() override {
      ExecChannel exec = m_conn.exec("who -q");
      std::istream& in = exec.getStdOut();

      in.ignore( std::numeric_limits<std::streamsize>::max(), '=');
      
      std::size_t nusers = 0;
      in >> nusers;
      
      return nusers;            
      
    }
    
    std::vector<std::string> getUsers() override {
     
    }
    
  protected:
    Connection& m_conn;
};

class LinuxOperatingSystem : public UnixOperatingSystem {
  public:
    LinuxOperatingSystem(Connection& conn, std::string name) :
       UnixOperatingSystem(conn, name) {
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
    
};

class FreeBSDOperatingSystem : public UnixOperatingSystem {
  public:
    FreeBSDOperatingSystem(Connection& conn, std::string name) :
       UnixOperatingSystem(conn,name) {
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
