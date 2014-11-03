#include "Connection.h"
#include <iostream>

Connection::Connection(const std::string& hostname) : m_hostname(hostname), m_os(nullptr) {
  m_session = ssh_new();
  
  if (m_session == nullptr)
    throw ("Initialization failed");
    
  ssh_options_set(m_session, SSH_OPTIONS_HOST, hostname.c_str());
  
  int status = ssh_connect( m_session );
  if (status != SSH_OK) {
    ssh_free(m_session);
    m_session = nullptr;
    return;
  }
  //  error();
  
  int state = ssh_is_server_known(m_session);
  
  std::string name;
  unsigned char* hash = nullptr;
  int hlen = ssh_get_pubkey_hash(m_session, &hash);
  
  if (hlen < 0)
    error();
    
  switch (state) {
    case SSH_SERVER_KNOWN_OK:
      break;
    case SSH_SERVER_KNOWN_CHANGED:
    case SSH_SERVER_FOUND_OTHER:
      std::cerr << "Found wrong identification for the server" << std::endl;
      std::cerr << "SERVER CONNECTION WILL BE CLOSED." << std::endl;
      std::cerr << "CHECK RIGHT NOW THAT NOBODY IS DOING NASTY STUFF." << std::endl;
      ssh_free(m_session);
      m_session = nullptr;
      throw "POSSIBLE ATTACK";
    default:
      //todo
      std::cerr << "Server is unkown or shit" << std::endl;
      error();
  }
  
  state = ssh_userauth_publickey_auto(m_session, NULL, NULL);
  switch (state) {
    case SSH_AUTH_ERROR:
    case SSH_AUTH_DENIED:
      error();
      break;
    case SSH_AUTH_PARTIAL:
      std::cerr << "Not done yet!" << std::endl;
      break;
    case SSH_AUTH_SUCCESS:
      break;
  }
  
  m_os = createOS(*this);
}

Connection::Connection(Connection&& other) {
  m_session = other.m_session;
  m_os = other.m_os;
  other.m_session = nullptr;
  other.m_os = nullptr;
}

Connection& Connection::operator= (Connection&& other) {
  if (m_session != nullptr) {
    ssh_disconnect( m_session );
    ssh_free( m_session );
  }
  delete m_os;
  m_os = other.m_os;
  other.m_os = nullptr;
  m_session = other.m_session;
  other.m_session = nullptr;
  
  return *this;
}

void Connection::error() {
  std::cerr << "An error has occurred:\n\t" << ssh_get_error(m_session) << std::endl;
  ssh_disconnect( m_session );
  ssh_free( m_session );
  m_session = nullptr;
  throw "Error!";
}

Connection::~Connection() {
  if (m_os != nullptr)
    delete m_os;
  if (m_session != nullptr) {
    ssh_disconnect( m_session );
    ssh_free( m_session );
  }
}

OperatingSystem* Connection::getOS() {
  return m_os;
}

ExecChannel Connection::exec(const std::string& command) {
  ExecChannel chan { *this, command };
  
  return chan;
}

Connection::operator bool const() {
  return m_session != nullptr;
}

ExecChannel::ExecChannel(Connection& conn, const std::string& command) {
  m_chan = ssh_channel_new( conn.m_session );
  int status = ssh_channel_open_session( m_chan );
  
  if (status != SSH_OK) {
    std::cerr << "Could not open channel!" << std::endl;
    ssh_channel_free( m_chan );
    m_chan = nullptr;
    return;
  }
  
  status = ssh_channel_request_exec(m_chan, command.c_str());
  m_stdout_buff = new SSHIBuffer( m_chan, false );
  m_stderr_buff = new SSHIBuffer( m_chan, true );
  m_stdout = new std::istream( m_stdout_buff );
  m_stderr = new std::istream( m_stderr_buff );
}

ExecChannel::ExecChannel(ExecChannel && other) :
  m_stdout(other.m_stdout), m_stderr(other.m_stderr), m_stdout_buff(other.m_stdout_buff), m_stderr_buff(other.m_stderr_buff)
{
  m_chan = other.m_chan;
  other.m_chan = nullptr;
  other.m_stdout = nullptr;
  other.m_stderr = nullptr;
  other.m_stdout_buff = nullptr;
  other.m_stderr_buff = nullptr;
}

ExecChannel::~ExecChannel() {
  if (m_chan == nullptr)
    return;
    
  delete m_stdout;
  delete m_stderr;
  delete m_stdout_buff;
  delete m_stderr_buff;
  ssh_channel_close( m_chan );
  ssh_channel_free( m_chan );
}

std::istream& ExecChannel::getStdOut() {
  return *m_stdout;
}

std::istream& ExecChannel::getStdErr() {
  return *m_stderr;
}