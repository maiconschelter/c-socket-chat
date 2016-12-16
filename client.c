#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment (lib, "ws2_32.lib")

unsigned __stdcall receive_message(SOCKET acct)
{
	int 	rsize;
	char response[2000];

	for(;;)
	{
		rsize = recv(acct, response, 2000, 0);
		if(rsize > 0)
		{
			response[rsize] = '\0';
			printf("\r\n\r\nNICK=> %s\r\nYOU=> ", response);
			if(strstr(response, "quit") != NULL)
			{
				closesocket(acct);
				WSACleanup();
				printf("NICK quit from chat\r\n");
				break;
			}
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	WSADATA wsa;
	SOCKET 	sckt;
	struct 		sockaddr_in client;
	char 		message[200], hostname[200], response[2000];
	int 			rsize;
	uint16_t	portno;
	HANDLE	caller;

	if(argv[1])
		strcpy(hostname, argv[1]);
	else
		strcpy(hostname, "127.0.0.1");

	if(argv[2])
		portno = strtol(argv[2], NULL, 0);
	else
		portno = 65400;

	printf("Client starting in %s:%d\r\n", hostname, portno);

	if(WSAStartup(MAKEWORD(2,2), &wsa) != 0)
	{
		printf("Failed in WSAStartup - %d\r\n", WSAGetLastError());
		return 1;
	}

	if((sckt = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket - %d\r\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	client.sin_family = AF_INET;
	client.sin_addr.s_addr = inet_addr(hostname);
	client.sin_port = htons(portno);
	
	if(connect(sckt, (struct sockaddr *)&client, sizeof(client)) < 0)
	{
		printf("Could connect on server - %d\r\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	rsize = recv(sckt, response, 2000, 0);
	if(rsize > 0)
	{
		response[rsize] = '\0';
		printf(response);
	}

	caller = (HANDLE)_beginthreadex(NULL, 0, (void *)receive_message, sckt, 0, NULL);
	if(caller == NULL)
	{
		printf("Failed in create thread: %s\n", strerror(errno));
		return 1;
	}

	do
	{
		printf("\r\nYOU=> ");
		fgets(message, MAXCHAR, stdin);
		if(send(sckt, message, strlen(message), 0) < 0)
		{
			printf("Could send message - %d\r\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		if(strstr(message, "quit") != NULL)
		{
			break;
		}
	}
	while(TRUE);

	closesocket(sckt);
	WSACleanup();

	return 0;
}