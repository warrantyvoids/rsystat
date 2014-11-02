#ifndef PROCESS_H
#define PROCESS_H

class Connection;
struct Process {
  Connection* connection;
  std::string command;
  std::string user;
  double time; //time in hours
  double cpu;
  double mem;
  std::size_t pid;
};


#endif