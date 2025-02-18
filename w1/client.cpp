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
  const char *serverAdress = "localhost";
  const char *serverPort = "2025";
  addrinfo serverAddrInfo;

  int sfd = create_dgram_socket(serverAdress, serverPort, &serverAddrInfo);

  if (sfd == -1)
  {
    printf("Cannot create a client socket!");
    return 1;
  }


  char *buffer = new char[BUF_SIZE];
  ssize_t sendRes = -1; 
  ssize_t recvRes = -1; 

  while (true)
  {
    printf(">");
    scanf("%s", buffer);
    
    sendRes = sendto(
      sfd, buffer, BUF_SIZE,
      MSG_WAITALL, serverAddrInfo.ai_addr, serverAddrInfo.ai_addrlen);
    if (sendRes == -1)
    { 
      printf("%s", strerror(errno)); 
    }

    while(recvRes < 0)
    {
      recvRes = recvfrom(
        sfd, buffer, BUF_SIZE,
        MSG_WAITALL, serverAddrInfo.ai_addr, &serverAddrInfo.ai_addrlen);
      if (recvRes > 0)
      {
        printf("serv_resp: %s\n", buffer);
      }
    }

    sendRes = -1;
    recvRes = -1;
  }


  delete[] buffer;

  return 0;
}
