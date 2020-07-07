/* Model for the client */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define DEFAULT_PORT "27015"
#define DEFAULT_LEN 512


/*
Alternate declaration to figure out...
int __cdecl main(int argc, char **argv)
*/

using namespace std;

int main(int argc, char** argv) {
	/* Define relevant variables */
	WSADATA wsaData; // Structure contains info about windows socket implmenetation
	int works;
	int receiveLength = DEFAULT_LEN;
	char receiver[DEFAULT_LEN];
	struct addrinfo* result = NULL, * ptr = NULL, hints; // Contains sockaddr structure
	int sender = 1;
	string message;

	/* Validate parameters */
	if (argc != 2) {
		printf("Error on parameters\n");
		return 1;
	}

	/* Initialize Winsock */
	works = WSAStartup(MAKEWORD(2, 2), &wsaData); // Initiate Winsock DLL
	if (works != 0) {
		printf("WSAStartup failed\n");
		return 1;
	}

	/* Creating a socket */
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Internet address family: unspecified
	hints.ai_socktype = SOCK_STREAM; // Socket type: Stream
	hints.ai_protocol = IPPROTO_TCP; // Protocol: TCP/IP

	/* Check server address/port available */
	works = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
	if (works != 0) {
		printf("Error on getaddrinfo\n");
		WSACleanup();
		return 1;
	}

	/* Keep looking for a connection from the linked list format */
	SOCKET connecter = INVALID_SOCKET;
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		/* Create the connecting socket */
		connecter = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol); // Create socket
		if (connecter == INVALID_SOCKET) {
			printf("Error during socket creation\n");
			WSACleanup();
			return 1;
		}

		/* Connecting to the server */
		works = connect(connecter, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (works == SOCKET_ERROR) {
			closesocket(connecter);
			connecter = INVALID_SOCKET;
			continue;
		}
		break;
	}
	freeaddrinfo(result);
	if (connecter == INVALID_SOCKET) {
		printf("Error during server connection\n");
		WSACleanup();
		return 1;
	}


	/* SEND AND RECEIVE */
	printf("Press '#' to end the connection\n");
	do {
		if (sender == 1) {
			printf("Please enter a message for the client to send: ");
			getline(cin, message);
			works = send(connecter, message.c_str(), message.length(), 0);
			if (works == SOCKET_ERROR) {
				printf("Error during message send\n");
				closesocket(connecter);
				WSACleanup();
				return 1;
			}
			printf("CLIENT bytes sent: %ld\n", works);
			printf("CLIENT message sent: ");
			printf(message.c_str());
			printf("\n\n");
			sender = 1 - sender;
		}
		else {
			works = recv(connecter, receiver, (int)strlen(receiver), 0); // Receive message from server
			if (works == 0) {
				printf("Connection closed\n"); // Done receiving messages
			}
			else if (works < 0) {
				printf("Error during receive\n"); // Error encountered
			}
			else {
				if (receiver[0] != '#') {
					printf("CLIENT bytes received: %d\n", works);
					printf("CLIENT message received: ");
					printf("%.*s\n", works, receiver);
					printf("\n");
					sender = 1 - sender;
				}
			}
		}
	} while (works > 0 && message[0] != '#' && receiver[0] != '#');

	/* Shutdown client but still can receive data */
	works = shutdown(connecter, SD_SEND);
	if (works == SOCKET_ERROR) {
		printf("Error during shutdown\n");
		closesocket(connecter);
		WSACleanup();
		return 1;
	}

	/* Final stages of cleanup */
	closesocket(connecter);
	WSACleanup();
	printf("Connection closed\n");
	return 0;
}