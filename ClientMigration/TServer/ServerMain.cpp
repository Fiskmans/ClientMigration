#include "stdafx.h"
#include "ServerMain.h"
#include <string>
#include <SetupMessage.h>
#include <StatusMessage.h>
#include <MoveMessage.h>
#include <thread>
#include <TimeHelper.h>
#include <Random.h>

CServerMain::CServerMain()
{
}


CServerMain::~CServerMain()
{
}

#define PORT "5763" //The port on which to listen for incoming data
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

	std::string broadcastIP;
	char addbuf[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, reinterpret_cast<in_addr*>(result->ai_addr),addbuf, INET_ADDRSTRLEN);
	if (CalculateBroadcastIP("10.155.92.1","255.255.252.0",broadcastIP) == S_OK)
	{
		std::cout << "broadcastip is: " + broadcastIP + "\n";
		if (inet_pton(AF_INET, broadcastIP.c_str(), &myBroadcastAddress.sin_addr) != 1)
		{
			std::cout << "failed to generate bradcast address\n";
		}
		myBroadcastAddress.sin_port = htons(CLIENTPORT);
	}
	else
	{
		std::cout << "failed to generate broadcast ip\n";
	}
	
	std::cout << "Bind Successfull\n";

	u_long mode = 1;  // 1 to enable non-blocking socket
	ioctlsocket(mySocket, FIONBIO, &mode);

	Listen();

	closesocket(mySocket);
	WSACleanup();

	return;
}

void CServerMain::Step()
{
	static float lastBall = 0.f;
	static float lastTick = Tools::GetTotalTime();
	static float lastMediumRange = Tools::GetTotalTime();
	static float lastFullUpdate = Tools::GetTotalTime();
	static float lastBroadcast = Tools::GetTotalTime();
	float now = Tools::GetTotalTime();
	float dt = now - lastTick;



	if (dt < 0.016f)
	{
		return;
	}

	if (now - lastBroadcast > 1.f)
	{
		StatusMessage mess;
		mess.myStatus = StatusMessage::Status::ServerExists;
		TransmitMessage(mess);
		lastBroadcast = now;
	}
	return;

	myUpdateDistance = 0;
	if (now - lastMediumRange > 0.08f)
	{
		myUpdateDistance = 1;
		lastMediumRange = now;
	}
	if (now - lastFullUpdate > 0.3f)
	{
		myUpdateDistance = 100;
		lastFullUpdate = now;
	}


	lastTick = now;
	if (myBalls.size() < 10 && now - lastBall > 1.f)
	{
		lastBall = now;
		ServerBall ball;
		V3F dir = Tools::RandomDirection();
		dir.z = 0;
		dir.Normalize();
		ball.aDirection = { dir.x,dir.y };
		ball.myPosition = { Tools::RandomNormalized(),Tools::RandomNormalized() };
		myBalls.push_back(ball);

		StatusMessage message;
		message.myStatus = StatusMessage::Status::BallCreated;
		message.myObjectID = ball.GetID();
		message.myVector = ball.myPosition;
		TransmitMessage(message);

		std::cout << "Created Ball\n";
	}
	for (auto& i : myBalls)
	{
		i.myPosition += i.aDirection * dt * 0.2f;
		if (i.myPosition.x < 0.f)
		{
			i.aDirection.x = abs(i.aDirection.x);
		}
		if (i.myPosition.y < 0.f)
		{
			i.aDirection.y = abs(i.aDirection.y);
		}
		if (i.myPosition.x > 1.f)
		{
			i.aDirection.x = -abs(i.aDirection.x);
		}
		if (i.myPosition.y > 1.f)
		{
			i.aDirection.y = -abs(i.aDirection.y);
		}



		StatusMessage message;
		message.myStatus = StatusMessage::Status::BallMoved;
		message.myObjectID = i.GetID();
		message.myVector = i.myPosition;
		TransmitMessage(message);
	}

	ScratchBalls();
}

void CServerMain::ScratchBalls()
{
	for (int i = int(myBalls.size()) - 1; i >= 0; i--)
	{
		for (size_t j = i + 1; j < myBalls.size(); j++)
		{
			Tga2D::Vector2f delta = myBalls[i].myPosition - myBalls[j].myPosition;
			if (sqrt(delta.x * delta.x + delta.y * delta.y) < 0.03f)
			{
				size_t max = max(i, j);
				size_t min = min(i, j);
				RemoveBall(max(i, j));
				RemoveBall(min(i, j));
			}
		}
	}
}

void CServerMain::RemoveBall(size_t aIndex)
{
	StatusMessage message;
	message.myStatus = StatusMessage::Status::BallDestroyed;
	message.myObjectID = myBalls[aIndex].GetID();

	myBalls.erase(myBalls.begin() + aIndex);
	TransmitMessage(message);
}

void CServerMain::Listen()
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
		Step();

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
	case NetMessage::Type::Chat:
	{
		const ChatMessage& chat = *reinterpret_cast<const ChatMessage*>(&aMessage);
		if (chat.myTarget == 0)
		{
			std::cout << "Sending [" << chat.myMessage << "] to all clients\n";
			for (auto& it : myClients)
			{
				it.second.Send(chat);
			}
		}
		else
		{
			for (auto& it : myClients)
			{
				if (it.second.GetID() == chat.myTarget)
				{
					std::cout << "Sending [" << chat.myMessage << "] to " + it.second.GetName() + "\n";
					it.second.Send(chat);
					return;
				}
			}
			std::cout << "message [" << chat.myMessage << "] could not be sent as there is no user with id: " + std::to_string(chat.myTarget) + " is online\n";
		}
	}
	break;
	case NetMessage::Type::Move:
	{
		const MoveMessage* move = reinterpret_cast<const MoveMessage*>(&aMessage);
		for (auto& it : myClients)
		{
			if (it.second.GetID() != move->myTarget)
			{
				it.second.Send(*move);
			}
		}
	}
	break;
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
		case StatusMessage::Status::BallCreated:
		case StatusMessage::Status::BallDestroyed:
		{
			for (auto& cli : myClients)
			{
				cli.second.Send(status);
			}
		}
		break;
		case StatusMessage::Status::BallMoved:
		{
			for (auto& cli : myClients)
			{
				if (cli.second.Distance(&status) <= myUpdateDistance)
				{
					cli.second.Send(status);
				}
			}
		}
		break;
		case StatusMessage::Status::ServerExists:
			std::cout << "Broadcasted\n";
			sendto(mySocket, reinterpret_cast<const char*>(&aMessage), sizeof(StatusMessage), 0, reinterpret_cast<sockaddr*>(&myBroadcastAddress),sizeof(myBroadcastAddress));
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