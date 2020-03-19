#pragma once
#include "NetMessage.h"
#include <string>

class StatusMessage : public NetMessage
{
public:
	StatusMessage();
	enum class Status : char
	{
		UserConnected,
		UserOnline,
		UserDisconnected,
		PotentialServer
	} myStatus;
	union
	{
		char myUsername[128];
		struct PotentialServer
		{
			sockaddr aAddress;
			char aAddressSize;
		} myServer;
	};

	void SetName(const std::string& aName);

};

