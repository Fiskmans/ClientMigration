#pragma once
#include <CommonNetworkIncludes.h>
#include <functional>
#include <NetworkInterface.h>

class MoveMessage;
class StatusMessage;

class Connection : public NetworkInterface
{
public:
	Connection();
	Connection(sockaddr_in aAddress, int aAddressSize, SOCKET aSocket, unsigned short aID, std::function<void(const NetMessage&)> aCallbackFunction);

	bool IsAlive();

	template<class T>
	void Send(const T& aData);
	void Send(const char* aData,int aDataSize) override;
	void Receive(char* someData, const int aDataSize) override;
	void Invalidate();

	unsigned short GetID();
	std::string GetName();
private:
	bool Evaluate(MoveMessage* aMessage);

	bool HandShake(char* aData, int aAmount);
	void Parse(char* aData, int aAmount);


	bool myIsValid;
	char myConnectedUser[MAXUSERNAMELENGTH];
	unsigned short myID;

	SOCKET mySocket;

	float myPosition[2];

	sockaddr_in myAddress;
	int myAddressSize;
	std::function<void(const NetMessage&)> myCallbackFunction;
};

template<class T>
inline void Connection::Send(const T& aData)
{
	PreProcessAndSend(&aData,sizeof(T));
}
