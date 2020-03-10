#include "stdafx.h"
#include "ChatMessage.h"

ChatMessage::ChatMessage()
{
	myType = Type::Chat;
}

void ChatMessage::SetMessage(const std::string& aString)
{
	strcpy_s<MAXMESASGELEGNTHE>(myMessage, aString.c_str());
}
