#ifndef SSHSTREAM_H
#define SSHSTREAM_H

#include <streambuf>
#include <cstdlib>
#include <vector>
#include <libssh/libssh.h>

class SSHIBuffer : public std::streambuf {
  public:
    explicit SSHIBuffer( ssh_channel channel, bool isError = false, std::size_t size = 256, std::size_t putback = 8 );
    
    SSHIBuffer( const SSHIBuffer & other ) = delete;
    SSHIBuffer( SSHIBuffer && other );
    
    SSHIBuffer& operator= (const SSHIBuffer & other) = delete;
    SSHIBuffer& operator= (SSHIBuffer&& other);
    
    int_type underflow() override;
    
  private:
    ssh_channel m_chan;
    bool m_isErr;
    const std::size_t m_nPutback;
    std::vector<char> m_buffer;

};

class SSHOBuffer : public std::streambuf {
};

#endif
