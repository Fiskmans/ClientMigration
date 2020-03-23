#include <pch.h>
#include "Connection.h"
#include <SetupMessage.h>
#include <StatusMessage.h>
#include <NetworkHelpers.h>

Connection::Connection()
{
}

Connection::Connection(sockaddr_in aAddress, int aAddressSize, SOCKET aSocket, unsigned short aID, std::function<void(const NetMessage&)> aCallbackFunction)
{
	myCallbackFunction = aCallbackFunction;
	myID = aID;
	mySocket = aSocket;
	myAddress = aAddress;
	myAddressSize = aAddressSize;
	myIsValid = false;
}

bool Connection::IsAlive()
{
	return true; // TODO
}

void Connection::Send(const char* aData, int aDataSize, sockaddr* aCustomAddress)
{
	sendto(mySocket, aData,aDataSize, 0, (struct sockaddr*) & myAddress, myAddressSize);
}

void Connection::Receive(char* someData, const int aDataSize)
{
	std::cout << "[" + std::to_string(aDataSize) + "]\n";
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
	NetworkInterface::PreProcessAndSend(&response,sizeof(response));

	StatusMessage logonResponse;
	logonResponse.myAssignedID = myID;
	logonResponse.SetName(myConnectedUser);
	logonResponse.myStatus = StatusMessage::Status::UserConnected;
	myCallbackFunction(logonResponse);


	std::cout << "Connection Established with user: " << myConnectedUser << " on " << ReadableAddress((sockaddr*)&myAddress) << "\n";

	return true;
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
		if (status->myStatus == StatusMessage::Status::UserDisconnected)
		{
			myCallbackFunction(*status);
		}
		else
		{
			std::cout << myConnectedUser << " behaved badly and was kicked. sent status that was not disconnect\n";
			myIsValid = false;
		}
		}
		break;
	case NetMessage::Type::Invalid:
	case NetMessage::Type::Setup:
	default:
		std::cout << myConnectedUser << " behaved badly and was kicked. sent unkown message type\n";
		myIsValid = false;
		return;
	}
}

void Connection::TimedOut()
{
	myIsValid = false;
}
