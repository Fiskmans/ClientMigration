#pragma once
#include <CommonNetworkIncludes.h>
#include <unordered_map>
#include "Connection.h"
#include <NetworkInterface.h>

class CServerMain
{
public:
	CServerMain();
	~CServerMain();
	void StartServer();


private:


	void Listen();
	void TransmitMessage(const NetMessage& aMessage);

	char myUpdateDistance = 0;

	SOCKET mySocket;
	std::unordered_map<std::string, Connection> myClients;
};

