#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>

#include <iostream>
#include <thread>
#include <string>

#include "socket_tools.h"
#include "settings.h"

int main(int argc, const char **argv)
{
  int sockfd = -1;
  
  sockaddr_in serverSockaddrIn;
  sockaddr_in clientSockaddrIn;
  socklen_t sockaddrInLen = sizeof(sockaddr_in);

  {
    std::string serverAdress = SERVER_ADDR;
    std::string serverPort = SERVER_PORT;

    if (get_sockaddr_by_addr(serverAdress.c_str(), serverPort.c_str(), &serverSockaddrIn))
    {
      std::cout << "Cannot get server sockaddr!" << std::endl;
      return 1;
    }

    int clientPortFrom = CLIENT_PORT_FROM;
    int clientPortRange = CLIENT_PORT_RANGE;
    std::string selfPort;

    for (int i = 0; i < clientPortRange && sockfd == -1; ++i)
    {
      selfPort = std::to_string(clientPortFrom + i);

      sockfd = create_recv_socket(selfPort.c_str(), &clientSockaddrIn);
    }
    if (sockfd == -1)
    {
      std::cout << "Cannot create a client recv socket!" << std::endl;
      return 1;
    }

    std::cout 
    << "Server port: " << ntohs(serverSockaddrIn.sin_port) 
    << "; Your port: " << ntohs(clientSockaddrIn.sin_port) 
    << std::endl;
  }

  
  fd_set readSet;
  FD_ZERO(&readSet);
  FD_SET(sockfd, &readSet);
  timeval timeout = { 0, 100000 };

  char *buffer = new char[BUF_SIZE];
  std::string input;
  ssize_t sendRes = -1; 
  ssize_t recvRes = -1; 


  std::thread thread_recv([&]()
  {
    while(true)
    {
      fd_set tmpReadSet = readSet;

      select(sockfd + 1, &tmpReadSet, nullptr, nullptr, &timeout);
  
      if (FD_ISSET(sockfd, &readSet))
      {
        recvRes = recvfrom(
          sockfd, buffer, BUF_SIZE, 
          0, (sockaddr *)&serverSockaddrIn, &sockaddrInLen);
        if (recvRes > 0)
        {
          std::cout << "reply from server: " << buffer << std::endl;
        }
      }
    }
  });

  std::thread thread_send([&]()
  {
    while(true)
    {
      std::cin >> input;
      sprintf(buffer, "%s", input.c_str());
      
      sendRes = sendto(
        sockfd, buffer, BUF_SIZE,
        0, (sockaddr *)&serverSockaddrIn, sockaddrInLen);
      if (sendRes == -1)
      { 
        std::cout << strerror(errno) << std::endl;
      }
    }
  });

  thread_recv.join();
  thread_send.join();

  delete[] buffer;

  return 0;
}
