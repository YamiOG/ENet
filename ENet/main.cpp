#include <iostream>
#include <vector>
#include <string>

#include <SDL.h>
#include <SDL_net.h>

#include "ENet.h"

using namespace std;

//Main
SDL_Window *window;
SDL_Event ev;

//Net
vector<TCPsocket> socket;
vector<bool> activeVec;
IPaddress ip;
string ipAddress;
int idNum = -1;
int socketMax = 30;

//Bools
bool running = true;
bool isOnline = false;
bool consoleData = true;

//ETC
char answer = '0';

void Setup()
{
	//SDL Init
	SDL_Init(SDL_INIT_EVERYTHING);
	SDLNet_Init();

	//Setup Window
	window = SDL_CreateWindow("ENet", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 500, 500, SDL_WINDOW_OPENGL);

	//Makes a vector bool to handle active sockets
	for (int i = 0; i < socketMax; i++)
	{
		//Sets all bools to false to represent unactive
		bool tmpBool = false;
		activeVec.push_back(tmpBool);
	}

	//Network Setup
	//Finds if user wants a client or server
	cout << "Setup Complete" << endl;
	cout << "Input S for server and C for Client: ";
	cin >> answer;

	if (tolower(answer) == 'c')
	{
		//Gets the wanted IP
		cout << "Input IP(127.0.0.1 for LocalHost): ";
		cin >> ipAddress;

		//Setup SDLNet Based on Data
		ENet_Init(answer, socket, activeVec, idNum, ip, ipAddress, 25565, consoleData);
	}
	if (tolower(answer) == 's')
	{
		//Setup SDLNet Based on Data
		ENet_Init(answer, socket, activeVec, idNum, ip, " ", 25565, consoleData);
	}
}

int main(int argc, char *argv[])
{
	//Sets up variables
	Setup();

	while (running == true)
	{
		while (SDL_PollEvent(&ev))
		{
			//Checks if Attempting to Close from Window
			if (ev.type == SDL_QUIT)
			{
				if (isOnline == true)
				{
					if (answer == 'c')
					{
						ENet_CloseServer(socket, 0, idNum, consoleData);
					}
				}
				running = false;
			}
		}

		//Network Code
		if (tolower(answer) == 'c')
		{
			//Trying to Open a Server
			ENet_OpenServer(ip, socket[0], idNum, isOnline, consoleData);

			//Handlers Server data and sends data
			ENet_ClientHandler(socket[0], idNum, isOnline, consoleData);
		}
		if (tolower(answer) == 's')
		{
			//Adds Clients when clients try to connect
			ENet_AddClient(socket, 0, activeVec, socketMax, isOnline, consoleData);

			//Handles Client data and Sends data
			ENet_ServerHandler(socket, 0, activeVec, idNum, isOnline, consoleData);
		}
	}

	//After Running
	if (answer == 's')
	{
		//Clean up Server
		ENet_ServerCleanUp(socket, activeVec, idNum, consoleData);
	}

	SDLNet_Quit();
	SDL_Quit();
	return 0;
}
