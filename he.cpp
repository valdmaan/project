#include "winsock2.h"
#include "unistd.h"
#include <random>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
using namespace std::chrono;
int main() {
	ofstream os("output.txt");
	mt19937 gen(time(NULL));				   // message size generator
	uniform_int_distribution<> uid(10, 50000); // in range [10; 50000]

	WSADATA wsaData;
	SOCKET ListenSocket;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "# WSAStartup opening is invalid!"; os << "# WSAStartup opening is invalid!" << endl;
		getchar();
		return 1;
	}
	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == 0) {
		cout << "# Socket creation error"; os << "# Socket creation error" << endl;
		getchar();
		return 1;
	}
	sockaddr_in service;
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = inet_addr("127.0.0.1");
	service.sin_port = htons(1048);

	if (bind(ListenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
		cout << "# Socket binding error"; os << "# Socket binding error" << endl;
		closesocket(ListenSocket);
		getchar();
		return 1;
	}
	if (listen(ListenSocket, 1) == SOCKET_ERROR) {
		cout << "# Error listening on socket"; os << "# Error listening on socket" << endl;
	}
	cout << "# Listening on socket...\n"; os << "# Listening on socket..." << endl;
	SOCKET AcceptSocket;
	cout << "# Waiting for client to connect...\n"; os << "# Waiting for client to connect...\n" << endl;
	while (1) {
		AcceptSocket = SOCKET_ERROR;
		while (AcceptSocket == SOCKET_ERROR) {
			AcceptSocket = accept(ListenSocket, NULL, NULL);
		}
		time_t now = time(0); char* dt = ctime(&now); //local time variable for log
		system("cls");
		cout << "# Client connected.\n"; os << dt << "   Client connected\n" << endl;
		ListenSocket = AcceptSocket;
		break;
	}

	while (true) {
		char recvbuf[15];
		cout << "\n# Waiting for a command from a client...\n\n";
		if (recv(AcceptSocket, (char*)&recvbuf, sizeof(recvbuf), 0) == -1) {
			time_t now = time(0); char* dt = ctime(&now); //local time variable for log
			cout << "Client closed\n"; os << dt << "   Client closed" << endl;
			sleep(1);
			closesocket(ListenSocket);
			closesocket(AcceptSocket);
			WSACleanup();
			return 0;
		}
		if (strcmp(recvbuf, "swho          ") == 0) { //who
			char whoBuf[45] = "Dmitriy Valdman, #23, Testuvannya merezhi\n";
			send(AcceptSocket, whoBuf, sizeof(whoBuf), 0);
			time_t now = time(0); char* dt = ctime(&now);//local time variable for log
			cout << "   Sent information about the creator\n"; os << dt << "   Sent information about the creator\n\n";
		}
		else if (recvbuf[0] == 's' && recvbuf[1] == 'p' && recvbuf[2] == 'r' && recvbuf[3] == 'f') { //testperformance
			int N;//number of messages
			stringstream ss;
			for (size_t i = 4; i < sizeof(recvbuf); ++i) {
				if (recvbuf[i] == ' ')
					break;
				ss << recvbuf[i];
			}
			ss >> N;
			time_t now = time(0); char* dt = ctime(&now); //local time variable for log
			cout << "   Performance testing...\n   " << N << " messages will be sent from a server\n\n";
			os << dt << "   Performance testing...\n   " << N << " messages will be sent from a server\n\n";

			for (int i = 0; i < N; ++i) {
				//RECEIVING A BUFFER

				int num;
				//Receiving buffer size from a client
				recv(AcceptSocket, (char*)&num, 4, 0);
				//Receive buffer
				char recvbuf[num];
				//Receiving buffer from a client
				recv(AcceptSocket, recvbuf, sizeof(recvbuf), 0);
				//TIME when received
				auto end1 = steady_clock::now();
				//Sending it to a client;
				send(AcceptSocket, (char*)&end1, 8, 0);

				cout << left << setw(14)
					<< "Received:     " << right << setw(5)
					<< num << " bytes\n";

				//SENDING A BUFFER

				//Random buffer size generation
				num = uid(gen);
				//Sending size to a client
				send(AcceptSocket, (char*)&num, 4, 0);
				////Send buffer
				char sendbuf[num];
				//TIME when sent
				auto start2 = steady_clock::now();
				//Buffer sending to a client
				send(AcceptSocket, sendbuf, sizeof(sendbuf), 0);
				//Sending TIME to a client;
				send(AcceptSocket, (char*)&start2, 8, 0);

				cout << left << setw(14)
					<< "Sent:     " << right << setw(5)
					<< num << " bytes\n";
			}
		}
		else if (strcmp(recvbuf, "sxit          ") == 0) { //exit
			time_t now = time(0); char* dt = ctime(&now); //local time variable for log
			cout << "Client exited\n"; os << dt << "   Client exited" << endl;
			sleep(1);
			closesocket(ListenSocket);
			closesocket(AcceptSocket);
			WSACleanup();
			return 0;
		}
		else {
			cout << "UNKNOWN ERROR\n";
			sleep(2);
			closesocket(ListenSocket);
			closesocket(AcceptSocket);
			WSACleanup();
			break;
		}
	}
}

