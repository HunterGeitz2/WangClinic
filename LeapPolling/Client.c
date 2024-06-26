//
//#include <stdio.h>
//#include <winsock2.h>
//
//#pragma comment(lib, "ws2_32.lib") // Winsock Library
//
//SOCKET s;
//struct sockaddr_in server;
//int connected = 0;
//
//void handle(char* message) {
//    if (!connected) {
//        WSADATA wsa;
//
//        printf("\nInitialising Winsock...");
//        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
//            printf("Failed. Error Code : %d", WSAGetLastError());
//            return;
//        }
//
//        printf("Initialised.\n");
//
//        // Create a socket
//        if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
//            printf("Could not create socket : %d", WSAGetLastError());
//        }
//
//        printf("Socket created.\n");
//
//        inet_pton(AF_INET, "127.0.0.1", &(server.sin_addr));
//        server.sin_family = AF_INET;
//        server.sin_port = htons(8888);
//
//        // Connect to remote server
//        if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0) {
//            puts("connect error");
//            return;
//        }
//
//        puts("Connected");
//        connected = 1;
//    }
//
//    // Calculate the length of the message
//    int message_length = strlen(message);
//
//    // Convert the length to a string
//    char message_length_str[10];
//    sprintf_s(message_length_str, sizeof(message_length_str), "%d", message_length);
//
//    // Send the length of the message
//    if (send(s, message_length_str, strlen(message_length_str), 0) < 0) {
//        puts("Send failed");
//        return;
//    }
//
//    // Wait for the server to acknowledge the message length
//    char ack[10];
//    recv(s, ack, sizeof(ack), 0);
//
//    // Send the message
//    if (send(s, message, message_length, 0) < 0) {
//        puts("Send failed");
//        return;
//    }
//
//    puts("Data Send\n");
//    printf(message);
//}

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

SOCKET s;
struct sockaddr_in server;
int connected = 0;

void handle(char* message) {
    if (!connected) {
        WSADATA wsa;

        //printf("\nInitialising Winsock...");
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            //printf("Failed. Error Code : %d", WSAGetLastError());
            return;
        }

        //printf("Initialised.\n");

        // Create a socket
        if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
            //printf("Could not create socket : %d", WSAGetLastError());
        }

        //printf("Socket created.\n");

        //inet_pton(AF_INET, "10.38.80.69", &(server.sin_addr));
        inet_pton(AF_INET, "127.0.0.1", &(server.sin_addr)); ////Local
        server.sin_family = AF_INET;
        server.sin_port = htons(8888);

        // Connect to remote server
        if (connect(s, (struct sockaddr*)&server, sizeof(server)) < 0) {
            puts("connect error");
            return;
        }

        // Disable Nagle's Algorithm
        int flag = 1;
        setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));

        puts("Connected");
        connected = 1;
    }

    // Calculate the length of the message
    int message_length = strlen(message);

    // Convert the length to a string
    char message_length_str[10];
    sprintf_s(message_length_str, sizeof(message_length_str), "%d", message_length);

    // Send the length of the message
    if (send(s, message_length_str, strlen(message_length_str), 0) < 0) {
        puts("Send failed");
        return;
    }

    // Wait for the server to acknowledge the message length
    char ack[10];
    recv(s, ack, sizeof(ack), 0);

    // Send the message
    if (send(s, message, message_length, 0) < 0) {
        puts("Send failed");
        return;
    }

    //puts("Data Send\n");
    //printf(message);
}
