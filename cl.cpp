#include "winsock2.h"
#include "unistd.h"
#include <random>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

using namespace std;
using namespace std::chrono;


int main() {
	ofstream os("output.txt");
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "WSAStartup opening is invalid!"; os << "WSAStartup opening is invalid!" << endl;
		getchar();
		return 1;
	}
	SOCKET ConnectSocket;
	ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET) {
		cout << "Socket creation error"; os << "Socket creation error" << endl;
		WSACleanup();
		getchar();
		return 1;
	}
	sockaddr_in clientService;
	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientService.sin_port = htons(1048);
	if (connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService)) == 0) {
		time_t now = time(0); char* dt = ctime(&now); //local time variable for log
		cout << "Connected to server.\n\n"; os << dt << "   Connected to server.\n" << endl;
	}
	else {
		cout << "Failed to connect\n"; os << "Failed to connect\n" << endl;
		closesocket(ConnectSocket);
		WSACleanup();
		sleep(1);
		return 1;
	}

	while (true) {
		int command = 1;
		cout << "The list of commands:\n"
			"[1] help\n"
			"[2] who\n"
			"[3] testperformance\n"
			"[4] exit\n"
			"> Choose a command(1-4): ";
		while (!(cin >> command) || command <= 0 || command > 4) {
			cout << "> Choose a command(1-4): ";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
		}
		switch (command)
		{
		case 1: { //help; works on client side
			time_t now = time(0); char* dt = ctime(&now); //local time variable for log
			os << dt << "   <help> command was entered\n" << endl;
			system("cls");
			cout << "Available commands:\n\n" << left
				<< setw(15) << "  who" << "  :  displays the creator name, the number and the name of the variant\n\n"
				<< setw(15) << "  testperformance" << "  :  sends messages of random sizes between client and server\n\n"
				<< setw(15) << "  exit" << "  :  closes the program\n\n"
				"Press any button to continue...";
			getchar(); getchar(); system("cls");
			break;
		}
		case 2: { //who
			time_t now = time(0); char* dt = ctime(&now); //local time variable for log
			os << dt << "   <swho>() command was entered\n" << endl;
			char sendbuf[15] = "swho          ";
			send(ConnectSocket, (char*)&sendbuf, sizeof(sendbuf), 0);    //sending command{2} to a server
			char recvwhoBuf[45];
			recv(ConnectSocket, recvwhoBuf, sizeof(recvwhoBuf), 0);
			system("cls");
			for (char item : recvwhoBuf) {
				cout << item;
			}
			cout << "\nPress any button to continue...";
			getchar(); getchar();
			system("cls");
			break;
		}
		case 3: { //testperformance
			duration<int64_t, micro>::rep total = 0;
			double totalsize;
			int minSize;
			int maxSize;
			mt19937 gen(time(NULL));				   // message size generator
			uniform_int_distribution<> uid(10, 50000); // in range [10; 50000]
			int N = 1;
			cout << "\n> Enter the number of messages to be sent from a client: ";
			while (!(cin >> N) || N <= 0 || N > 999999999) {
				cout << setw(58) << left << "\n> Please, enter a positive number (1 - 999999999): ";
				cin.clear();
				cin.ignore(numeric_limits<streamsize>::max(), '\n');
			}
			cout << endl;
			stringstream ss;
			ss << "sprf" << N;
			string sendString = ss.str();
			while (sendString.size() != 15) {
				sendString.push_back(' ');
			}
			char sendbuf[15];
			for (int i = 0; i < 15; ++i) {
				sendbuf[i] = sendString[i];
			}
			send(ConnectSocket, (char*)&sendbuf, sizeof(sendbuf), 0);   //sending command{3} to a server
			time_t now = time(0); char* dt = ctime(&now);
			os << dt << "   <sprf>(" << N << ")command was entered" << endl;
			cout << "   Performance testing started...\n\n";
			os << "   Performance testing started...\n   " << N << " messages will be sent from a client\n" << endl;
			for (int i = 0; i < N; ++i) {
				//SENDING A BUFFER

				//Random buffer size generation
				int num = uid(gen);
				if (i == 0) {
					minSize = num; maxSize = num;
				}
				else {
					if (num < minSize)
						minSize = num;
					if (num > maxSize)
						maxSize = num;
				}
				totalsize += num;

				//Sending buffer size to a server
				send(ConnectSocket, (char*)&num, 4, 0);
				//Send buffer
				char sendbuf[num];
				//TIME when sending from a client
				auto start1 = steady_clock::now();
				//Buffer sending to a server
				send(ConnectSocket, sendbuf, sizeof(sendbuf), 0);
				//TIME when received on server
				steady_clock::time_point end1;
				//Receiving it from a server
				recv(ConnectSocket, (char*)&end1, 8, 0);

				cout << left << setw(14) << "Sent:     " << right << setw(5)
					<< num << " bytes " << "in "
					<< setw(5) << duration_cast<microseconds>(end1 - start1).count()
					<< " microseconds\n";
				total += duration_cast<microseconds>(end1 - start1).count();

				//RECEIVING A BUFFER

				//Receiving buffer size from a server
				recv(ConnectSocket, (char*)&num, 4, 0);
				if (num < minSize)
					minSize = num;
				if (num > maxSize)
					maxSize = num;
				totalsize += num;

				//Receive buffer
				char recvbuf[num];
				//Receiving buffer from a server
				recv(ConnectSocket, recvbuf, sizeof(recvbuf), 0);
				//TIME when received
				auto end2 = steady_clock::now();
				//TIME when sent from a server
				steady_clock::time_point start2;
				//Receiving it from a server
				recv(ConnectSocket, (char*)&start2, 8, 0);

				cout << left << setw(14) << "Received:     "
					<< right << setw(5) << num << " bytes " << "in "
					<< setw(5) << duration_cast<microseconds>(end2 - start2).count()
					<< " microseconds\n";
				total += duration_cast<microseconds>(end2 - start2).count();
			}
			cout << "\n   Performance testing ended...\n"
				<< left << "\n# " << N * 2 << " random size messages sent from client and server at all\n"
				<< setw(28) << "# Total time: " << total << " microseconds\n"
				<< setw(28) << "# Average message size: " << totalsize / (N * 2) << " bytes\n"
				<< setw(28) << "# Average time per message: " << total / (N * 2) << " microseconds\n"
				<< setw(28) << "# Min message size: " << minSize << "\n"
				<< setw(28) << "# Max message size: " << maxSize << "\n\n"
				"Press any button to continue...\n";
			{time_t now1 = time(0); char* dt1 = ctime(&now1); //local time variable for log
			os << dt1 << "   Performance testing ended...\n"; }
			os << left << "\n# " << N * 2 << " random size messages sent from client and server at all\n"
				<< setw(28) << "# Total time: " << total << " microseconds\n"
				<< setw(28) << "# Average message size: " << totalsize / (N * 2) << " bytes\n"
				<< setw(28) << "# Average time per message: " << total / (N * 2) << " microseconds\n"
				<< setw(28) << "# Min message size: " << minSize << "\n"
				<< setw(28) << "# Max message size: " << maxSize << "\n" << endl;
			getchar(); getchar();
			break;
		}
		case 4: { //exit
			char sendbuf[15] = "sxit          ";
			send(ConnectSocket, (char*)&sendbuf, sizeof(sendbuf), 0);
			time_t now = time(0); char* dt = ctime(&now); //local time variable for log
			cout << "Exiting..."; os << dt << "   <sxit>() command was entered\n" << "   Client exited" << endl;
			sleep(1);
			closesocket(ConnectSocket);
			WSACleanup();
			return 0;
		}
		default: {
			system("cls");
			cout << "UNKNOWN ERROR, try again...\n\n"
				"Press any button to continue...";
			getchar(); getchar(); system("cls");
			break;
		}
		}

	}
}
