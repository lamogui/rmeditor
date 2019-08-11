#ifndef JASSERT_HPP
#define JASSERT_HPP

#include "logmanager.hpp"

#define JUCE_BLOCK_WITH_FORCED_SEMICOLON(x) do { x } while (false)

#if _DEBUG

// Try to reproduce the juce jjassert behavior
#ifdef _MSC_VER
#include <windows.h>

#ifndef __INTEL_COMPILER
#pragma intrinsic (__debugbreak)
#endif

#define jassertfalse JUCE_BLOCK_WITH_FORCED_SEMICOLON(Log::Assertion(QString("[") + __FILE__ + "] line " + QString::number(__LINE__) + " assertion failed !"); if (IsDebuggerPresent()) __debugbreak(); )
#define jassert(expression) JUCE_BLOCK_WITH_FORCED_SEMICOLON(if (! (expression)) jassertfalse;)

#else 
#pragma error "TODO !"

#endif // MSVC

#else 

#define jassertfalse JUCE_BLOCK_WITH_FORCED_SEMICOLON(;)
#define jassert(x) JUCE_BLOCK_WITH_FORCED_SEMICOLON(;)

#endif // _DEBUG




#endif // JASSERT_HPP