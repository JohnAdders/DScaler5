#include "config.h"

#include <string.h>
#include <stdio.h>
#include <windows.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#elif defined(HAVE_STDINT_H)
#include <stdint.h>
#endif

#include "../src/dvbpsi.h"
#include "../src/dvbpsi_private.h"

void _win32_debug_log(const char* src, const char* szFormat, ...)
{
    char szMessage[2048];
    va_list Args;
    int result;

    va_start(Args, szFormat);
    result = _vsnprintf(szMessage,2048, szFormat, Args);
    va_end(Args);
    if(result == -1)
    {
        OutputDebugString("DebugString too long, truncated!!\n");
    }
    OutputDebugString(szMessage);
}

void _win32_error_log(const char* src, const char* szFormat, ...)
{
    char szMessage[2048];
    va_list Args;
    int result;

    va_start(Args, szFormat);
    result = _vsnprintf(szMessage,2048, szFormat, Args);
    va_end(Args);
    if(result == -1)
    {
        OutputDebugString("DebugString too long, truncated!!\n");
    }
    OutputDebugString(szMessage);
}
