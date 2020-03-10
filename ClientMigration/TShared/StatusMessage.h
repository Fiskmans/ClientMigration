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
	} myStatus;
	union
	{
		char myUsername[128];

	};

	void SetName(const std::string& aName);

};

