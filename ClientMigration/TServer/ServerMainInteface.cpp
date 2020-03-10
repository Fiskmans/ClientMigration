#include "stdafx.h"
#include "ServerMainInteface.h"
#include "ServerMain.h"


IServerMain::IServerMain()
	:myServer(nullptr)
{
}


IServerMain::~IServerMain()
{
}

void IServerMain::StartServer()
{
	if (myServer)
	{
		PRINT_ERROR(L"Server already started!");
		return;
	}
	myServer = new CServerMain();
	if (!myServer)
	{
		PRINT_ERROR(L"Out of memory");
	}
	myServer->StartServer();
}
