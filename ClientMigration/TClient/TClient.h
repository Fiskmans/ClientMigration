#pragma once
#include <CommonNetworkIncludes.h>
#include <thread>
#include <string>
#include <unordered_map>
#include <functional>
#include <unordered_map>
#include <NetworkInterface.h>
#include <future>
#include <variant>
#include <vector>

#define MAXBUFFERSIZE 1024 << 5
#define MAXBUFFERCOUNT 1024

class NetMessage;
class NetworkClient : NetworkInterface
{
public:
	NetworkClient();

	void Connect();
	void Disconnect();

	template<class T>
	void Send(const T& aMessage);

	void SetCallback(std::function<void(NetMessage&)> aFunction);
	void Flush();

	const std::string& GetName() const;


	void Receive(char* someData, const int aDataSize) override;
	void Send(const char* someData, const int aDataSize) override;

private:
	std::variant<int, std::string> SelectServer();
	std::vector<sockaddr_in> myServers;
	std::mutex myServerMutex;

	char myBuffer[MAXBUFFERSIZE];
	short myPackageSize[MAXBUFFERCOUNT];
	char* myBufferStart = nullptr;
	short* myPackageStart = nullptr;

	SOCKET mySocket;
	struct sockaddr_in myTargetAddress;
	sockaddr_in myRecievedAddress;

	void HandShake();
	bool HandShakeAttempt(char* aData,int aDataSize);

	std::function<void(NetMessage&)> myCallback;
	std::string myName;

	unsigned short myId;
	bool myIsHandshaking = false;
	bool myIsConnected = false;

	// Inherited via NetworkInterface
	virtual void TimedOut() override;
};

template<class T>
inline void NetworkClient::Send(const T& aMessage)
{
	NetworkInterface::PreProcessAndSend(const_cast<NetMessage*>(reinterpret_cast<const NetMessage*>(&aMessage)), sizeof(aMessage));
}
