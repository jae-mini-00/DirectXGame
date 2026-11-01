#pragma once

class Logger
{
	public :
		static void Info(const char* where, const char* message);
		static void Warning(const char* where, const char* message);
		static void Error(const char* where, const char* message);
};