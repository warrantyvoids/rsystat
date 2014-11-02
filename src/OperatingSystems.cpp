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
    
    std::vector<Process> getProcesses() override {
      std::vector<Process> returns;
      ExecChannel exec = m_conn.exec("ps acx -o command,user,pid,pcpu,pmem,etimes");
      std::istream& in = exec.getStdOut();
      
      std::string line;
      std::getline(in, line);
      
      Process p;
      p.connection = &m_conn;
      while (std::getline(in, line)) {
        std::istringstream lin(line);
        
        lin >> p.command >> p.user >> p.pid >> p.cpu >> p.mem >> p.time;
        returns.push_back(p);
      }
      return returns;
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
