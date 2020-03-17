#pragma once
#include <string>

template<size_t SIZE>
class EasyAssignString
{
public:

	EasyAssignString& operator=(const std::string& aString)
	{
		strcpy_s<SIZE>(myBuffer, aString.c_str());
		return *this;
	}

	operator std::string() 
	{
		return myBuffer;
	}

private:
	char myBuffer[SIZE];
};

