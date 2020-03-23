#include <pch.h>
#include "Connection.h"
#include <SetupMessage.h>
#include <StatusMessage.h>
#include <NetIdentify.h>
#include <TimeHelper.h>
#include <NetworkHelpers.h>

#define TTL 1.5f
#define REDIRECTTIMEOUT 5.f

Connection::Connection()
{
}

Connection::Connection(sockaddr_in aAddress, int aAddressSize, SOCKET aSocket, unsigned short aID, SendCallback aCallbackFunction, ConnectionRequestFunction aRequestFunction)
{
	myCallbackFunction = aCallbackFunction;
	myConnectionListRequestFunction = aRequestFunction;
	myID = aID;
	mySocket = aSocket;
	myAddress = aAddress;
	myAddressSize = aAddressSize;
	myIsValid = false;
	myIsRedirecting = false;
}


bool Connection::IsAlive()
{
	float now = Tools::GetTotalTime();
	return myIsValid; // && (now - myLastUpdate < TTL); // TODO
}

void Connection::Send(const char* aData, int aDataSize, sockaddr* aCustomAddress)
{
	sendto(mySocket, aData, aDataSize, 0, (struct sockaddr*) & myAddress, myAddressSize);
}

void Connection::Receive(char* someData, const int aDataSize)
{
	myLastUpdate = Tools::GetTotalTime();
	if (aDataSize == 0)
	{
		std::cout << myConnectedUser << " Has exited gracefully.\n";
		myIsValid = false;
		return;
	}
	NetMessage* message = reinterpret_cast<NetMessage*>(someData);
	if (message->IsImportant())
	{
		static std::hash<std::string> hasher;
		NetMessageImportantResponse resp;
		resp.myMessageHash = hasher(std::string(someData, aDataSize));
		myCallbackFunction(resp);
	}
	if (myIsValid)
	{
		Parse(someData, aDataSize);
	}
	else
	{
		myIsValid = HandShake(someData, aDataSize);
	}
}

void Connection::Invalidate()
{
	myIsValid = false;
	NetworkInterface::Clear();
}

void Connection::Flush()
{
	NetworkInterface::Flush();
	if (myIsRedirecting)
	{
		float now = Tools::GetTotalTime();
		if (now - myRedirectStart > REDIRECTTIMEOUT)
		{
			EvaluateRedirectResult(true);
		}
	}
}



unsigned short Connection::GetID()
{
	return myID;
}

std::string Connection::GetName()
{
	return myConnectedUser;
}

bool Connection::Evaluate(MoveMessage* aMessage)
{
	return true;
}

bool Connection::HandShake(char* aData, int aAmount)
{
	if (aAmount != sizeof(SetupMessage))
	{
		std::cout << "Got invalid Setup package size: [" + std::to_string(aAmount) + "] expected [" + std::to_string(sizeof(SetupMessage)) + "]\n";
		char buf2[BUFLEN + 1];
		buf2[BUFLEN] = '\0';
		memcpy(buf2, aData, BUFLEN);
		std::cout << "Package: [" << buf2 << "]\n";
		return false;
	}
	NetMessage* message = reinterpret_cast<NetMessage*>(aData);
	if (message->myType == NetMessage::Type::Setup)
	{
		SetupMessage* setup = reinterpret_cast<SetupMessage*>(aData);
		if (setup->myStep == SetupMessage::SetupStep::Request)
		{
			memset(myConnectedUser, '\0', MAXUSERNAMELENGTH);
			memcpy(myConnectedUser, setup->myIdentifier, min(MAXUSERNAMELENGTH, MAXIDENTIFIERLENGTH));
			myConnectedUser[MAXUSERNAMELENGTH - 1] = '\0';
		}
		else
		{
			std::cout << "Expected setup request got: [" + std::to_string(static_cast<char>(setup->myStep)) + "]\n";
			SetupMessage response;
			response.myResponse.myResult = false;
			NetworkInterface::PreProcessAndSend(&response, sizeof(response));
			return false;
		}
	}
	else
	{
		std::cout << "Expected setup message got: [" + std::to_string(static_cast<short>(message->myType)) + "]\n";
		SetupMessage response;
		response.myResponse.myResult = false;
		NetworkInterface::PreProcessAndSend(&response, sizeof(response));
		return false;
	}

	SetupMessage response;
	response.myResponse.myResult = true;
	response.myResponse.myID = myID;
	response.myStep = SetupMessage::SetupStep::Response;
	NetworkInterface::PreProcessAndSend(&response, sizeof(response));

	StatusMessage logonResponse;
	logonResponse.myAssignedID = myID;
	logonResponse.SetName(myConnectedUser);
	logonResponse.myStatus = StatusMessage::Status::UserConnected;
	myCallbackFunction(logonResponse);


	std::cout << "Connection Established with user: " << myConnectedUser << " on " << ReadableAddress((sockaddr*)&myAddress) << "\n";

	return true;
}

