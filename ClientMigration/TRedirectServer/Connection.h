#pragma once
#include <CommonNetworkIncludes.h>
#include <functional>
#include <NetworkInterface.h>

class MoveMessage;
class StatusMessage;

typedef std::function<void(const NetMessage&)> SendCallback;

class Connection : public NetworkInterface
{
	typedef std::function<std::vector<Connection*> (const Connection&)> ConnectionRequestFunction;
public:
	Connection();
	Connection(sockaddr_in aAddress, int aAddressSize, SOCKET aSocket, unsigned short aID, SendCallback aCallbackFunction, ConnectionRequestFunction aRequestFunction);

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

	void SendServerStatus();

	bool myIsServer;

	bool myIsValid;
	char myConnectedUser[MAXUSERNAMELENGTH];
	unsigned short myID;

	SOCKET mySocket;

	float myPosition[2];

	sockaddr_in myAddress;
	int myAddressSize;
	SendCallback myCallbackFunction;
	ConnectionRequestFunction myConnectionListRequestFunction;

	// Inherited via NetworkInterface
	virtual void TimedOut() override;
};

template<class T>
inline void Connection::Send(const T& aData)
{
	PreProcessAndSend(&aData,sizeof(T));
}
