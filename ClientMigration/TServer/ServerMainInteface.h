#pragma once

class CServerMain;
class IServerMain
{
public:
	IServerMain();
	~IServerMain();
	void StartServer();
private:
	CServerMain* myServer;

};

