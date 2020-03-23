#include <pch.h>
#include "ServerMain.h"
#include <string>
#include <SetupMessage.h>
#include <StatusMessage.h>
#include <thread>
#include <TimeHelper.h>
#include <Random.h>
#include <NetworkHelpers.h>
#include <NetIdentify.h>

CServerMain::CServerMain()
{
}


CServerMain::~CServerMain()
{
}

#define CLIENTPORT 5764
#define GAMESERVERPORT 5765
#define SOURCEPORT 5763

#define PORTRANGEBEGIN 5766
#define PORTRANGEEND 5801



int CalculateBroadcastIP(std::string aIP, std::string aSubNet, std::string& aBroadcast)
{
	struct in_addr host, mask, broadcast;
	char broadcast_address[INET_ADDRSTRLEN];
	if (inet_pton(AF_INET, aIP.c_str(), &host) == 1 &&
		inet_pton(AF_INET, aSubNet.c_str(), &mask) == 1)
	{
		broadcast.S_un.S_addr = host.S_un.S_addr | ~mask.S_un.S_addr;
	}
	else
	{
		return 1;
	}
	if (inet_ntop(AF_INET,&broadcast,broadcast_address,INET_ADDRSTRLEN) != NULL)
	{
		std::cout << "Broadcast address of " + aIP + " with netmask " + aSubNet + " is " + broadcast_address + "\n";
	}
	else
	{
		return 1;
	}

	aBroadcast = broadcast_address;
	return S_OK;
}

void CServerMain::StartServer()
{
	struct sockaddr_in si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	slen = sizeof(si_other);

	//Initialise winsock
	std::cout << "Initialising Winsock...\n";
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "Failed.Error Code: " + std::to_string(WSAGetLastError()) + "\n";
		return;
	}
	std::cout << "Initialised.\n";

	//Create a socket
	mySocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mySocket == INVALID_SOCKET)
	{
		std::cout << "Could not create socket: " + std::to_string(WSAGetLastError()) + "\n";
		return;
	}
	std::cout << "Socket created.\n";

	struct addrinfo* result = NULL, * ptr = NULL, hints;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	int iResult = getaddrinfo(NULL, STRINGVALUE(GAMESERVERPORT), &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return;
	}

	std::cout << "Got address info!\n";

	//Bind
	sockaddr bindAddress = *result->ai_addr;
	bool bound = false;
	for (size_t i = PORTRANGEBEGIN; i < PORTRANGEEND; i++)
	{
		((sockaddr_in*)&bindAddress)->sin_port = i;
		if (bind(mySocket, &bindAddress, sizeof(bindAddress)))
		{
			std::cout << "Failed to bind to: " + std::to_string(i) + " trying next in range\n";
		}
		else
		{
			bound = true;
			break;
		}
	}
	if (!bound)
	{
		std::cout << "Failed to bind on all ports:\n";
		return;
	}

	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(mySocket, (struct sockaddr*) & sin, &len) != SOCKET_ERROR)
	{
		std::cout << "Bind Successfull on ip: " + ReadableAddress((sockaddr*)&sin) + "\n";
	}
	else
	{
		std::cout << "could not retrieve address from bound socket but got no error on bind.\n";
	}

	u_long mode = 1;  // 1 to enable non-blocking socket
	ioctlsocket(mySocket, FIONBIO, &mode);

	Connect();

	Listen();

	closesocket(mySocket);
	WSACleanup();

	return;
}

void CServerMain::Receive(char* someData, const int aDataSize)
{

	if (myIsHandshaking && HandShakeAttempt(someData, aDataSize))
	{
	}
	else
	{
		NetMessage* message = reinterpret_cast<NetMessage*>(someData);
		if (message->myType == NetMessage::Type::Ping)
		{
			std::cout << "I was pinged\n";
		}
		//Normal stuff
	}
}

void CServerMain::Send(const char* someData, const int aDataSize,sockaddr* aCustomAddress)
{
	if (aCustomAddress)
	{
		sendto(myUpstreamSocket, someData, aDataSize, 0, aCustomAddress, sizeof(*aCustomAddress));
	}
	else
	{
		sendto(myUpstreamSocket, someData, aDataSize, 0, &myUpstreamAddress, sizeof(myUpstreamAddress));
	}
}

void CServerMain::TimedOut()
{
	if (myIsConnected)
	{
		std::cout << "Upstream Timed out\n";
		Connect();
	}
}


void CServerMain::HandShake()
{
	char buf[BUFLEN];
	int recv_len;
	int slen;
	slen = sizeof(myUpstreamAddress);
	myIsConnected = false;

	SetupMessage message;
	DWORD length = MAXIDENTIFIERLENGTH;
	GetUserNameA(message.myIdentifier, &length);
	message.myStep = SetupMessage::SetupStep::Request;
	std::string username = "GameServer " + std::to_string(GetCurrentProcessId());

	strcpy_s<MAXIDENTIFIERLENGTH>(message.myIdentifier, username.c_str());

	SetWindowTextA(GetConsoleWindow(), message.myIdentifier);

	std::cout << "Starting Handshake\n";
	while (!myIsConnected)
	{
		std::cout << "Sending Handshake package\n";
		SendUpstream(message);
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
	identify.myProcessType = NetIdentify::IdentificationType::IsServer;
	identify.myIsServer.myName = "GameServer " + std::to_string(GetCurrentProcessId());
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(mySocket, (struct sockaddr*) & sin, &len) != SOCKET_ERROR)
	{
		identify.myIsServer.myPort = sin.sin_port;
		std::cout << "Identifying on port: " + std::to_string(sin.sin_port) + "\n";
	}
	SendUpstream(identify);
}

