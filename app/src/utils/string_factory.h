#pragma once

#include "core/core_minimal.h"
#include "memory/base_arena.h"

class StringFactory
{
public:
	static void Init(Arena* pPermanent, Arena* pFrameArena) 
	{ 
		AssertMsg(pPermanent != nullptr, "Arena null! Call StringFactory::Init()!");
		AssertMsg(pFrameArena != nullptr, "Arena null! Call StringFactory::Init()!");
		sm_pArena = pPermanent;
		sm_pFrameArena = pFrameArena;
	}

	static const char* Format(const char* pFmt)
	{
		AssertMsg(sm_pArena != nullptr, "Arena null! Call StringFactory::Init()!");
		return arena_sprintf(sm_pArena, pFmt);
	}

	static const char* TempFormat(const char* pFmt)
	{
		AssertMsg(sm_pFrameArena != nullptr, "Arena null! Call StringFactory::Init()!");
		return arena_sprintf(sm_pFrameArena, pFmt);
	}

	static const char* Format(const char* pFmt, ...)
	{
		AssertMsg(sm_pArena != nullptr, "Arena null! Call StringFactory::Init()!");
		va_list args;
		va_start(args, pFmt);
		char* pResult = arena_vsprintf(sm_pArena, pFmt, args);
		va_end(args);
		return pResult;
	}

	static const char* TempFormat(const char* pFmt, ...)
	{
		AssertMsg(sm_pFrameArena != nullptr, "Arena null! Call StringFactory::Init()!");
		va_list args;
		va_start(args, pFmt);
		char* pResult = arena_vsprintf(sm_pFrameArena, pFmt, args);
		va_end(args);
		return pResult;
	}

	static const char* MakeString(const char* pStr)
	{
		AssertMsg(sm_pArena != nullptr, "Arena null! Call StringFactory::Init()!");
		return arena_strdup(sm_pArena, pStr);
	}

	static const char* TempMakeString(const char* pStr)
	{
		AssertMsg(sm_pFrameArena != nullptr, "Arena null! Call StringFactory::Init()!");
		return arena_strdup(sm_pFrameArena, pStr);
	}

private:
	static Arena* sm_pArena;
	static Arena* sm_pFrameArena;
};

