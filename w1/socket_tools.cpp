#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdio.h>

#include "socket_tools.h"


int get_sockaddr_by_addr(const char *address, const char *port, sockaddr_in *res_sockaddr)
{
  addrinfo hints;
  memset(&hints, 0, sizeof(addrinfo));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  hints.ai_flags = AI_PASSIVE;


  addrinfo *ai_list = nullptr;

  if (getaddrinfo(nullptr, port, &hints, &ai_list) == 0)
  {
    *res_sockaddr = *((sockaddr_in *)ai_list->ai_addr);

    freeaddrinfo(ai_list);

    return 0;
  }

  return 1;
}


int create_recv_socket(const char *port, sockaddr_in *res_sockaddr)
{
  addrinfo hints;
  memset(&hints, 0, sizeof(addrinfo));

  bool isListener = !address;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  if (isListener)
    hints.ai_flags = AI_PASSIVE;

  addrinfo *result = nullptr;
  if (getaddrinfo(address, port, &hints, &result) != 0)
    return -1;

  int sfd = -1;
  addrinfo *ai_list = nullptr;

  if (getaddrinfo(nullptr, port, &hints, &ai_list) == 0)
  {
    for (addrinfo *cur_ai = ai_list; cur_ai != nullptr; cur_ai = cur_ai->ai_next)
    {
      sfd = socket(cur_ai->ai_family, cur_ai->ai_socktype, cur_ai->ai_protocol);
      if (sfd == -1) 
      { continue; }

      if (res_sockaddr)
      { *res_sockaddr = *((sockaddr_in *)cur_ai->ai_addr); }
  
      int true_val = 1;
      fcntl(sfd, F_SETFL, O_NONBLOCK);
      setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &true_val, sizeof(int));
      
      if (bind(sfd, cur_ai->ai_addr, cur_ai->ai_addrlen) == 0) 
      { break; }
  
      close(sfd);
    }

    freeaddrinfo(ai_list);
  }

  //freeaddrinfo(result);
  return sfd;
}

