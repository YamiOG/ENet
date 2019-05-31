#include <iostream>
#include <vector>
#include <string>

#include <SDL.h>
#include <SDL_net.h>

using namespace std;

//ENet Network Data Transfer Format
struct ENet_Data
{
	int id, time;
	char command;
};

int ENet_InactiveBool(vector<bool> activeVec)
{
	int inactiveNum = 0;
	for (int i = 0; i < activeVec.size(); i++)
	{
		if (activeVec[i] == false)
		{
			inactiveNum = i;
			break;
		}
	}
	return inactiveNum;
}

int ENet_ActiveBoolNum(vector<bool> activeVec)
{
	int activeNum = 0;
	for (int i = 0; i < activeVec.size(); i++)
	{
		if (activeVec[i] == true)
		{
			activeNum++;
		}
	}
	return activeNum;
}

int ENet_Init(char answer, vector<TCPsocket> &socket, vector<bool> &activeVec, int &idNum, IPaddress &ip, string ipAddress, int port, bool consoleData)
{
	if (tolower(answer) == 'c')
	{
		if (consoleData == true)
		{
			cout << "Setting Up Client" << endl;
		}

		//Makes Client Socket
		TCPsocket tmpSocket = NULL;
		socket.push_back(tmpSocket);

		//Sets up Ip Variable
		SDLNet_ResolveHost(&ip, ipAddress.c_str(), port);
	}
	if (tolower(answer) == 's')
	{
		if (consoleData == true)
		{
			cout << "Setting Up Server" << endl;
		}

		//Makes Server Socket
		TCPsocket tmpSocket = NULL;
		socket.push_back(tmpSocket);

		//Sets up Ip Variable
		SDLNet_ResolveHost(&ip, NULL, port);

		//Sets Id equal to 0 and setups the Server Socket
		idNum = 0;
		activeVec[idNum] = true;
		socket[0] = SDLNet_TCP_Open(&ip);
	}

	if (consoleData == true)
	{
		cout << "Setup Sucessful" << endl;
	}

	return 0;
}

void ENet_OpenServer(IPaddress ip, TCPsocket &socket, int &idNum, bool &isOnline, bool consoleData)
{
	if (isOnline == false)
	{
		if (!socket)
		{
			//Trys to Open Server
			socket = SDLNet_TCP_Open(&ip);
			if (consoleData == true)
			{
				cout << "Attempting to Open Server: " << endl;
			}
		}
		if (socket)
		{
			bool connected = false;
			int recvId = -1;
			//Receives if connected or not
			SDLNet_TCP_Recv(socket, &connected, sizeof(connected));
			if (connected == true)
			{
				//Receives id on server
				SDLNet_TCP_Recv(socket, &recvId, sizeof(recvId));
				idNum = recvId;

				ENet_Data tmpData;
				tmpData.id = idNum;
				tmpData.time = SDL_GetTicks();
				tmpData.command = '0';

				//Sends Data to Server
				SDLNet_TCP_Send(socket, &tmpData, sizeof(tmpData));

				//Sets Client to Online
				isOnline = true;
				if (consoleData == true)
				{
					cout << "Connection Successful" << endl;
				}
			}
		}
	}
}

void ENet_ClientHandler(TCPsocket &socket, int &idNum, bool &isOnline, bool consoleData)
{
	if (isOnline == true)
	{
		if (socket)
		{
			ENet_Data tmpData;
			int connectedNum = 0;
			bool kick = false;

			//Receives Connected Clients Num
			SDLNet_TCP_Recv(socket, &connectedNum, sizeof(connectedNum));

			for (int i = 0; i < connectedNum; i++)
			{
				//Receives Data
				SDLNet_TCP_Recv(socket, &tmpData, sizeof(tmpData));
				//If Command 0 Receiving Data
				if (tmpData.command == '0')
				{
					if (consoleData == true)
					{
						cout << "Recv: " << tmpData.id << " - " << tmpData.time << endl;
					}
				}
				//If Command 1 Client Closing
				if (tmpData.command == '1')
				{
					if (consoleData == true)
					{
						cout << "Recv: Closing Client" << endl;
					}

					//tmpData.id == inactive
				}
				//If Command 2 kicked from Server or Server shutdown 
				if (tmpData.command == '2')
				{
					SDLNet_TCP_Close(socket);
					socket = NULL;
					isOnline = false;
					kick = true;
					if (consoleData == true)
					{
						cout << "Recv: Kicked From Server or Server Shutdown" << endl;
					}
				}
			}

			//Checks if Socket Was closed or not
			if (kick == false && connectedNum > 0)
			{
				tmpData.id = idNum;
				tmpData.time = SDL_GetTicks();
				tmpData.command = '0';

				if (consoleData == true)
				{
					cout << "Send: " << tmpData.id << " - " << tmpData.time << endl;
				}

				//Sends Data
				SDLNet_TCP_Send(socket, &tmpData, sizeof(tmpData));
			}
		}
	}
}

