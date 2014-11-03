#ifndef FILESYSTEMS_H
#define FILESYSTEMS_H

struct Filesystem {
  Connection* connection;
  std::string source;
  std::string target;
  std::string type;
  std::size_t size;
  std::size_t used;
};


#endif
