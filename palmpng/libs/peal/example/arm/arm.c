#include <Standalone.h>
#include <stdint.h>
#include <stdlib.h>
#include "pealstub.h"

STANDALONE_CODE_RESOURCE_ID(1000);


uint32_t CallCount = 0;
void *Callback_m68k = NULL;

extern char *ReturnOne(void);
extern char *ReturnTwo(void);
static char * (*fn)(void);


// Demonstrate zerofilled non-constant data
uint32_t GetCallCount(void)
{
    CallCount++;
    return CallCount;
}


// Demonstate function pointers and non-local functions
void SetFunctionPointer(uint32_t which)
{
    CallCount++;
    fn = which ? ReturnOne : ReturnTwo;
}


// Demonstrate function pointers
char *CallFunctionPointer(void)
{
    CallCount++;
    return (*fn)();
}


// Demonstrate 68K calls using globals set by PealArmStub()
uint32_t Call68K(void)
{
    return (*gCall68KFuncP)(gEmulStateP, (unsigned long)Callback_m68k, NULL, 0);
}
