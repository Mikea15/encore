#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef enum
{
	LOG_LEVEL_TRACE = 0,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL,
	LOG_LEVEL_ASSERT
} LogLevel;

// Configuration
static struct
{
	LogLevel minLevel;
	bool bOutputToConsole;
	bool bOutputToDebugger;
	bool bOutputToFile;
	FILE* pLogFile;
	bool bIncludeTimestamp;
	bool include_level;
} g_logConfig = {
	.minLevel = LOG_LEVEL_TRACE,
	.bOutputToConsole = true,
	.bOutputToDebugger = true,
	.bOutputToFile = false,
	.pLogFile = NULL,
	.bIncludeTimestamp = true,
	.include_level = true
};

// Get formatted timestamp
static void GetLogTime(char* buffer, size_t buffer_size)
{
	if(!buffer || buffer_size == 0) return;

	SYSTEMTIME st;
	GetLocalTime(&st);

	_snprintf_s(buffer, buffer_size, _TRUNCATE,
		"%04d-%02d-%02d %02d:%02d:%02d.%03d",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
}

// Get level string
static const char* GetLevelString(LogLevel level)
{
	switch(level)
	{
	case LOG_LEVEL_TRACE:   return "TRACE";
	case LOG_LEVEL_INFO:    return "INFO";
	case LOG_LEVEL_WARNING: return "WARNING";
	case LOG_LEVEL_ERROR:   return "ERROR";
	case LOG_LEVEL_FATAL:   return "FATAL";
	case LOG_LEVEL_ASSERT:  return "ASSERT";
	default:                return "UNKNOWN";
	}
}

// Get level color for console (if supported)
static const char* GetLevelColor(LogLevel level)
{
	// ANSI color codes (work in modern Windows Terminal)
	switch(level)
	{
	case LOG_LEVEL_TRACE:   return "\033[37m";     // White
	case LOG_LEVEL_INFO:    return "\033[36m";     // Cyan
	case LOG_LEVEL_WARNING: return "\033[33m";     // Yellow
	case LOG_LEVEL_ERROR:   return "\033[31m";     // Red
	case LOG_LEVEL_FATAL:   return "\033[35m";     // Magenta
	case LOG_LEVEL_ASSERT:  return "\033[41;37m";  // Red background, white text
	default:                return "\033[0m";      // Reset
	}
}

// Core logging function
static void LogImpl(LogLevel level, const char* file, int line, const char* func,
	const char* format, va_list args)
{
	if(level < g_logConfig.minLevel)
	{
		return;
	}

	static const size_t BUFFER_SIZE = 2048;
	char formatedMessage[BUFFER_SIZE];
	char finalMessage[BUFFER_SIZE];
	char timestamp[64] = { 0 };

	// Format the user message
	_vsnprintf_s(formatedMessage, BUFFER_SIZE, _TRUNCATE, format, args);

	// Get timestamp if needed
	if(g_logConfig.bIncludeTimestamp)
	{
		GetLogTime(timestamp, sizeof(timestamp));
	}

	// Build final message
	char* pos = finalMessage;
	size_t remaining = BUFFER_SIZE;

	// Add timestamp
	if(g_logConfig.bIncludeTimestamp && timestamp[0])
	{
		int written = _snprintf_s(pos, remaining, _TRUNCATE, "[%s] ", timestamp);
		if(written > 0)
		{
			pos += written;
			remaining -= written;
		}
	}

	// Add level
	if(g_logConfig.include_level)
	{
		int written = _snprintf_s(pos, remaining, _TRUNCATE, "%-8s ", GetLevelString(level));
		if(written > 0)
		{
			pos += written;
			remaining -= written;
		}
	}

	// Add location for errors and asserts
	if(level >= LOG_LEVEL_ERROR && file && func)
	{
		int written = _snprintf_s(pos, remaining, _TRUNCATE, "\n\t >>> (%s:%d in %s)\n\t >>> ",
			strrchr(file, '\\') ? strrchr(file, '\\') + 1 : file,
			line, func);
		if(written > 0)
		{
			pos += written;
			remaining -= written;
		}
	}

	// Add the actual message
	_snprintf_s(pos, remaining, _TRUNCATE, "%s\n", formatedMessage);

	// Output to console
	if(g_logConfig.bOutputToConsole)
	{
		// Use colors for console output if it's an assert or error
		if(level >= LOG_LEVEL_WARNING)
		{
			printf("%s%s\033[0m", GetLevelColor(level), finalMessage);
		}
		else
		{
			printf("%s", finalMessage);
		}
		fflush(stdout);
	}

	// Output to debugger
	if(g_logConfig.bOutputToDebugger)
	{
		OutputDebugStringA(finalMessage);
	}

	// Output to file
	if(g_logConfig.bOutputToFile && g_logConfig.pLogFile)
	{
		fprintf(g_logConfig.pLogFile, "%s", finalMessage);
		fflush(g_logConfig.pLogFile);
	}
}

// Public logging functions
static void Log(LogLevel level, const char* file, int line, const char* func,
	const char* format, ...)
{
	va_list args;
	va_start(args, format);
	LogImpl(level, file, line, func, format, args);
	va_end(args);
}

// Configuration functions
static void LogSetMinLevel(LogLevel level)
{
	g_logConfig.minLevel = level;
}

static void LogSetOutputs(bool console, bool debugger, bool file)
{
	g_logConfig.bOutputToConsole = console;
	g_logConfig.bOutputToDebugger = debugger;
	g_logConfig.bOutputToFile = file;
}

static bool LogOpenFile(const char* filename)
{
	if(g_logConfig.pLogFile)
	{
		fclose(g_logConfig.pLogFile);
	}

	errno_t err = fopen_s(&g_logConfig.pLogFile, filename, "a");
	if(err == 0 && g_logConfig.pLogFile)
	{
		g_logConfig.bOutputToFile = true;
		return true;
	}
	return false;
}

static void LogCloseFile(void)
{
	if(g_logConfig.pLogFile)
	{
		fclose(g_logConfig.pLogFile);
		g_logConfig.pLogFile = NULL;
	}
	g_logConfig.bOutputToFile = false;
}

// Convenience macros
#define LOG_TRACE(format, ...)   Log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...)    Log(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define LOG_WARNING(format, ...) Log(LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...)   Log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define LOG_FATAL(format, ...)   Log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

// Macros without variadic arguments (for single string messages)
#define LOG_TRACE_STR(message)   Log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __FUNCTION__, "%s", message)
#define LOG_INFO_STR(message)    Log(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, "%s", message)
#define LOG_WARNING_STR(message) Log(LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__, "%s", message)
#define LOG_ERROR_STR(message)   Log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, "%s", message)
#define LOG_FATAL_STR(message)   Log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __FUNCTION__, "%s", message)
