#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include "socket_tools.h"
#include "settings.h"

int main(int argc, const char **argv)
{
  int sockfd = -1;
  
  sockaddr_in serverSockaddrIn;
  sockaddr_in clientSockaddrIn;
  socklen_t sockaddrInLen = sizeof(sockaddr_in);

  {
    const char *serverAdress = SERVER_ADDR;
    const char *serverPort = SERVER_PORT;

    if (get_sockaddr_by_addr(serverAdress, serverPort, &serverSockaddrIn))
    {
      printf("Cannot get server sockaddr!\n");
      return 1;
    }

    char selfPort[8];

    for (int i = 0; i < CLIENT_PORT_RANGE && sockfd == -1; ++i)
    {
      sprintf(selfPort, "%d", CLIENT_PORT_FROM + i);

      sockfd = create_recv_socket(selfPort, &clientSockaddrIn);
    }
    if (sockfd == -1)
    {
      printf("Cannot create a client recv socket!\n");
      return 1;
    }

    printf("Server port: %d; Your port: %d\n", 
      ntohs(serverSockaddrIn.sin_port), ntohs(clientSockaddrIn.sin_port));
  }

  

  char *buffer = new char[BUF_SIZE];
  ssize_t sendRes = -1; 
  ssize_t recvRes = -1; 

  while (true)
  {
    printf(CLIENT_INVITE);
    fgets(buffer, BUF_SIZE, stdin);
    
    sendRes = sendto(
      sockfd, buffer, BUF_SIZE,
      0, (sockaddr *)&serverSockaddrIn, sockaddrInLen);
    if (sendRes == -1)
    { 
      printf("%s", strerror(errno)); 
    }

    recvRes = recvfrom(
      sockfd, buffer, BUF_SIZE, 
      0, (sockaddr *)&serverSockaddrIn, &sockaddrInLen);
    if (recvRes > 0)
    {
      printf("repty from (%s:%d): %s", 
        inet_ntoa(serverSockaddrIn.sin_addr), ntohs(serverSockaddrIn.sin_port), buffer);
    }
  }

  delete[] buffer;

  return 0;
}
