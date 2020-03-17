#pragma once

#define MAXPACKAGESIZE 576

typedef unsigned int NetMessageIdType;

class NetMessage
{
public:
	enum class Type : short
	{
		Invalid,
		Setup,
		Status,
		PackageResponse,
		BigFile,
		Identify
	};
	NetMessage();
	Type  myType;
	NetMessageIdType myID;
	bool IsImportant() const;
	char* begin();

	static NetMessageIdType ourCounter;
};

class NetMessageImportantResponse : public NetMessage
{
public:
	size_t myMessageHash;
};

inline NetMessage::NetMessage()
{
	myID = ourCounter++;
	myType = Type::Invalid;
}

inline char* NetMessage::begin()
{
	return reinterpret_cast<char*>(&myType);
}