void ENet_AddClient(vector<TCPsocket> &socket, int arrayNum, vector<bool>& activeVec, int socketMax, bool &isOnline, bool consoleData)
{ 
	//Makes a Tmp Socket to receive client with
	TCPsocket tmpSocket = NULL;
	if (!tmpSocket)
	{
		//Accepts Incoming Clients
		tmpSocket = SDLNet_TCP_Accept(socket[arrayNum]);
	}
	if (tmpSocket)
	{
		if (consoleData == true)
		{
			cout << "Client Attempting to Connect" << endl;
		}

		//Checks if Server Max isn't reached
		if (ENet_ActiveBoolNum(activeVec) < socketMax)
		{
			//Sends that the client is connected
			bool connected = true;
			SDLNet_TCP_Send(tmpSocket, &connected, sizeof(bool));

			//Sends Id
			int tmpNum = ENet_InactiveBool(activeVec);
			activeVec[tmpNum] = true;
			SDLNet_TCP_Send(tmpSocket, &tmpNum, sizeof(int));

			//Adds Client to Socket Vector
			socket.push_back(tmpSocket);
			
			//Sets Server Online to true
			isOnline = true;
			if (consoleData == true)
			{
				cout << "New Client Connected" << endl;
			}
		}
		else
		{
			//Sends that the client isn't connected
			bool connected = false;
			SDLNet_TCP_Send(tmpSocket, &connected, sizeof(bool));
		}
	}
}

void ENet_ServerHandler(vector<TCPsocket>& socket, int arrayNum, vector<bool>& activeVec, int& idNum, bool isOnline, bool consoleData)
{
	if (isOnline == true)
	{
		for (int i = 0; i < socket.size(); i++)
		{
			if (socket[i] && i != arrayNum)
			{
				ENet_Data tmpData;

				//Receives Data
				SDLNet_TCP_Recv(socket[i], &tmpData, sizeof(tmpData));
				//If Command 0 Receiving Data
				if (tmpData.command == '0')
				{
					if (consoleData == true)
					{
						cout << "Recv: " << tmpData.id << " - " << tmpData.time << endl;
					}
				}
				//If Command 1 Client Closing
				if (tmpData.command == '1')
				{
					if (consoleData == true)
					{
						cout << "Recv: Closing Client" << endl;
					}

					//Closes and Erases Socket from Vector
					SDLNet_TCP_Close(socket[i]);
					socket[i] = NULL;
					socket.erase(socket.begin() + i);

					//Sets Id Inactive
					activeVec[i] = false;

					//Sends Data to Other Clients
					for (int k = 0; k < socket.size(); k++)
					{
						if ((k == i) || (k == 0))
						{
							continue;
						}
						SDLNet_TCP_Send(socket[k], &tmpData, sizeof(tmpData));
					}
					//Skips rest of for loop if command equals 1
					continue;
				}

				//Sends Received Data to Other Clients
				for (int k = 0; k < socket.size(); k++)
				{
					if ((k == i) || (k == 0))
					{
						continue;
					}
					SDLNet_TCP_Send(socket[k], &tmpData, sizeof(tmpData));
				}

				//Sends Connected Number of Client
				int connectedNum = ENet_ActiveBoolNum(activeVec) - 1;
				SDLNet_TCP_Send(socket[i], &connectedNum, sizeof(connectedNum));

				//Sends Server Data to Client
				tmpData.id = idNum;
				tmpData.time = SDL_GetTicks();
				tmpData.command = '0';
				if (consoleData == true)
				{
					cout << "Send: " << tmpData.id << " - " << tmpData.time << endl;
				}

				SDLNet_TCP_Send(socket[i], &tmpData, sizeof(tmpData));
			}
		}
	}
}

void ENet_CloseServer(vector<TCPsocket> &socket, int arrayNum, int idNum, bool consoleData)
{
	ENet_Data tmpData;
	tmpData.id = idNum;
	tmpData.time = -1;
	tmpData.command = '1';

	//Sends Server Close Command
	SDLNet_TCP_Send(socket[arrayNum], &tmpData, sizeof(tmpData));

	//Allows for Server to Process Data
	SDL_Delay(1000);

	//Closes Server
	SDLNet_TCP_Close(socket[arrayNum]);
	socket[arrayNum] = NULL;

	if (consoleData == true)
	{
		cout << "Closed Server" << endl;
	}
}

void ENet_ServerCleanUp(vector<TCPsocket>& socket, vector<bool>& activeVec, int &idNum, bool consoleData)
{
	ENet_Data tmpData;
	tmpData.id = idNum;
	tmpData.time = -1;
	tmpData.command = '2';

	int connectedNum = 1;

	//Closes Sockets
	for (int i = 0; i < socket.size(); i++)
	{
		if (socket[i])
		{
			SDLNet_TCP_Send(socket[i], &connectedNum, sizeof(connectedNum));
			SDLNet_TCP_Send(socket[i], &tmpData, sizeof(tmpData));
			SDL_Delay(1000);
			SDLNet_TCP_Close(socket[i]);
			socket[i] = NULL;
			activeVec[i] = false;
		}
	}

	//Sets Id
	idNum = -1;

	if (consoleData == true)
	{
		cout << "Server Cleaned Up" << endl;
	}
}