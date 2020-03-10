// ClientMigration.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <pch.h>
#include <iostream>
#include <TClient.h>

int main()
{
	NetworkClient client;
	client.Connect();
	while (true)
	{
		client.Flush();
	}
	return 1;
}