// Launcher_GameServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <pch.h>
#include <iostream>
#include <ServerMain.h>

int main()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GAMESERVERCOLOR);
	CServerMain server;
	server.StartServer();
	return EXIT_SUCCESS; 
}