#include <pch.h>
#include "Connection.h"
#include <SetupMessage.h>
#include <StatusMessage.h>
#include <StatusMessage.h>
#include <NetIdentify.h>


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
}


bool Connection::IsAlive()
{
	return true; // TODO
}

void Connection::Send(const char* aData, int aDataSize)
{
	sendto(mySocket, aData, aDataSize, 0, (struct sockaddr*) & myAddress, myAddressSize);
}

void Connection::Receive(char* someData, const int aDataSize)
{
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
	NetworkInterface::PreProcessAndSend(&response, sizeof(response));

	StatusMessage logonResponse;
	logonResponse.myID = myID;
	logonResponse.SetName(myConnectedUser);
	logonResponse.myStatus = StatusMessage::Status::UserConnected;
	myCallbackFunction(logonResponse);

	char addressBuffer[512];
	InetNtopA(AF_INET, &myAddress, addressBuffer, 512);
	std::string addrstring = addressBuffer;

	std::cout << "Connection Established with user: " << myConnectedUser << " on " << addrstring << "\n";

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
	case NetMessage::Type::Identify:
	{
		NetIdentify* ident = reinterpret_cast<NetIdentify*>(aData);
		switch (ident->myProcessType)
		{
		case NetIdentify::IdentificationType::IsServer:
			std::cout << "Server Connected\n";
			//myAddress.sin_port = htons(ident->myIsServer.myPort);
			myIsServer = true;
			break;
		case NetIdentify::IdentificationType::IsClient:
			std::cout << "Client Connected\n";
			myIsServer = false;
			NetIdentify response;
			response.myProcessType = NetIdentify::IdentificationType::IsServer;
			NetworkInterface::HookCallBack(response.myID, std::bind(&Connection::SendServerStatus, this));
			Send(response);
			break;
		}

	}
	break;
	case NetMessage::Type::Setup:
		break;
	case NetMessage::Type::Invalid:
	default:
		std::cout << myConnectedUser << " behaved badly and was kicked. sent unkown message type\n";
		myIsValid = false;
		return;
	}
}

void Connection::SendServerStatus()
{
	std::cout << "Sending server list\n";
	std::vector<Connection*> connections = myConnectionListRequestFunction(*this);
	for (auto& conn : connections)
	{
		if (conn->myIsValid && conn->myIsServer)
		{
			std::cout << "\t" + conn->GetName() + "\n";
			StatusMessage message;
			message.myStatus = StatusMessage::Status::PotentialServer;
			memcpy(&message.myServer.aAddress, &conn->myAddress, conn->myAddressSize);
			message.myServer.aAddressSize = conn->myAddressSize;
			Send(message);
		}
	}

}

void Connection::TimedOut()
{
	myIsValid = false;
}
