#include <pch.h>
#include "ServerMain.h"
#include <string>
#include <SetupMessage.h>
#include <StatusMessage.h>
#include <thread>
#include <TimeHelper.h>
#include <Random.h>

RedirectServer::RedirectServer()
{
}


RedirectServer::~RedirectServer()
{
}

#define PORT "5763"
#define CLIENTPORT 5764



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

void RedirectServer::StartServer()
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
	int iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return;
	}

	std::cout << "Got address info!\n";

	int broadcast = 1;
	setsockopt(mySocket, SOL_SOCKET, SO_BROADCAST, (const char*)&broadcast, sizeof broadcast);
	//Bind
	if (bind(mySocket, result->ai_addr, (int)result->ai_addrlen))
	{
		std::cout << "Bind failed with error code : " + std::to_string(WSAGetLastError()) + "\n";
		return;
	}
		
	std::cout << "Bind Successfull\n";

	u_long mode = 1;  // 1 to enable non-blocking socket
	ioctlsocket(mySocket, FIONBIO, &mode);

	Connect();

	Listen();

	closesocket(mySocket);
	WSACleanup();

	return;
}


void RedirectServer::HandShake()
{
}

void RedirectServer::Connect()
{
}

void RedirectServer::Listen()
{
	sockaddr_in si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	slen = sizeof(si_other);
	while (true)
	{
		for (auto& cli : myClients)
		{
			if (!cli.second.IsAlive())
			{
				StatusMessage message;
				message.myStatus = StatusMessage::Status::UserDisconnected;
				message.myID = cli.second.GetID();
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
		auto it = myClients.find(key);
		if (it == myClients.end())
		{
			static unsigned short IDCOUNTER = 3;
			myClients[key] = Connection(si_other, slen, mySocket, ++IDCOUNTER, std::bind(&RedirectServer::TransmitMessage, this, std::placeholders::_1),std::bind(&RedirectServer::RequestList,this,std::placeholders::_1));
			it = myClients.find(key);
		}

		if (it->second.PreProcessReceive(buf, recv_len))
		{
			it->second.Receive(buf, recv_len);
		}
	}
}

void RedirectServer::TransmitMessage(const NetMessage& aMessage)
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
			std::cout << "Telling all clients that [" << status.myUsername << "] has connected on id [" + std::to_string(status.myID) + "]\n";
			for (auto& it : myClients)
			{
				if (it.second.GetID() == status.myID)
				{
					for (auto& cli : myClients)
					{
						if (cli.second.GetID() != it.second.GetID())
						{
							StatusMessage userStatus;
							userStatus.SetName(cli.second.GetName());
							userStatus.myStatus = StatusMessage::Status::UserOnline;
							userStatus.myID = cli.second.GetID();
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
				if (i.second.GetID() == status.myID)
				{
					std::cout << i.second.GetName();
				}
			}
			std::cout << "] has disconnected.\n";
			for (auto& it : myClients)
			{
				if (it.second.GetID() != status.myID)
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

std::vector<Connection*> RedirectServer::RequestList(const Connection& aRequester)
{
	std::vector<Connection*> out;
	for (auto& i : myClients)
	{
		out.push_back(&i.second);
	}

	return out;
}
