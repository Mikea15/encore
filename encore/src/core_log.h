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
	LogLevel min_level;
	bool output_to_console;
	bool output_to_debugger;
	bool output_to_file;
	FILE* log_file;
	bool include_timestamp;
	bool include_level;
} g_log_config = {
	.min_level = LOG_LEVEL_TRACE,
	.output_to_console = true,
	.output_to_debugger = true,
	.output_to_file = false,
	.log_file = NULL,
	.include_timestamp = true,
	.include_level = true
};

// Get formatted timestamp
static void GetLogTime(char* buffer, size_t buffer_size)
{
	if(!buffer || buffer_size == 0) return;

	SYSTEMTIME st;
	GetLocalTime(&st);

	_snprintf_s(buffer, buffer_size, _TRUNCATE,
		"[%04d-%02d-%02d %02d:%02d:%02d.%03d]",
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
	if(level < g_log_config.min_level)
	{
		return;
	}

	static const size_t BUFFER_SIZE = 2048;
	char formatted_message[BUFFER_SIZE];
	char final_message[BUFFER_SIZE];
	char timestamp[64] = { 0 };

	// Format the user message
	_vsnprintf_s(formatted_message, BUFFER_SIZE, _TRUNCATE, format, args);

	// Get timestamp if needed
	if(g_log_config.include_timestamp)
	{
		GetLogTime(timestamp, sizeof(timestamp));
	}

	// Build final message
	char* pos = final_message;
	size_t remaining = BUFFER_SIZE;

	// Add timestamp
	if(g_log_config.include_timestamp && timestamp[0])
	{
		int written = _snprintf_s(pos, remaining, _TRUNCATE, "[%s] ", timestamp);
		if(written > 0)
		{
			pos += written;
			remaining -= written;
		}
	}

	// Add level
	if(g_log_config.include_level)
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
	_snprintf_s(pos, remaining, _TRUNCATE, "%s\n", formatted_message);

	// Output to console
	if(g_log_config.output_to_console)
	{
		// Use colors for console output if it's an assert or error
		if(level >= LOG_LEVEL_WARNING)
		{
			printf("%s%s\033[0m", GetLevelColor(level), final_message);
		}
		else
		{
			printf("%s", final_message);
		}
		fflush(stdout);
	}

	// Output to debugger
	if(g_log_config.output_to_debugger)
	{
		OutputDebugStringA(final_message);
	}

	// Output to file
	if(g_log_config.output_to_file && g_log_config.log_file)
	{
		fprintf(g_log_config.log_file, "%s", final_message);
		fflush(g_log_config.log_file);
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
	g_log_config.min_level = level;
}

static void LogSetOutputs(bool console, bool debugger, bool file)
{
	g_log_config.output_to_console = console;
	g_log_config.output_to_debugger = debugger;
	g_log_config.output_to_file = file;
}

static bool LogOpenFile(const char* filename)
{
	if(g_log_config.log_file)
	{
		fclose(g_log_config.log_file);
	}

	errno_t err = fopen_s(&g_log_config.log_file, filename, "a");
	if(err == 0 && g_log_config.log_file)
	{
		g_log_config.output_to_file = true;
		return true;
	}
	return false;
}

static void LogCloseFile(void)
{
	if(g_log_config.log_file)
	{
		fclose(g_log_config.log_file);
		g_log_config.log_file = NULL;
	}
	g_log_config.output_to_file = false;
}

// Convenience macros
#define LOG_TRACE(format, ...)   Log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)
#define LOG_INFO(format, ...)    Log(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)
#define LOG_WARNING(format, ...) Log(LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)
#define LOG_ERROR(format, ...)   Log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)
#define LOG_FATAL(format, ...)   Log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __FUNCTION__, format, __VA_ARGS__)

// Macros without variadic arguments (for single string messages)
#define LOG_TRACE_STR(message)   Log(LOG_LEVEL_TRACE, __FILE__, __LINE__, __FUNCTION__, "%s", message)
#define LOG_INFO_STR(message)    Log(LOG_LEVEL_INFO, __FILE__, __LINE__, __FUNCTION__, "%s", message)
#define LOG_WARNING_STR(message) Log(LOG_LEVEL_WARNING, __FILE__, __LINE__, __FUNCTION__, "%s", message)
#define LOG_ERROR_STR(message)   Log(LOG_LEVEL_ERROR, __FILE__, __LINE__, __FUNCTION__, "%s", message)
#define LOG_FATAL_STR(message)   Log(LOG_LEVEL_FATAL, __FILE__, __LINE__, __FUNCTION__, "%s", message)
