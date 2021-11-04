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



class Networking {
	private:
		SOCKET s;
		struct sockaddr_in sother;
		int slen = sizeof(sother);
	


	public:
		void start() {
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
		}



		void stop() {
			closesocket(s);
			WSACleanup();
		}



        void recv(NET_RECVDATA data) {
            cout << "[RECV] (" << data.address << ":" << data.port << ") -> " << data.data << endl;
        }



		void send(const char buffer[]) {
            string address = inet_ntoa(sother.sin_addr);
            unsigned short port = ntohs(sother.sin_port);

            int send_len = sendto(s, buffer, strlen(buffer), 0, (SOCKADDR*)&sother, slen);
            if (send_len == SOCKET_ERROR) {
                cout << "[SEND] (" << address << ":" << port << ") Error - didn't send anything! (EC:" << WSAGetLastError() << ")" << endl;
            } else {
                cout << "[SEND] (" << address << ":" << port << ") <- " << buffer << endl;
            }
		}



		NET_RECVDATA waitAndReceive() {
            char buffer[NET_BUFFERLEN];
			memset(buffer, '\0', NET_BUFFERLEN);
			int recv_len = recvfrom(s, buffer, NET_BUFFERLEN, 0, (SOCKADDR*)&sother, &slen);
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
			return result;
		}



		void loop() {
			// trzeba cos wyslac, inaczej recv zwroci blad
			send("hello");
			
			while (true) {
				NET_RECVDATA data = waitAndReceive();
				recv(data);
			}
		}
};

Networking netw = Networking();


void netMain() {
	netw.start();
	netw.loop();
}


int main() {


	thread net(netMain);

	//start communication
	char message[NET_BUFFERLEN];
	while (true)
	{
		cout << "Enter message: ";
		cin >> message;

		//send the message
		netw.send(message);
	}

	netw.stop();
	net.join();



	return 0;
}
