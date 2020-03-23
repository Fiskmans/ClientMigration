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
		PotentialServer,
		EvaluatedServer,
		ConnectToServer
	} myStatus;

	sockaddr myAddress;
	union
	{
		char myUsername[128];
		struct PotentialServer
		{
			char aAddressSize;
		} myServer;
		struct EvaluatedServer
		{
			float aPing;
		} myEvalServer;
	};
	unsigned short myAssignedID;
	void SetName(const std::string& aName);

};

