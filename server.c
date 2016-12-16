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
	SOCKET 	sckt, acct;
	struct 		sockaddr_in server, client;
	int 			csize, rsize, threadnum;
	uint16_t	portno;
	char 		*message, sender[200], response[2000], hostname[256];
	HANDLE	caller;

	if(argv[1])
		strcpy(hostname, argv[1]);
	else
		strcpy(hostname, "127.0.0.1");

	if(argv[2])
		portno = strtol(argv[2], NULL, 0);
	else
		portno = 65400;

	printf("Server started in %s:%d\r\n", hostname, portno);

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

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = inet_addr(hostname);
	server.sin_port = htons(portno);

	if(bind(sckt, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Server bind failed - %d\r\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	if(listen(sckt, SOMAXCONN) == SOCKET_ERROR)
	{
		printf("Failed in listen - %d\r\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	csize = sizeof(struct sockaddr_in);
	acct = accept(sckt, (struct sockaddr *)&client, &csize);

	if(acct == INVALID_SOCKET)
	{
		printf("Failed in accept connections - %d\r\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}
	else
	{
		printf("\r\nWelcome!\r\n\r\n");
	}

	message = "\r\nWelcome!\r\n\r\n";
	send(acct, message, strlen(message), 0);
	closesocket(sckt);

	caller = (HANDLE)_beginthreadex(NULL, 0, (void *)receive_message, acct, 0, NULL);
	if(caller == NULL)
	{
		printf("Failed in create thread: %s\n", strerror(errno));
		return 1;
	}

	for(;;)
	{
		printf("\r\nYOU=> ");
		fgets(sender, MAXCHAR, stdin);
		if(send(acct, sender, strlen(sender), 0) < 0)
		{
			printf("Could send message - %d\r\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}
		if(strstr(sender, "quit") != NULL)
		{
			break;
		}
	}

	shutdown(acct, SD_SEND);
	closesocket(acct);
	WSACleanup();

	return 0;
}