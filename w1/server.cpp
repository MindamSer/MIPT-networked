#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

#include "socket_tools.h"
#include "settings.h"


int main(int argc, const char **argv)
{
  int sockfd = -1;

  {
    std::string openPort = SERVER_PORT;
    sockaddr_in serverSockaddrIn;

    sockfd = create_recv_socket(openPort.c_str(), &serverSockaddrIn);
    if (sockfd == -1)
    {
      std::cout << "Cannot create a server socket!" << std::endl;
      return 1;
    }

    std::cout << "Server at port " 
    << ntohs(serverSockaddrIn.sin_port) 
    << ". Listening..." << std::endl;
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

    select(sockfd + 1, &tmpReadSet, nullptr, nullptr, &timeout);


    if (FD_ISSET(sockfd, &readSet))
    { 
      recvRes = recvfrom(
        sockfd, buffer, BUF_SIZE, 
        0, (sockaddr *)&senderSockaddrIn, &senderSockaddrInLen);
      if (recvRes > 0)
      {
        std::cout 
        << "(" << inet_ntoa(senderSockaddrIn.sin_addr) 
        << ":" << ntohs(senderSockaddrIn.sin_port) 
        << "): " << buffer << std::endl;


        sendRes = sendto(
          sockfd, buffer, BUF_SIZE,
          0, (sockaddr *)&senderSockaddrIn, senderSockaddrInLen);
        if (sendRes == -1)
        { 
          std::cout << strerror(errno) << std::endl;
        }
      }
    }
  }

  return 0;
}
