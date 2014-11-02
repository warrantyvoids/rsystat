#include "SSHStream.h"
#include <cstring>

SSHIBuffer::SSHIBuffer( ssh_channel channel, bool isError, std::size_t size, std::size_t putback ) 
   : m_chan(channel), m_isErr(isError), m_nPutback( putback > 1 ? putback : 1 ), m_buffer( (size > putback ? size : putback) + putback ) {
  char* end = m_buffer.data() + m_buffer.size();
  setg(end, end, end);
}

SSHIBuffer::int_type SSHIBuffer::underflow() {

  //There is still data left in our buffer
  if (gptr() < egptr())
    return traits_type::to_int_type(*gptr());
  
  char * base = m_buffer.data();
  char * start = base;
  
  //first fill?
  if (eback() == base) {
    //nope
    std::memmove(base, egptr() - m_nPutback, m_nPutback);
    start += m_nPutback;
  }
  
  std::size_t n = ssh_channel_read( m_chan, start, m_buffer.size() - (start - base), m_isErr ? 1 : 0 );
  if (n == 0)
    return traits_type::eof();
    
  setg(base, start, start+n);
    
  return traits_type::to_int_type(*gptr());
}
