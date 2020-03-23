#pragma once
#include <CommonNetworkIncludes.h>
#include <functional>
#include <NetworkInterface.h>

class Connection;

struct Evaluation
{
	Connection* myConnection;
	float myPing;
	bool myHasResult = false;
};

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
	void Send(const char* aData,int aDataSize, sockaddr* aCustomAddress = nullptr) override;
	void Receive(char* someData, const int aDataSize) override;
	void Invalidate();

	void Flush();

	unsigned short GetID();
	std::string GetName();


private:
	bool Evaluate(MoveMessage* aMessage);

	float myLastUpdate;

	bool HandShake(char* aData, int aAmount);
	void Parse(char* aData, int aAmount);

	void BeginRedirect();

	void EvaluateRedirectResult(bool aIsTimeOutEval);
	bool myIsRedirecting;
	int myPotentialRedirectCount;
	float myRedirectStart;
	std::map<std::array<char, sizeof(sockaddr)>, Evaluation> myEvaluations;
	bool myIsServer;
	sockaddr_in myServerAddress;

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
