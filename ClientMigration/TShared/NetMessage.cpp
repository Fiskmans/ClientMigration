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
		return true;
	default:
		return false;
	}
}
