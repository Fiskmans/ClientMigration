#include "stdafx.h"
#include "NetMessage.h"
#include "StatusMessage.h"

NetMessageIdType NetMessage::ourCounter = 100;

bool NetMessage::IsImportant() const
{
	switch (myType)
	{
	case NetMessage::Type::Status:
	{
		const StatusMessage* mess = reinterpret_cast<const StatusMessage*>(this);
		if (mess->myStatus == StatusMessage::Status::BallMoved)
		{
			return false;
		}
		return true;
	}
	break;
	case NetMessage::Type::Setup:
	case NetMessage::Type::Chat:
	case NetMessage::Type::BigFile:
		return true;
	default:
		return false;
	}
}
