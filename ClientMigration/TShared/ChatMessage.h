#pragma once
#include "NetMessage.h"
#include <string>
#define MAXMESASGELEGNTHE 256
class ChatMessage : public NetMessage
{
public:
	ChatMessage();

	unsigned short myTarget;
	unsigned short mySender;
	void SetMessage(const std::string& aString);
	char myMessage[MAXMESASGELEGNTHE];
};

