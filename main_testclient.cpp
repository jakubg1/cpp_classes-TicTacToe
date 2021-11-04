#include <iostream>
#include <thread>

#include <stdio.h>
#include <winsock2.h>

using namespace std;



// consts
const char SERVER[] = "127.0.0.1";	//ip address of udp server
const short NET_PORT = 40332;	//The port on which to listen for incoming data
const short NET_BUFFERLEN = 512;	//Max length of buffer

struct NET_RECVDATA {
    string address;
    unsigned short port;
    char data[NET_BUFFERLEN];
};


struct sockaddr_in sother;
int s, slen = sizeof(sother);



int main_recv() {
	char buffer[NET_BUFFERLEN];

	// trzeba cos wyslac, inaczej recv zwroci blad
	sendto(s, "hello", 5, 0, (struct sockaddr*)&sother, slen);
	
	while (true) {
		memset(buffer, '\0', NET_BUFFERLEN);
		int recv_len = recvfrom(s, buffer, NET_BUFFERLEN, 0, (struct sockaddr*)&sother, &slen);
		if (recv_len == SOCKET_ERROR) {
			// normalnie tutaj powinnismy wejsc tylko gdy z zewnatrz zamkniemy socket
			// wtedy nie ma gdzie wyslac i wysyla error
			cout << "[RECV] Error - networking terminated! (EC:" << WSAGetLastError() << ")" << endl;
			exit(EXIT_FAILURE);
		}

		NET_RECVDATA result;
		result.address = inet_ntoa(sother.sin_addr);
		result.port = ntohs(sother.sin_port);
		for (short i = 0; i < NET_BUFFERLEN; i++) {
			result.data[i] = buffer[i];
		}

		cout << "[RECV] (" << result.address << ":" << result.port << ") -> " << result.data << endl;
	}
}



int main() {
	char message[NET_BUFFERLEN];

	// otwieramy bramke SMS
	WSADATA wsaData;

	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != NO_ERROR) {
		cout << "Network error 1" << endl;
	}

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (s == INVALID_SOCKET) {
		cout << "Socket error 1" << endl;
		//WSACleanup();
	}

	//setup address structure
	memset((char*)&sother, 0, sizeof(sother));
	sother.sin_family = AF_INET;
	sother.sin_port = htons(NET_PORT);
	sother.sin_addr.S_un.S_addr = inet_addr(SERVER);

	thread recv(main_recv);

	//start communication
	while (true)
	{
		cout << "Enter message: ";
		cin >> message;

		//send the message
		if (sendto(s, message, strlen(message), 0, (struct sockaddr*)&sother, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
	}

	closesocket(s);
	WSACleanup();

	recv.join();



	return 0;
}
