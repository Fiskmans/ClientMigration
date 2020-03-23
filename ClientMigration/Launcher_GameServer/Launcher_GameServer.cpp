// Launcher_GameServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <pch.h>
#include <iostream>
#include <ServerMain.h>
#include <ClipBoardHelper.h>

void DieWith(HANDLE parent)
{
	std::cout << "Hooked to process: " + std::to_string(reinterpret_cast<long>(parent)) + ":" + std::to_string(GetProcessId(parent)) + "\n";
	DWORD code;
	do
	{
		std::this_thread::yield();
		if (GetExitCodeProcess(parent, &code) != TRUE)
		{
			std::cout << "error detected [" + std::to_string(GetLastError()) + "], closing\n";
			break;
		}

	} while (code == STILL_ACTIVE);
	std::cout << "Parent Closed, closing self\n";
	exit(EXIT_SUCCESS);
}

int main()
{
	std::cout << "Started: " + std::to_string(GetCurrentProcessId()) + "\n";

	using namespace std::string_literals;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), GAMESERVERCOLOR);
	std::string clip = Tools::ReadFromClipboard();
	std::wstring wclip = std::wstring(clip.begin(), clip.end());

	
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);


	if (wclip.size() > strlen("CommandLine: ") && wclip.substr(0, strlen("CommandLine: ")) == L"CommandLine: ")
	{
		std::wcout << L"parsing commandline: " + wclip;
		LPWSTR* argv = CommandLineToArgvW(wclip.c_str() + strlen("CommandLine: "),&argc);
	}
	for (size_t i = 0; i < argc; i++)
	{
		if (!argv[i])
		{
			break;
		}
		std::wcout << L"Argument: "s + argv[i] + L"\n";

		std::wstring arg = argv[i];
		if (arg.size() > strlen("Clients:") && arg.substr(0,strlen("Clients:")) == L"Clients:")
		{
			std::wstring trimmedArg = std::wstring(argv[i] + strlen("Clients:"));
			size_t charpos = trimmedArg.find_first_not_of(L"1234567890");
			if (charpos == std::wstring::npos)
			{
				long count = std::stoi(trimmedArg);
				std::string clipContent = Tools::ReadFromClipboard();
				std::wstring exeName = std::wstring(*argv, (*argv) + std::wstring(*argv).size());
				std::wstring commandline = L"Parent:" + std::to_wstring(GetCurrentProcessId());
				std::wcout << L"Starting Process: " + exeName + L"\n" << std::flush;
				std::wstring uglyCommandline = L"CommandLine: " + exeName + L" " + commandline;
				std::string	 uglynowideCommandLine = std::string(uglyCommandline.begin(), uglyCommandline.end());
				Tools::WriteToClipboard(uglynowideCommandLine);
				for (size_t i = 0; i < count; i++)
				{
					STARTUPINFO si;
					PROCESS_INFORMATION pi;

					WIPE(si);
					si.cb = sizeof(si);
					WIPE(pi);


					if (CreateProcessW(exeName.data(), &commandline[0], NULL,NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) == FALSE)
					{
						std::cout << "It failed: " + std::to_string(GetLastError()) + "\n";
					}
					else
					{
						std::cout << "It success\n" << std::flush;
					}




					CloseHandle(pi.hProcess);
					CloseHandle(pi.hThread);
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
				Tools::WriteToClipboard(clipContent);
			}
			else
			{
				std::cout << "'Client:' arg has invalid character [" + std::string(1, trimmedArg[charpos]) + "]\n";
			}
		}
		else if (arg.size() > strlen("Parent:") && arg.substr(0, strlen("Parent:")) == L"Parent:")
		{
			std::wstring trimmedArg = std::wstring(argv[i] + strlen("Parent:"));
			size_t charpos = trimmedArg.find_first_not_of(L"1234567890");
			if (charpos == std::wstring::npos)
			{
				long parentpid = std::stol(trimmedArg);
				HANDLE parent = OpenProcess(PROCESS_QUERY_INFORMATION,FALSE,parentpid);
				if (parent == NULL)
				{
					std::cout << "Could not access process handle of pid " + std::to_string(parentpid) + "\n";
				}
				else
				{
					std::thread t = std::thread(&DieWith, parent);
					t.detach();
				}
			}
			else
			{
				std::cout << "'Client:' arg has invalid character [" + std::string(1, trimmedArg[charpos]) + "]\n";
			}

		}
		else
		{
			std::wcout << L"Unkown argument: "s + argv[i] + L"\n";
		}
	}
	CServerMain server;
	server.StartServer();
	return EXIT_SUCCESS; 
}