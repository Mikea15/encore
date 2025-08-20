#pragma once

#include "core_types.h"
#include "core_platform.h"
#include "core_log.h"

#include <crtdbg.h>

#define COMPILE_DEMO 0

#ifdef _MSC_VER
#define ENC_FUNC_NAME __FUNCTION__
#else
#define ENC_FUNC_NAME __func__
#endif

#if ENC_DEBUG

// Internal assert logging function
static void _log_assert(const char* type, const char* condition, const char* message,
	const char* file, int line, const char* func)
{
	// Log using the logging system
	if(message && *message)
	{
		Log(LOG_LEVEL_ASSERT, file, line, func, "%s FAILED: %s - %s", type, condition, message);
	}
	else
	{
		Log(LOG_LEVEL_ASSERT, file, line, func, "%s FAILED: %s", type, condition);
	}
}

// ASSERT functions: Crashes/stops execution and logs
static bool _assert_impl(bool condition, const char* condition_str, const char* message,
	const char* file, int line, const char* func)
{
	if(!condition)
	{
		_log_assert("ASSERT", condition_str, message, file, line, func);
		__debugbreak();
	}
	return condition;
}

// ENSURE functions: Stops execution and logs (but allows continuing in debugger)
static bool _ensure_impl(bool condition, const char* condition_str, const char* message,
	const char* file, int line, const char* func)
{
	if(!condition)
	{
		_log_assert("ENSURE", condition_str, message, file, line, func);
		if(_CrtDbgReport(_CRT_ASSERT, file, line, NULL,
			message ? "ENSURE FAILED: %s - %s" : "ENSURE FAILED: %s",
			condition_str, message) == 1)
		{
			__debugbreak();
		}
	}
	return condition;
}

// VERIFY functions: Only logs, continues execution
static bool _verify_impl(bool condition, const char* condition_str, const char* message,
	const char* file, int line, const char* func)
{
	if(!condition)
	{
		_log_assert("VERIFY", condition_str, message, file, line, func);
	}
	return condition;
}

// Public function interfaces
#define Assert(condition) \
    _assert_impl(!!(condition), #condition, NULL, __FILE__, __LINE__, ENC_FUNC_NAME)

#define AssertMsg(condition, message) \
    _assert_impl(!!(condition), #condition, message, __FILE__, __LINE__, ENC_FUNC_NAME)

#define Ensure(condition) \
    _ensure_impl(!!(condition), #condition, NULL, __FILE__, __LINE__, ENC_FUNC_NAME)

#define EnsureMsg(condition, message) \
    _ensure_impl(!!(condition), #condition, message, __FILE__, __LINE__, ENC_FUNC_NAME)

#define Verify(condition) \
    _verify_impl(!!(condition), #condition, NULL, __FILE__, __LINE__, ENC_FUNC_NAME)

#define VerifyMsg(condition, message) \
    _verify_impl(!!(condition), #condition, message, __FILE__, __LINE__, ENC_FUNC_NAME)

// Expression versions
#define AssertExpr(condition) \
    _assert_impl(!!(condition), #condition, NULL, __FILE__, __LINE__, ENC_FUNC_NAME)

#define EnsureExpr(condition) \
    _ensure_impl(!!(condition), #condition, NULL, __FILE__, __LINE__, ENC_FUNC_NAME)

#define VerifyExpr(condition) \
    _verify_impl(!!(condition), #condition, NULL, __FILE__, __LINE__, ENC_FUNC_NAME)

// Utility functions that return values for early returns
#define AssertReturn(condition, return_value) \
    do { \
        if (!_assert_impl(!!(condition), #condition, "Returning: " #return_value, \
                         __FILE__, __LINE__, ENC_FUNC_NAME)) { \
            return (return_value); \
        } \
    } while(0)

#define EnsureReturn(condition, return_value) \
    do { \
        if (!_ensure_impl(!!(condition), #condition, "Returning: " #return_value, \
                         __FILE__, __LINE__, ENC_FUNC_NAME)) { \
            return (return_value); \
        } \
    } while(0)

#define VerifyReturn(condition, return_value) \
    do { \
        if (!_verify_impl(!!(condition), #condition, "Returning: " #return_value, \
                         __FILE__, __LINE__, ENC_FUNC_NAME)) { \
            return (return_value); \
        } \
    } while(0)


#else // Release build - all functions become no-ops or pass-through

// Release versions that compile to nothing
static inline bool _noop_true(bool condition, ...) { return true; }
static inline bool _noop_condition(bool condition, ...) { return condition; }

#define Assert(condition)               _noop_true(!!(condition))
#define AssertMsg(condition, message)   _noop_true(!!(condition))
#define Ensure(condition)               _noop_condition(!!(condition))
#define EnsureMsg(condition, message)   _noop_condition(!!(condition))
#define Verify(condition)               _noop_condition(!!(condition))
#define VerifyMsg(condition, message)   _noop_condition(!!(condition))

#define AssertReturn(condition, return_value) \
    do { if (!(condition)) return (return_value); } while(0)
#define EnsureReturn(condition, return_value) \
    do { if (!(condition)) return (return_value); } while(0)
#define VerifyReturn(condition, return_value) ((void)0)

#define AssertExpr(condition) _noop_condition(!!(condition))
#define EnsureExpr(condition) _noop_condition(!!(condition))
#define VerifyExpr(condition) _noop_condition(!!(condition))

#endif // ENC_DEBUG

#if COMPILE_DEMO

static void test_assert_system()
{
	int x = 5;
	int y = 10;

	// Configure logging
	LogSetMinLevel(LOG_LEVEL_TRACE);
	LogSetOutputs(true, true, false);  // Console and debugger only

	// Test different log levels
	LOG_TRACE("Application starting up");
	LOG_INFO("Loading configuration from file: %s", "config.ini");
	LOG_WARNING("Could not find optional plugin: %s", "optional_plugin.dll");
	LOG_ERROR("Failed to connect to database, retrying in %d seconds", 5);
	LOG_FATAL("Critical system failure, shutting down");

	// Test VERIFY functions (should log but continue)
	OutputDebugStringA("\n--- Testing VERIFY Functions ---\n");
	Verify(x < y);  // Should pass
	VerifyMsg(x > y, "This should fail and log");  // Should log

	// Test expression usage
	bool result = VerifyExpr(x < y);  // Can be used in expressions
	OutputDebugStringA("VERIFY test completed - execution continued\n");

	// Test ENSURE functions (will show dialog in debug, can continue)
	OutputDebugStringA("\n--- Testing ENSURE Functions ---\n");
	Ensure(x < y);  // Should pass
	EnsureMsg(x > y, "This will show dialog");  // Uncomment to test dialog

	// Test ASSERT functions (will break into debugger)
	OutputDebugStringA("\n--- Testing ASSERT Functions ---\n");
	Assert(x < y);  // Should pass
	AssertMsg(x > y, "This will break into debugger");  // Uncomment to test break
}

// Example function demonstrating usage
int DivideSafe(int numerator, int denominator)
{
	// ASSERT: Critical condition that should never happen
	AssertMsg(denominator != 0, "Division by zero is not allowed");

	// ENSURE: Important condition, but we can handle it
	EnsureReturn(denominator != 0, 0);  // Early return with safe value

	// VERIFY: Log suspicious but non-critical conditions
	VerifyMsg(numerator >= 0, "Negative numerator might be unexpected");

	return numerator / denominator;
}
#endif
