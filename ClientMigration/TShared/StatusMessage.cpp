#include "stdafx.h"
#include "StatusMessage.h"

void StatusMessage::SetName(const std::string& aName)
{
	strcpy_s<128>(myUsername, aName.c_str());
}
StatusMessage::StatusMessage()
{
	myType = Type::Status;
}