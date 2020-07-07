/* Model for the Server */

#undef UNICODE 
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

#define DEFAULT_PORT "27015"
#define DEFAULT_LENGTH 512

/* Alternate declaration of the method
	int __cdecl main(void)
*/

using namespace std;
int main(void) {
	/* Relevant variables */
	WSADATA wsaData;
	int works;
	struct addrinfo* result = NULL;
	struct addrinfo hints;
	int sendWorks = 1;
	char receiver[DEFAULT_LENGTH];
	int receiveLength = DEFAULT_LENGTH;
	int sender = 0;
	string message;

	/* Initialize Winsock */
	works = WSAStartup(MAKEWORD(2, 2), &wsaData); // Same as client
	if (works != 0) {
		printf("Error on WSAStartup\n");
		return 1; // Indicates error encountered
	}
	ZeroMemory(&hints, sizeof(hints)); // Fills block of memory w/ 0's
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE; // Bind function uses returned address structure

	/* Getting local address/port */
	works = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (works != 0) {
		printf("Error on getaddrinfo\n");
		WSACleanup();
		return 1;
	}
	/* Create the listener socket */
	SOCKET listener = INVALID_SOCKET;
	listener = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listener == INVALID_SOCKET) {
		printf("Error on creating listener socket\n");
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	/* Bind the socket */
	works = bind(listener, result->ai_addr, (int)result->ai_addrlen);
	if (works == SOCKET_ERROR) {
		printf("Error on bind\n");
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result); // Don't need address info after bind function

	/* Listen on a socket */
	if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
		/*
		listen() function has a maximum queue length for pending connection
		set to SOMAXCONN (est. reasonable # of pending connections)
		*/
		printf("Error on listen\n");
		closesocket(listener);
		WSACleanup();
		return 1;
	}

	/* Accepting connection to a socket */
	SOCKET client = INVALID_SOCKET;
	client = accept(listener, NULL, NULL);
	if (client == INVALID_SOCKET) {
		printf("Error on accepting connection\n");
		closesocket(listener);
		WSACleanup();
		return 1;
	}


	/* Send and receive data */
	printf("Press '#' to end the connection\n");
	do {
		if (sender == 1) {
			printf("Please enter the server response: ");
			getline(cin, message);
			works = send(client, message.c_str(), message.length(), 0);
			if (works == SOCKET_ERROR) {
				printf("Error on sending message\n");
				closesocket(client);
				WSACleanup();
				return 1;
			}
			else {
				printf("SERVER bytes sent: %d\n", works);
				printf("SERVER message sent: ");
				printf("%s", message.c_str());
				printf("\n\n");
			}
			sender = 1 - sender;
		}
		else {
			works = recv(client, receiver, (int)strlen(receiver), 0);
			if (works > 0) {
				if (receiver[0] != '#') {
					printf("SERVER bytes received: %d\n", works);
					printf("SERVER message received: ");
					printf("%.*s\n", works, receiver);
					printf("\n");
					sender = 1 - sender;
				}
			}
			else if (works == 0) {
				printf("Connection closed\n");
				break;
			}
			else {
				printf("Error on receive\n");
				WSACleanup();
				return 1;
			}
		}
	} while (works > 0 && receiver[0] != '#' && message[0] != '#');

	/* Shutting down the sending connection */
	works = shutdown(client, SD_SEND);
	if (works == SOCKET_ERROR) {
		printf("Error on shutdown\n");
		closesocket(client);
		WSACleanup();
		return 1;
	}

	/* Cleanup upon receiving data */
	closesocket(client);
	WSACleanup();
	printf("Connection closed\n");
	return 0;
}