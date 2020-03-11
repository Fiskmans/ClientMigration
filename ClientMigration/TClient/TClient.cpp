#include <pch.h>
#include "TClient.h"
#include <iostream>
#include <string>
#include <SetupMessage.h>
#include <thread>
#include <chrono>
#include <functional>
#include <StatusMessage.h>
#include "TimeHelper.h"

#define BUFLEN 576
#define SERVERPORT 5763
#define PORT 5764

bool isRunning = true;

void ReplaceLastLine(const std::string& aLine)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleScreenBufferInfo(console, &info) == TRUE)
	{
		info.dwCursorPosition.Y -= 1;
		SetConsoleCursorPosition(console, info.dwCursorPosition);
		std::cout << std::string(80, ' ') + "\r" + aLine + "\n";
	}
	else
	{
		std::cout << GetLastError() << std::endl;
	}
}

void InsertLine(const std::string& aLine)
{
	std::cout << aLine << std::endl;
	return;
	CONSOLE_SCREEN_BUFFER_INFO info;
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
	if (GetConsoleScreenBufferInfo(console, &info) == TRUE)
	{
		CHAR_INFO data[80];
		COORD bufferSize;
		bufferSize.X = 80;
		bufferSize.Y = 1;
		COORD ReadPosition;
		ReadPosition.X = 0;
		ReadPosition.Y = info.dwCursorPosition.Y;
		SMALL_RECT searchRect;
		searchRect.Left = ReadPosition.X;
		searchRect.Top = ReadPosition.Y;
		searchRect.Right = ReadPosition.X + 80;
		searchRect.Bottom = ReadPosition.Y + 1;
		if (ReadConsoleOutputA(console, data, bufferSize, ReadPosition, &searchRect) == FALSE)
		{
			std::cout << GetLastError();
		}
		ReplaceLastLine(aLine);
		COORD writePosition;
		writePosition = ReadPosition;
		writePosition.Y += 1;
		WriteConsoleOutputA(console, data, bufferSize, writePosition, &searchRect);
		COORD newCursorPos;
		newCursorPos = info.dwCursorPosition;
		newCursorPos.Y += 1;
		SetConsoleCursorPosition(console, newCursorPos);
	}
	else
	{
		std::cout << GetLastError() << std::endl;
	}
}


NetworkClient::NetworkClient()
{
	Flush();
}



void NetworkClient::Connect()
{
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	slen = sizeof(myTargetAddress);

	//Initialise winsock
	std::cout << "Initialising Winsock...\n";
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "Failed.Error Code: " + std::to_string(WSAGetLastError()) + "\n";
		return;
	}
	std::cout << "Initialised.\n";

	//Create a socket
	mySocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mySocket == INVALID_SOCKET)
	{
		std::cout << "Could not create socket: " + std::to_string(WSAGetLastError()) + "\n";
		return;
	}
	std::cout << "Socket created.\n";


	memset(&myTargetAddress, 0, sizeof(myTargetAddress));
	myTargetAddress.sin_family = AF_INET;
	myTargetAddress.sin_port = htons(SERVERPORT);

	//std::future<std::variant<int, std::string>> selectedServer = std::async(&NetworkClient::SelectServer,this);
	//while (!selectedServer.valid())
	//{
	//	std::this_thread::yield();
	//}


	u_long mode = 1;  // 1 to enable non-blocking socket
	ioctlsocket(mySocket, FIONBIO, &mode);

	std::cout << "Got address info!\n";

	sockaddr_in sockAddr;

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(PORT);
	sockAddr.sin_addr.S_un.S_addr = INADDR_ANY; // use default

	//Bind
	if (bind(mySocket, reinterpret_cast<sockaddr*>(&sockAddr), sizeof(sockAddr)))
	{
		std::cout << "Bind failed with error code : " + std::to_string(WSAGetLastError()) + "\n";
		return;
	}

	//using namespace std::chrono_literals;
	//while (selectedServer.wait_for(0ms) != std::future_status::ready)
	//{
	//	Flush();
	//}


	std::string address;
	std::cout << "Enter Adress: ";
	//std::getline(std::cin, address);
	address = "server.mansandersen.com";
	std::cout << "\n";
	if (address.empty())
	{
		address = "127.0.0.1";
	}
	ReplaceLastLine("Enter Adress: " + address);
	if (inet_pton(AF_INET, address.c_str(), &myTargetAddress.sin_addr.S_un.S_addr) != 1)
	{
		struct addrinfo* ptr = NULL;
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_DGRAM;
		hints.ai_protocol = IPPROTO_UDP;

		while(true)
		{
			std::this_thread::yield();
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			int error = getaddrinfo(address.c_str(), NULL, &hints, &ptr);
			if (error)
			{
				error = WSAGetLastError();
				switch (error)
				{
				case WSAHOST_NOT_FOUND:
					std::cout << "host not found";
					break;
				default:
					std::cout << "could not get address info with error: " + std::to_string(error) + "\n";
					break;
				}
			}
			else
			{
				struct addrinfo* result = ptr;

				while (result)
				{
					using namespace std::string_literals;
					char host[256];
					char serv[256];
					if (getnameinfo(result->ai_addr, result->ai_addrlen, host, 256, serv, 256, 0) != NULL)
					{
						std::cout << "wsaerror: " + std::to_string(WSAGetLastError()) + "\n";
					}
					else
					{
						//std::cout << "host: " << host << std::endl;
						//std::cout << "serv: " << serv << std::endl;
						char address[INET6_ADDRSTRLEN];
						if (inet_ntop(result->ai_family, &result->ai_addr, address, INET6_ADDRSTRLEN) != NULL)
						{
							std::cout << "Address: "s + address + "\t" + host + "\n";
							memcpy(&myTargetAddress.sin_addr.S_un.S_addr, &result->ai_addr, result->ai_addrlen);
							myTargetAddress.sin_port = htons(SERVERPORT);
							myTargetAddress.sin_family = result->ai_family;
						}
						else
						{
							std::cout << "could not translate address\n";
						}
					}
					result = result->ai_next;
				}
			}
			//freeaddrinfo(ptr);
		}
	}

	HandShake();

	return;
}

