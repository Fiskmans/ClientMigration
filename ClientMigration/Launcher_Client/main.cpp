// ClientMigration.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <pch.h>
#include <iostream>
#include <TClient.h>

int main()
{
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	NetworkClient client;
	client.Connect();
	while (true)
	{
		client.Flush();
	}
	return 1;
}