#pragma once

struct sockaddr_in;

int create_recv_socket(const char *port, sockaddr_in *res_sockaddr);

int get_sockaddr_by_addr(const char *address, const char *port, sockaddr_in *res_sockaddr);
