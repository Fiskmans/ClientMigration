#pragma once
#include "NetMessage.h"
#include <string>
#include "../DX2DEngine/tga2d/math/vector2imp.h"

class StatusMessage : public NetMessage
{
public:
	StatusMessage();
	enum class Status : char
	{
		UserConnected,
		UserOnline,
		UserDisconnected,
		ServerExists,
		ServerClosing,
		BallCreated,
		BallDestroyed,
		BallMoved
	} myStatus;
	union
	{
		struct
		{
			short myObjectID;
			Tga2D::Vector2f myVector;
		};
		char myUsername[128];

	};

	void SetName(const std::string& aName);

};

