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

  {
    const char *openPort = SERVER_PORT;
    sockaddr_in serverSockaddrIn;

    sockfd = create_recv_socket(openPort, &serverSockaddrIn);
    if (sockfd == -1)
    {
      printf("Cannot create a server socket!\n");
      return 1;
    }

    printf("Server at port %d. Listening...\n", ntohs(serverSockaddrIn.sin_port));
  }



  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(sockfd, &readSet);
  timeval timeout = { 0, 100000 }; // 100 ms

  char *buffer = new char[BUF_SIZE];
  ssize_t recvRes = -1;
  ssize_t sendRes = -1;

  sockaddr_in senderSockaddrIn;
  socklen_t senderSockaddrInLen = sizeof(sockaddr_in);

  while (true)
  {
    fd_set tmpReadSet = readSet;

    select(sockfd + 1, &tmpReadSet, NULL, NULL, &timeout);


    if (FD_ISSET(sockfd, &readSet))
    { 
      recvRes = recvfrom(
        sockfd, buffer, BUF_SIZE, 
        0, (sockaddr *)&senderSockaddrIn, &senderSockaddrInLen);
      if (recvRes > 0)
      {
        printf("(%s:%d): %s", 
          inet_ntoa(senderSockaddrIn.sin_addr), ntohs(senderSockaddrIn.sin_port), buffer);

        sendRes = sendto(
          sockfd, buffer, BUF_SIZE,
          0, (sockaddr *)&senderSockaddrIn, senderSockaddrInLen);
        if (sendRes == -1)
        { 
          printf("%s", strerror(errno)); 
        }
      }
    }
  }

  return 0;
}