bool CServerMain::HandShakeAttempt(char* aData, int aDataSize)
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
			myUpstreamID = setupMess->myResponse.myID;


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
void CServerMain::Connect()
{

	//Create a socket
	myUpstreamSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (myUpstreamSocket == INVALID_SOCKET)
	{
		std::cout << "Could not create socket: " + std::to_string(WSAGetLastError()) + "\n";
		return;
	}
	std::cout << "Socket created.\n";


	memset(&myUpstreamAddress, 0, sizeof(myUpstreamAddress));

	u_long mode = 1;  // 1 to enable non-blocking socket
	ioctlsocket(myUpstreamSocket, FIONBIO, &mode);

	sockaddr_in sockAddr;

	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(5763);
	sockAddr.sin_addr.S_un.S_addr = INADDR_ANY; // use default

	using namespace std::string_literals;
	TranslateAddress("server.mansandersen.com", (sockaddr*)&myUpstreamAddress, false, [](std::string message, bool isError) { std::cout << "["s + (isError ? "Error" : "Message") + "]: " + message + "\n"; });
	((sockaddr_in*)&myUpstreamAddress)->sin_port = htons(SOURCEPORT);


	char addBuf[INET6_ADDRSTRLEN];
	if (inet_ntop(((sockaddr_in*)&myUpstreamAddress)->sin_family, &((sockaddr_in*)&myUpstreamAddress)->sin_addr, addBuf, INET6_ADDRSTRLEN) != NULL)
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

void CServerMain::Listen()
{
	sockaddr_in si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	slen = sizeof(si_other);

	sockaddr sin;
	int len = sizeof(sin);
	if (getsockname(mySocket,& sin, &len) != SOCKET_ERROR)
	{
		std::cout << "Listening on: " + ReadableAddress(&sin) + "\n";
	}
	else
	{
		std::cout << "Could not get address info on listener\n";
	}

	while (true)
	{
		Flush();

		for (auto& cli : myClients)
		{
			if (!cli.second.IsAlive())
			{
				StatusMessage message;
				message.myStatus = StatusMessage::Status::UserDisconnected;
				message.myAssignedID = cli.second.GetID();
				TransmitMessage(message);
				myClients.erase(cli.first);
				break;
			}
			else
			{
				cli.second.Flush();
			}
		}

		recv_len = recvfrom(mySocket, buf, BUFLEN, 0, reinterpret_cast<sockaddr*>(&si_other), &slen);
		
		char addressBuffer[512];
		InetNtopA(AF_INET, &si_other, addressBuffer, 512);
		std::string key = addressBuffer;
		if (recv_len == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error == WSAEWOULDBLOCK)
			{
				std::this_thread::yield();
				continue;
			}
			std::cout << "Could not recvieve data from client(s): " + std::to_string(WSAGetLastError()) + " ip: " + key + "\n";
			myClients[key].Invalidate();
			continue;
		}

		NetMessage* message = reinterpret_cast<NetMessage*>(buf);
		if (message->myType == NetMessage::Type::Ping)
		{
			std::cout << "I Was pinged\n";
		}

		std::cout << "here?\n";

		auto it = myClients.find(key);
		if (it == myClients.end())
		{
			static unsigned short IDCOUNTER = 3;
			myClients[key] = Connection(si_other, slen, mySocket, ++IDCOUNTER, std::bind(&CServerMain::TransmitMessage, this, std::placeholders::_1));
			it = myClients.find(key);
		}

		if (it->second.PreProcessReceive(buf, recv_len))
		{
			it->second.Receive(buf, recv_len);
		}
	}
}

void CServerMain::TransmitMessage(const NetMessage& aMessage)
{
	switch (aMessage.myType)
	{
	case NetMessage::Type::Status:
	{
		const StatusMessage& status = *reinterpret_cast<const StatusMessage*>(&aMessage);
		switch (status.myStatus)
		{
		case StatusMessage::Status::UserConnected:
		{
			std::cout << "Telling all clients that [" << status.myUsername << "] has connected on id [" + std::to_string(status.myAssignedID) + "]\n";
			for (auto& it : myClients)
			{
				if (it.second.GetID() == status.myAssignedID)
				{
					for (auto& cli : myClients)
					{
						if (cli.second.GetID() != it.second.GetID())
						{
							StatusMessage userStatus;
							userStatus.SetName(cli.second.GetName());
							userStatus.myStatus = StatusMessage::Status::UserOnline;
							userStatus.myAssignedID = cli.second.GetID();
							it.second.Send(userStatus);
						}
					}
				}
				else
				{
					it.second.Send(status);
				}
			}
		}
		break;
		case StatusMessage::Status::UserDisconnected:

		{
			std::cout << "Telling all clients that [";
			for (auto& i : myClients)
			{
				if (i.second.GetID() == status.myAssignedID)
				{
					std::cout << i.second.GetName();
				}
			}
			std::cout << "] has disconnected.\n";
			for (auto& it : myClients)
			{
				if (it.second.GetID() != status.myAssignedID)
				{
					it.second.Send(status);
				}
			}
		}
		break;
		case StatusMessage::Status::UserOnline:
		default:
			std::cout << "This should never happen\n";
			break;
		}
	}
	break;
	default:
		break;
	}

}

void CServerMain::Flush()
{
	NetworkInterface::Flush();

	char buf[BUFLEN];
	int recv_len;
	int slen;
	slen = sizeof(myUpstreamAddress);
	sockaddr addr;

	while (true)
	{
		recv_len = recvfrom(myUpstreamSocket, buf, BUFLEN, 0, &addr, &slen);
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
		if (NetworkInterface::PreProcessReceive(buf, recv_len))
		{
			Receive(buf, recv_len);
		}
	}
}
