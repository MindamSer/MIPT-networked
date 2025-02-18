#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "socket_tools.h"

int main(int argc, const char **argv)
{
  const char *openPort = "2025";

  int sfd = create_dgram_socket(nullptr, openPort, nullptr);

  if (sfd == -1)
  {
    printf("Cannot create a server socket!\n");
    return 1;
  }

  printf("Server is on. Listening...\n");


  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(sfd, &readSet);

  timeval timeout = { 0, 100000 }; // 100 ms

  char *buffer = new char[BUF_SIZE];
  ssize_t recvRes = -1; 
  ssize_t sendRes = -1; 
  addrinfo clientAddrInfo;

  sockaddr_in sin;


  
  while (true)
  {
    fd_set tmpReadSet = readSet;

    select(sfd + 1, &tmpReadSet, NULL, NULL, &timeout);


    if (FD_ISSET(sfd, &readSet))
    { 
      recvRes = recvfrom(
        sfd, buffer, BUF_SIZE, 
        MSG_WAITALL, clientAddrInfo.ai_addr, &clientAddrInfo.ai_addrlen);
      if (recvRes > 0)
      {
        sin = *((sockaddr_in*)clientAddrInfo.ai_addr);
        
        printf("(%s:%d) %s\n", inet_ntoa(sin.sin_addr), sin.sin_port, buffer); // assume that buffer is a string

        sendRes = sendto(
          sfd, buffer, BUF_SIZE,
          MSG_WAITALL, clientAddrInfo.ai_addr, clientAddrInfo.ai_addrlen);
        if (sendRes == -1)
        { 
          printf("%s", strerror(errno)); 
        }
      }
    }
  }
  return 0;
}
