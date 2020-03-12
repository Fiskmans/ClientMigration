#pragma once

//Constants
#define PI 3.14159265359f
#define LOCAL_HOST "127.0.0.1"
#define CUBEHALFSIZETOENCAPSULATINGSPHERERADIUS 1.73f // sqrt(3)
#define BUFLEN 576	//Max length of buffer
#define MAXUSERNAMELENGTH 128
#define GAMESERVERCOLOR FOREGROUND_BLUE | FOREGROUND_RED
#define GAMECOLORS 


//Flags
#define TRACKPERFORMANCE false

//Values
#define STANDARDMARG 0.00001

//Funtions
#define CAST(type, value) (static_cast<type>(value))
#define ENUM_CAST(value) (static_cast<int>(value))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define CLOSEENUF(a, b) (static_cast<double>(abs(a - b)) <= STANDARDMARG)
#define CLOSEENUFCUSTOM(a, b, margin) (abs(a - b) <= margin)

#define SAFE_DELETE(pointer) if(pointer) { delete (pointer); (pointer) = nullptr; }
#define SAFE_RELEASE(pointer) if(pointer) { pointer->Release(); pointer = nullptr; }

#define SAFE_DELETE_ARRAY(pointer) if(pointer) { delete[] (pointer); (pointer) = nullptr; }

#define CYCLIC_ERASE(vector, index) if(index >= 0 && index < vector.size()) { if(vector.size() > 0) { vector.at(index) = vector.at(vector.size() - 1); vector.pop_back(); } } else { assert(false); }

#define BIT(x) (1ULL << x)

#define LERP(a, b, val) ((a) * (1 - (val)) + (val) * (b))
#define INVERSELERP(a, b, val) (((val) - (a)) / ((b) - (a)))

#define CLAMP(low, high, value) ((value) < (low) ? (low) : ((high) < (value) ? (high) : (value)))

#ifndef ZEROMEMORY
#define ZEROMEMORY(adr, size) memset(adr, 0, size)
#endif // !ZeroMemory

#define WIPE(item) ZEROMEMORY(&item, sizeof(item))

#define STRING(arg) #arg
#define STRINGVALUE(arg) STRING(arg)

#ifdef _DEBUG
#define NAMETHREAD(name) SetThreadDescription(GetCurrentThread(), name);
#else
#define NAMETHREAD(name) ((void*)0);
#endif // _DEBUG

#if TRACKPERFORMANCE
#define PERFORMANCETAG(name) auto _ = Tools::ScopeDiagnostic(name);
#else
#define PERFORMANCETAG(name) ((void*)0);
#endif


#define TODEG(arg) ((arg) * 57.2957795131f)
#define TORAD(arg) ((arg) * 0.01745329251f)
