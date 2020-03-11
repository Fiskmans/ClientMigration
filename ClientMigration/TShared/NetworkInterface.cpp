#include <pch.h>
#include "NetworkInterface.h"
#include <string>
#include "TimeHelper.h"
#include "NetPackResponse.h"
#include <iostream>

#define RESENDTIMER 0.2f
#define MESSAGEDUPLICATIONTIMEOUT 2.f
#define LOGTRAFFIC false
#define RESENDCAP 40

void NetworkInterface::PreProcessAndSend(const NetMessage* aMessage, const int aDataSize)
{
	if (aMessage->IsImportant())
	{
		SentMessageTracker tracker;
#if LOGTRAFFIC
		std::cout << "Sent trackable package ID [" +  std::to_string(aMessage->myID) + "]\n";
#endif
		memcpy(tracker.package, aMessage, aDataSize);
		tracker.packageSize = aDataSize;

		tracker.ID = aMessage->myID;
		tracker.expectedHash = std::hash<std::string>()(std::string(reinterpret_cast<const char*>(aMessage), aDataSize));
		tracker.lastSent = Tools::GetTotalTime();
		tracker.sentCount = 0;
		
		myPendingAccepts.push_back(tracker);
	}
	Send(reinterpret_cast<const char*>(aMessage), aDataSize);
}

bool NetworkInterface::PreProcessReceive(char* someData, const int aDataSize)
{
	NetMessage* mess = reinterpret_cast<NetMessage*>(someData);
	bool consumed = false;

	if (mess->IsImportant())
	{
		for (size_t i = 0; i < myRecentMesseges.size(); i++)
		{
			if (myRecentMesseges[i].first == mess->myID)
			{
				consumed = true;
				myRecentMesseges[i].second = Tools::GetTotalTime();
				myRecentMesseges.push_back(myRecentMesseges[i]);
				myRecentMesseges.erase(myRecentMesseges.begin() + i);
#if LOGTRAFFIC
				std::cout << "Detected duplicate package, re-sending acc and ignoring\n";
#endif
				break;
			}
		}
		if (!consumed)
		{
			myRecentMesseges.emplace_back(mess->myID, Tools::GetTotalTime());
		}
#if LOGTRAFFIC
		std::cout << "Sent message acc for packageid: [" + std::to_string(mess->myID) + "]\n";
#endif
		NetPackResponse response;
		response.myPackageID = mess->myID;
		response.myHash = std::hash<std::string>()(std::string(someData, aDataSize));

		Send(reinterpret_cast<char*>(&response), sizeof(response));
	}

	if (mess->myType == NetMessage::Type::PackageResponse)
	{
		NetPackResponse* response = reinterpret_cast<NetPackResponse*>(mess);

		for (size_t i = 0; i < myPendingAccepts.size(); i++)
		{
			if (myPendingAccepts[i].ID == response->myPackageID && myPendingAccepts[i].expectedHash == response->myHash)
			{
#if LOGTRAFFIC
				std::cout << "Pending package with ID " + std::to_string(myPendingAccepts[i].ID) + " acknowledgement received.\n";
#endif
				myPendingAccepts.erase(myPendingAccepts.begin() + i);
				
				auto it = myCallbacks.find(response->myPackageID);
				if (it != myCallbacks.end())
				{
#if LOGTRAFFIC
					std::cout << "Package had a callback hooked, calling hook.\n";
#endif
					it->second(response->myPackageID);
					myCallbacks.erase(it->first);
				}
				break;
			}
		}
		consumed = true;
	}
	
	return !consumed;
}

void NetworkInterface::Flush()
{
	float now = Tools::GetTotalTime();
	for (auto& acc : myPendingAccepts)
	{
		if (now - acc.lastSent > RESENDTIMER)
		{
			if (acc.sentCount > RESENDCAP)
			{
				TimedOut();
			}
			acc.lastSent = now;
			Send(acc.package,acc.packageSize);
			acc.sentCount++;
#if LOGTRAFFIC
			std::cout << "Re-Sent package with id [" + std::to_string(reinterpret_cast<NetMessage*>(acc.package)->myID) + "]\n";
#endif
		}
	}
	while (!myRecentMesseges.empty() && now - myRecentMesseges.front().second > MESSAGEDUPLICATIONTIMEOUT)
	{
		myRecentMesseges.erase(myRecentMesseges.begin());
	}
}

void NetworkInterface::Clear()
{
	myPendingAccepts.clear();
	myRecentMesseges.clear();
}

void NetworkInterface::HookCallBack(NetMessageIdType aID, std::function<void(NetMessageIdType)> aFunction)
{
	myCallbacks[aID] = aFunction;
}
