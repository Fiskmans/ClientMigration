#pragma once
#include "NetMessage.h"
class MoveMessage : public NetMessage
{
public:
	MoveMessage();
	unsigned short myTarget;
	float myPosition[2];
};

