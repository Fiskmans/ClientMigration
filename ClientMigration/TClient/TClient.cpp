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
#include <NetworkHelpers.h>
#include <NetIdentify.h>
#include <PingMessage.h>

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
	for (size_t i = 0; i < 3; i++)
	{
		Tools::GetTotalTime();
	}
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

	u_long mode = 1;  // 1 to enable non-blocking socket
	ioctlsocket(mySocket, FIONBIO, &mode);

	sockaddr_in sockAddr;

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(PORT);
	sockAddr.sin_addr.S_un.S_addr = INADDR_ANY; // use default

	std::string address;
	std::cout << "Enter Adress: ";
	//std::getline(std::cin, address);
	address = "server.mansandersen.com"; // "127.0.0.1"; // 
	std::cout << "\n";
	if (address.empty())
	{
		address = "127.0.0.1";
	}
	ReplaceLastLine("Enter Adress: " + address);

	using namespace std::string_literals;
	TranslateAddress(address, (sockaddr*)&myTargetAddress, false, [](std::string message, bool isError) { std::cout << "["s + (isError ? "Error" : "Message") + "]: " + message + "\n"; });
	myTargetAddress.sin_port = htons(SERVERPORT);

	char addBuf[INET6_ADDRSTRLEN];
	if (inet_ntop(myTargetAddress.sin_family, &myTargetAddress.sin_addr, addBuf, INET6_ADDRSTRLEN) != NULL)
	{
		std::cout << "Final address: "s + addBuf + std::string(16 - strlen(addBuf), ' ') + "\n";
	}
	else
	{
		std::cout << "Not a valid target address. \n";
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
	std::cout << "\n";
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

	std::cout << "Starting Handshake " + ReadableAddress((sockaddr*)&myTargetAddress) + "\n";
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
	std::cout << "Identifying self\n";
	NetIdentify identify;
	identify.myProcessType = NetIdentify::IdentificationType::IsClient;
	identify.myIsClient.myUsername = myName;
	Send(identify);
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
	message.myAssignedID = myId;
	message.myStatus = StatusMessage::Status::UserDisconnected;
	int slen = sizeof(myTargetAddress);
	bool done = false;
	NetworkInterface::HookCallBack(message.myNetMessageID, [&](NetMessageIdType id) { done = true; });
	PreProcessAndSend(&message,sizeof(message));

	while (!done)
	{
		Flush();
		std::this_thread::yield();
	}
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

void NetworkClient::Send(const char* someData, const int aDataSize, sockaddr* aCustomAddress)
{
	if (aCustomAddress)
	{
		sendto(mySocket, someData, aDataSize,0, aCustomAddress, sizeof(*aCustomAddress));
		std::cout << "Client sent package to custom address: " + ReadableAddress(aCustomAddress) + "\n";
	}
	else
	{
		memcpy(myBufferStart, someData, aDataSize);
		*myPackageStart = aDataSize;
		myBufferStart += *myPackageStart;
		++myPackageStart;
	}
}

std::array<char, sizeof(sockaddr)> AddrToArray(sockaddr aAddress)
{
	return *reinterpret_cast<std::array<char, sizeof(sockaddr)>*>(&aAddress);
}

void NetworkClient::EvaluateServer(sockaddr aAddress)
{
	std::cout << "Evaluating server with ip: " + ReadableAddress(&aAddress) + "\n";

	PingMessage message;
	float now = Tools::GetTotalTime();
	NetworkInterface::HookCallBack(message.myNetMessageID, [this, aAddress,now](NetMessageIdType) 
		{ 
			auto key = AddrToArray(aAddress);
			float ping = Tools::GetTotalTime() - now;
			myServerEval[key] = ping;
			std::cout << "Server pinged in " + std::to_string(ping) + " seconds\n";
			StatusMessage response;
			response.myStatus = StatusMessage::Status::EvaluatedServer;
			response.myAddress = aAddress;
			response.myEvalServer.aPing = ping;
			Send(response);
		});
	Send(message,&aAddress);
}

void NetworkClient::Receive(char* someData, const int aDataSize)
{
	if (myIsHandshaking && HandShakeAttempt(someData, aDataSize))
	{
	}
	else
	{
		NetMessage* netMess = reinterpret_cast<NetMessage*>(someData);

		if (netMess->myType == NetMessage::Type::Status)
		{
			StatusMessage* status = reinterpret_cast<StatusMessage*>(netMess);
			if (status->myStatus == StatusMessage::Status::PotentialServer)
			{
				EvaluateServer(status->myAddress);
			}
			if (status->myStatus == StatusMessage::Status::ConnectToServer)
			{
				std::cout << "Redirecting to: " + ReadableAddress(&status->myAddress) + "\n";

				memcpy(&myTargetAddress, &status->myAddress, sizeof(sockaddr));
				HandShake();
			}
		}
		if (netMess->myType == NetMessage::Type::Setup)
		{
			SetupMessage* setupMessage = reinterpret_cast<SetupMessage*>(someData);
			std::cout << "Connection invalidated!\n";
			std::cout << "Reconnecting\n";
			HandShake();
		}
		else if (netMess->myType == NetMessage::Type::Identify)
		{
			NetIdentify* ident = reinterpret_cast<NetIdentify*>(netMess);
			switch (ident->myProcessType)
			{
			case NetIdentify::IdentificationType::IsServer:
				std::cout << "connected to server\n";
				break;
			case NetIdentify::IdentificationType::IsHost:
				std::cout << "connected to host\n";
				break;
			default:
				std::cout << "unexpected server type\n";
				break;
			}
		}
		else if (myCallback)
		{
			myCallback(*netMess);
		}
	}
}