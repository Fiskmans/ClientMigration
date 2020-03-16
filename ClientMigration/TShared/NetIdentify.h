#pragma once
#include "NetMessage.h"

#ifdef __INTELLISENSE__
#include <pch.h>
#endif 


namespace IdentificationTypes
{
	struct IsServer
	{
		EasyAssignString<128> myName;
	};

	struct IsClient
	{
		EasyAssignString<128> myUsername;
	};

	struct IsHost
	{

	};
}
class NetIdentify : public NetMessage
{
	enum class IdentificationType : char
	{
		Invalid,
		IsServer,
		IsClient,
		IsHost
	};
	union
	{
		
	};

};

