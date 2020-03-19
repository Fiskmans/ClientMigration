#pragma once
#include <CommonNetworkIncludes.h>
#include <unordered_map>
#include "Connection.h"
#include <NetworkInterface.h>

class CServerMain : public NetworkInterface
{
public:
	CServerMain();
	~CServerMain();
	void StartServer();

	virtual void Receive(char* someData, const int aDataSize) override final;
	virtual void Send(const char* someData, const int aDataSize) override final;
	virtual void TimedOut() override final;

private:

	void HandShake();
	bool HandShakeAttempt(char* aData, int aDataSize);


	void Connect();
	void Listen();
	void TransmitMessage(const NetMessage& aMessage);

	void Flush();


	template<class T>
	void SendUpstream(const T& aMessage);

	unsigned short myUpstreamID;

	bool myIsHandshaking = false;
	bool myIsConnected = false;
	char myUpdateDistance = 0;

	sockaddr myUpstreamAddress;
	SOCKET myUpstreamSocket;

	SOCKET mySocket;
	std::unordered_map<std::string, Connection> myClients;
};

template<class T>
inline void CServerMain::SendUpstream(const T& aMessage)
{
	NetworkInterface::PreProcessAndSend(&aMessage, sizeof(aMessage));
}
