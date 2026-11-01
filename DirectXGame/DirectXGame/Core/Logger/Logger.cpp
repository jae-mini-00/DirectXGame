#include "Logger.h"
#include <Windows.h>

void Logger::Info(const char* where, const char* message)
{
	OutputDebugStringA("[Info] ");
	OutputDebugStringA(where);
	OutputDebugStringA(": ");
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
}

void Logger::Warning(const char* where, const char* message)
{
	OutputDebugStringA("[Warning] ");
	OutputDebugStringA(where);
	OutputDebugStringA(": ");
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
}

void Logger::Error(const char* where, const char* message)
{
	OutputDebugStringA("[Error] ");
	OutputDebugStringA(where);
	OutputDebugStringA(": ");
	OutputDebugStringA(message);
	OutputDebugStringA("\n");
}