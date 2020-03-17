#pragma once
#include <CommonNetworkIncludes.h>
#include <unordered_map>
#include "Connection.h"
#include <NetworkInterface.h>

class RedirectServer
{
public:
	RedirectServer();
	~RedirectServer();
	void StartServer();


private:

	void Connect();
	void Listen();
	void TransmitMessage(const NetMessage& aMessage);

	char myUpdateDistance = 0;

	sockaddr myUpstreamAddress;
	SOCKET myUpstreamSocket;

	SOCKET mySocket;
	std::unordered_map<std::string, Connection> myClients;
};

