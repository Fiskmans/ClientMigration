#pragma once
#include "NetMessage.h"
#include <string>
#define MAXIDENTIFIERLENGTH 128

class SetupMessage :
	public NetMessage
{
public:
	SetupMessage();
	enum class SetupStep : char
	{
		Invalid,
		Request,
		Response
	};

	SetupStep myStep;
	union
	{
		char myIdentifier[MAXIDENTIFIERLENGTH];
		struct
		{
			bool myResult;
			unsigned short myID;
		} myResponse;
	};

	void SetIdentifier(const std::string& aIdentifier);
};

