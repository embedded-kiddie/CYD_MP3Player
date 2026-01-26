//================================================================================
// Defining debugging functions
//================================================================================
#ifndef _DEBUG_H_
#define _DEBUG_H_

//--------------------------------------------------------------------------------
// Enable to output audio information to the Serial Monitor
//--------------------------------------------------------------------------------
#define DEBUG_AUDIO 0

//--------------------------------------------------------------------------------
// Development purposes only
//--------------------------------------------------------------------------------
#define DEBUG       1

#if (DEBUG & 1)
#include <assert.h>
#define DBG_ASSERT(x) assert(x)
#else
#define DBG_ASSERT(x)
#endif

#if (DEBUG & 2)
#include <stdio.h>
#define DBG_EXEC(x)   x
#else
#define DBG_EXEC(x)
#endif

#endif // _DEBUG_H_