void NetworkClient::HandShake()
{
	char buf[BUFLEN];
	int recv_len;
	int slen;
	slen = sizeof(myTargetAddress);
	myIsConnected = false;

	SetupMessage message;
	DWORD length = MAXIDENTIFIERLENGTH;
	GetUserNameA(message.myIdentifier, &length);
	message.myStep = SetupMessage::SetupStep::Request;
	std::cout << "Enter Username: ";
	std::string username;
	//std::getline(std::cin, username);
	if (!username.empty())
	{
		strcpy_s<MAXIDENTIFIERLENGTH>(message.myIdentifier, username.c_str());
	}
	else
	{
		ReplaceLastLine("Enter Username: " + std::string(message.myIdentifier));
	}
	SetWindowTextA(GetConsoleWindow(), message.myIdentifier);

	myName = username;

	std::cout << "Starting Handshake\n";
	while (!myIsConnected)
	{
		std::cout << "Sending Handshake package\n";
		Send(message);
		bool waitingForResponse = true;
		while (waitingForResponse)
		{
			myIsHandshaking = true;
			Flush();
			if (!myIsHandshaking)
			{
				waitingForResponse = false;
			}
			myIsHandshaking = false;
		}
	}
}

bool NetworkClient::HandShakeAttempt(char* aData, int aDataSize)
{
	if (aDataSize != sizeof(SetupMessage))
	{
		std::cout << "Package of wrong size, got [" + std::to_string(aDataSize) + "] expected [" + std::to_string(sizeof(SetupMessage)) + "]\n";
		return false;
	}

	NetMessage* netMess = reinterpret_cast<NetMessage*>(aData);
	if (netMess->myType == NetMessage::Type::Setup)
	{
		SetupMessage* setupMess = reinterpret_cast<SetupMessage*>(aData);
		if (setupMess->myResponse.myResult)
		{
			std::cout << "Connection to server established with id: " + std::to_string(setupMess->myResponse.myID) + "\n";
			myId = setupMess->myResponse.myID;

			if (myCallback)
			{
				myCallback(*setupMess);
			}
			myIsConnected = true;
		}
		else
		{
			std::cout << "Server Rejected Handshake\n";
		}
	}
	else
	{
		std::cout << "Got invalid netmessageType: " + std::to_string(static_cast<int>(netMess->myType)) + "\n";
		return false;
	}
	myIsHandshaking = false;
	return true;
}

void NetworkClient::TimedOut()
{
	std::cout << "Server timed out\n";
	while (true)
	{

	}
}


