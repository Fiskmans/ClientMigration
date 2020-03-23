#include <pch.h>
#include "NetMessage.h"
#include "StatusMessage.h"

NetMessageIdType NetMessage::ourCounter = 100;

bool NetMessage::IsImportant() const
{
	switch (myType)
	{
	case NetMessage::Type::Status:
	case NetMessage::Type::Setup:
	case NetMessage::Type::BigFile:
	case NetMessage::Type::Identify:
	case NetMessage::Type::Ping:
		return true;
	default:
		return false;
	}
}