std::array<char, sizeof(sockaddr)> AddrToArray(sockaddr aAddress)
{
	return *reinterpret_cast<std::array<char, sizeof(sockaddr)>*>(&aAddress);
}

void Connection::Parse(char* aData, int aAmount)
{
	if (aAmount == SOCKET_ERROR)
	{
		std::cout << "Could not recieve data: " + std::to_string(WSAGetLastError()) + "\n";
		return;
	}

	NetMessage* netMess = reinterpret_cast<NetMessage*>(aData);
	switch (netMess->myType)
	{
	case NetMessage::Type::Status:
	{
		StatusMessage* status = reinterpret_cast<StatusMessage*>(aData);
		switch (status->myStatus)
		{
		case StatusMessage::Status::UserDisconnected:
		{

			NetMessage temp;
			status->myNetMessageID = temp.myNetMessageID;
			myCallbackFunction(*status);
		}
			break;
		case StatusMessage::Status::EvaluatedServer:
		{
			auto key = AddrToArray(status->myAddress);
			if (myEvaluations.count(key) != 0)
			{
				myEvaluations[key].myPing = status->myEvalServer.aPing;
				myEvaluations[key].myHasResult = true;
			}
			EvaluateRedirectResult(false);
		}
			break;
		default:
		{
			std::cout << myConnectedUser << " behaved badly and was kicked. sent status that was not disconnect or evaluation\n";
			myIsValid = false;
		}
			break;
		}
	}
	break;
	case NetMessage::Type::Identify:
	{
		NetIdentify* ident = reinterpret_cast<NetIdentify*>(aData);
		switch (ident->myProcessType)
		{
		case NetIdentify::IdentificationType::IsServer:
			myServerAddress = myAddress;
			//myServerAddress.sin_port = ident->myIsServer.myPort;
			std::cout << "Server on " + ReadableAddress((sockaddr*)&myAddress) + " added server to: " + std::to_string(htons(ident->myIsServer.myPort)) + " Resulting in address: " + ReadableAddress((sockaddr*)&myServerAddress) + "\n";
			myIsServer = true;
			break;
		case NetIdentify::IdentificationType::IsClient:
			std::cout << "Client Connected\n";
			myIsServer = false;
			NetIdentify response;
			response.myProcessType = NetIdentify::IdentificationType::IsServer;
			NetworkInterface::HookCallBack(response.myNetMessageID, std::bind(&Connection::BeginRedirect, this));
			Send(response);
			break;
		}

	}
	break;
	case NetMessage::Type::Setup:
	case NetMessage::Type::Ping:
		break;
	case NetMessage::Type::Invalid:
	default:
		std::cout << myConnectedUser << " behaved badly and was kicked. sent unkown message type\n";
		myIsValid = false;
		return;
	}
}

void Connection::BeginRedirect()
{
	std::cout << "Sending server list\n";
	myIsRedirecting = true;
	myPotentialRedirectCount = 0;
	myRedirectStart = Tools::GetTotalTime();
	std::vector<Connection*> connections = myConnectionListRequestFunction(*this);
	for (auto& conn : connections)
	{
		if (conn->myIsValid && conn->myIsServer)
		{
			auto key = AddrToArray(*reinterpret_cast<sockaddr*>(& conn->myServerAddress));
			myEvaluations[key].myConnection = conn;
			StatusMessage message;
			message.myStatus = StatusMessage::Status::PotentialServer;
			memcpy(&message.myAddress, &conn->myServerAddress, sizeof(conn->myServerAddress));
			message.myServer.aAddressSize = sizeof(conn->myServerAddress);
			Send(message);
			std::cout << "\t" + conn->GetName() + "\t" + ReadableAddress(&message.myAddress) + "\n";
			++myPotentialRedirectCount;
		}
	}

}

void Connection::EvaluateRedirectResult(bool aIsTimeOutEval)
{
	int results = 0;
	for (auto& i : myEvaluations)
	{
		if (i.second.myHasResult)
		{
			++results;
		}
	}
	if (results >= myPotentialRedirectCount || aIsTimeOutEval)
	{
		std::cout << "Evaluating redirect for " + GetName() + "\n";
		Evaluation* best = nullptr;
		for (auto& i : myEvaluations)
		{
			if (!best)
			{
				best = &i.second;
			}
			if (i.second.myPing < best->myPing)
			{
				best = &i.second;
			}
		}
		if (best)
		{
			std::cout << best->myConnection->GetName() + " was the best fit.\n";
			myIsRedirecting = false;
			StatusMessage redirect;
			redirect.myStatus = StatusMessage::Status::ConnectToServer;
			WIPE(redirect.myAddress);
			memcpy(&redirect.myAddress,&best->myConnection->myServerAddress,sizeof(best->myConnection->myServerAddress));
			Send(redirect);
		}
		else
		{
			std::cout << "could not find any functioning servers of [" + std::to_string(myPotentialRedirectCount) + "], retrying\n";
			BeginRedirect();
		}
	}
}

void Connection::TimedOut()
{
	std::cout << GetName() + " Timed out\n";
	myIsValid = false;
}
