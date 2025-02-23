#include <cstdint>
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
#include <sstream>
#include <vector>
#include <iterator>
#include <algorithm>

#include "socket_tools.h"
#include "settings.h"


template <typename Out>
void split(const std::string &s, char delim, Out result) 
{
    std::istringstream iss(s);
    std::string item;
    while (std::getline(iss, item, delim)) {
        *result++ = item;
    }
}

std::vector<std::string> split(const std::string &s, char delim) 
{
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

bool operator==(const sockaddr_in &left, const sockaddr_in &right)
{
  return (left.sin_port == right.sin_port) && (left.sin_addr.s_addr == right.sin_addr.s_addr);
}


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
  std::vector<std::string> messageElems;
  ssize_t recvRes = -1;
  ssize_t sendRes = -1;

  sockaddr_in senderSockaddrIn;
  socklen_t senderSockaddrInLen = sizeof(sockaddr_in);

  std::vector<sockaddr_in> registeredAddresses;




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
        if(buffer[0] != '/')
        {
          sprintf(buffer, "Expected: /{command}");
          sendRes = sendto(
            sockfd, buffer, BUF_SIZE,
            0, (sockaddr *)&senderSockaddrIn, senderSockaddrInLen);
          if (sendRes == -1)
          { 
            std::cout << strerror(errno) << std::endl;
          }

          continue;
        }

        std::cout 
        << "(" << inet_ntoa(senderSockaddrIn.sin_addr) 
        << ":" << ntohs(senderSockaddrIn.sin_port) 
        << "): " << buffer << std::endl;

        messageElems = split(buffer, ' ');

        if (std::find(
          registeredAddresses.begin(), registeredAddresses.end(), 
          senderSockaddrIn) == registeredAddresses.end())
        {
          if (messageElems[0] == COMMAND_REG)
          {
            registeredAddresses.push_back(senderSockaddrIn);

            std::cout << "user is now registered" << std::endl;

            sprintf(buffer, "You are now registered!");
            sendRes = sendto(
              sockfd, buffer, BUF_SIZE,
              0, (sockaddr *)&senderSockaddrIn, senderSockaddrInLen);
            if (sendRes == -1)
            {
              std::cout << strerror(errno) << std::endl;
            }

            continue;
          }

          std::cout << "unregistered user" << std::endl;

          sprintf(buffer, "You are not registered! Type /reg");
          sendRes = sendto(
            sockfd, buffer, BUF_SIZE,
            0, (sockaddr *)&senderSockaddrIn, senderSockaddrInLen);
          if (sendRes == -1)
          {
            std::cout << strerror(errno) << std::endl;
          }

          continue;
        }
        
        if (messageElems[0] == COMMAND_ESC)
        {
          registeredAddresses.erase(std::find(
            registeredAddresses.begin(), registeredAddresses.end(), 
            senderSockaddrIn));

          std::cout << "user is now unregistered" << std::endl;

          sprintf(buffer, "You are not longer registered!");
          sendRes = sendto(
            sockfd, buffer, BUF_SIZE,
            0, (sockaddr *)&senderSockaddrIn, senderSockaddrInLen);
          if (sendRes == -1)
          {
            std::cout << strerror(errno) << std::endl;
          }
        }
        else
        {
          sprintf(buffer, "Unknown command!");
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
  }

  return 0;
}
