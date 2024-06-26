#ifndef CLIENT_H
#define CLIENT_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#endif

#define HEADER 64
#define PORT 8891
#define FORMAT "utf-8"
#define DISCONNECT_MESSAGE "DISCONNECT"
//#define SERVER_IP "172.24.240.1"
#define SERVER_IP "150.250.121.136" // change to desired
#define MAX_BUFFER 2048

extern SOCKET client_socket;

void send_message(char* send_length, char* message);
void handle(char* msg);
int clientmain();

#endif // CLIENT_H

