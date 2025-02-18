#include <sys/socket.h>

#include <netdb.h>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

#include "socket_tools.h"

// Adaptation of linux man page: https://linux.die.net/man/3/getaddrinfo
static int get_dgram_socket(addrinfo *ai_list, bool should_bind, addrinfo *res_addr)
{
  int sfd = -1;

  for (addrinfo *cur_ai = ai_list; cur_ai != nullptr; cur_ai = cur_ai->ai_next)
  {
    sfd = socket(cur_ai->ai_family, cur_ai->ai_socktype, cur_ai->ai_protocol);
    if (sfd == -1) 
    { continue; }

    int true_val = 1;
    fcntl(sfd, F_SETFL, O_NONBLOCK);
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &true_val, sizeof(int));

    if (res_addr)
    { *res_addr = *cur_ai; }
    if (!should_bind) 
    { return sfd; }
    if (bind(sfd, cur_ai->ai_addr, cur_ai->ai_addrlen) == 0) 
    { return sfd; }

    close(sfd);
  }

  return -1;
}

int create_dgram_socket(const char *address, const char *port, addrinfo *res_addr)
{
  addrinfo hints;
  memset(&hints, 0, sizeof(addrinfo));

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = IPPROTO_UDP;
  hints.ai_flags = AI_PASSIVE;


  int sfd = -1;
  addrinfo *ai_list = nullptr;

  if (getaddrinfo(address, port, &hints, &ai_list) == 0)
  {
    sfd = get_dgram_socket(ai_list, !address, res_addr);

    freeaddrinfo(ai_list);
  }

  return sfd;
}