void NetworkClient::Disconnect()
{
	StatusMessage message;
	message.myID = myId;
	message.myStatus = StatusMessage::Status::UserDisconnected;
	int slen = sizeof(myTargetAddress);
	SimpleSend(mySocket, message, 0, (struct sockaddr*) & myTargetAddress, slen);
	std::cout << "Bye-bye! :wave:\n";
	isRunning = false;
}

void NetworkClient::SetCallback(std::function<void(NetMessage&)> aFunction)
{
	myCallback = aFunction;
}

void NetworkClient::Flush()
{
	NetworkInterface::Flush();

	if (!myPackageStart)
	{
		myBufferStart = myBuffer;
		myPackageStart = myPackageSize;
		return;
	}

	static size_t dataCount[2] = {};

	if (myBufferStart != myBuffer)
	{
		dataCount[0] += myBufferStart - myBuffer;
	}
	static float lastPrint = Tools::GetTotalTime();
	float now = Tools::GetTotalTime();
	if (now - lastPrint > 1.f)
	{
		lastPrint = now;
		if (dataCount[0] != 0 || dataCount[1] != 0)
		{
			std::cout << "Sent [" + std::to_string(dataCount[0]) + "] bytes\nRecived [" + std::to_string(dataCount[1]) + "] bytes\n";
		}
		dataCount[0] = 0;
		dataCount[1] = 0;
	}
	short* sizePointer = myPackageSize;
	char* dataPointer = myBuffer;
	while (dataPointer != myBufferStart)
	{
		sendto(mySocket, dataPointer, *sizePointer, 0, reinterpret_cast<sockaddr*>(&myTargetAddress), sizeof(myTargetAddress));
		dataPointer += *sizePointer;
		++sizePointer;
	}
	myBufferStart = myBuffer;
	myPackageStart = myPackageSize;

	char buf[BUFLEN];
	int recv_len;
	int slen;
	slen = sizeof(myTargetAddress);
	while (isRunning)
	{
		recv_len = recvfrom(mySocket, buf, BUFLEN, 0, reinterpret_cast<sockaddr*>(&myTargetAddress), &slen);
		if (recv_len == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK)
			{
				break;
			}
			std::cout << "Could not recvieve data from server: " + std::to_string(error) + "\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}
		dataCount[1] += recv_len;
		if (NetworkInterface::PreProcessReceive(buf, recv_len))
		{
			memcpy(&myRecievedAddress, &myTargetAddress, sizeof(sockaddr));
			Receive(buf, recv_len);
		}
	}
}

const std::string& NetworkClient::GetName() const
{
	return myName;
}

void NetworkClient::Send(const char* someData, const int aDataSize)
{
	memcpy(myBufferStart, someData, aDataSize);
	*myPackageStart = aDataSize;
	myBufferStart += *myPackageStart;
	++myPackageStart;
}

std::variant<int, std::string> NetworkClient::SelectServer()
{
	std::string selection;
	std::variant<int, std::string> outp;
	do
	{
		std::cout << "Enter address or index: ";
		std::cin >> selection;
		size_t invpos = selection.find_first_of("0123456789.");
		if (invpos != std::string::npos)
		{
			std::cout << "address contains invalid character: [" + std::string(1, selection[invpos]) + "]\n";
			continue;
		}
		size_t dotPos = selection.find('.');
		if (dotPos == std::string::npos)
		{
			int selected = std::stoi(selection);
			std::lock_guard lock(myServerMutex);

			if (selected > 0 && selected < myServers.size())
			{
				outp = selected;
				break;
			}
			else
			{
				std::cout << "[" + std::to_string(selected) + "] is not a valid server index\n";
			}
		}
		else
		{
			outp = selection;
			break;
		}

	} while (true);
	return outp;
}

void NetworkClient::Receive(char* someData, const int aDataSize)
{
	if (myIsHandshaking && HandShakeAttempt(someData, aDataSize))
	{
	}
	else
	{
		NetMessage* netMess = reinterpret_cast<NetMessage*>(someData);
		if (netMess->myType == NetMessage::Type::Setup)
		{
			SetupMessage* setupMessage = reinterpret_cast<SetupMessage*>(someData);
			std::cout << "Connection invalidated!\n";
			std::cout << "Reconnecting\n";
			HandShake();
		}
		else if (myCallback)
		{
			myCallback(*netMess);
		}
	}
}