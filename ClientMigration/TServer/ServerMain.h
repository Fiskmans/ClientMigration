#pragma once
#include <CommonNetworkIncludes.h>
#include <unordered_map>
#include "Connection.h"
#include <array>
#include <queue>
#include <ChatMessage.h>
#include <NetworkInterface.h>
#include "ServerBall.h"

class CServerMain
{
public:
	CServerMain();
	~CServerMain();
	void StartServer();

	void Step();

private:

	void ScratchBalls();
	void RemoveBall(size_t aIndex);

	void Listen();
	void TransmitMessage(const NetMessage& aMessage);

	char myUpdateDistance = 0;

	SOCKET mySocket;
	sockaddr_in myBroadcastAddress;
	std::unordered_map<std::string, Connection> myClients;
	std::vector<ServerBall> myBalls;
};

