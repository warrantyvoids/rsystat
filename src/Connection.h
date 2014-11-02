#ifndef CONNECTION_H
#define CONNECTION_H

#include "SSHStream.h"
#include "OperatingSystems.h"

#include <cstdlib>
#include <string>
#include <iostream>

#include <libssh/libssh.h>

class Connection;

class ExecChannel {
public:

  ExecChannel(const ExecChannel& other) = delete;
  ExecChannel& operator=(const ExecChannel& other) = delete;
  
  ExecChannel(ExecChannel && other);
  ExecChannel& operator=(ExecChannel && other) = default;
  
  std::istream& getStdOut();
  std::istream& getStdErr();
  std::ostream& getStdIn();

  ~ExecChannel();
  
private:
  friend class Connection;

  ExecChannel(Connection& conn, const std::string& command);  
  ssh_channel m_chan;
  std::istream* m_stdout;
  std::istream* m_stderr;
  SSHIBuffer * m_stdout_buff;
  SSHIBuffer * m_stderr_buff;
};

class ShellChannel {
};

class SftpChannel {
};

class OperatingSystem;
class Connection {
  public:
    Connection(const std::string& hostname);
    ~Connection();
    
    Connection(const Connection& other) = delete; 
    Connection& operator=(const Connection& other) = delete;
    
    Connection(Connection&& other);
    Connection& operator=(Connection&& other);
    
    operator bool const ();
    
    ExecChannel exec(const std::string& command);
    
    OperatingSystem* getOS();
    std::string getHostname() { return m_hostname; }
    
  private:
  
    void error();
    std::string m_hostname;
    ssh_session m_session;
    OperatingSystem* m_os;
    
    friend class ExecChannel;
};

#endif