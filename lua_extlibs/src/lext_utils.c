/*******************************************************************************
 *  @file: lext_utils.c
 *  
 *  @brief: Utilities for lua extension libraries.
*******************************************************************************/
#include "lext_utils.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "lext_utils";

#define STACK_ABS2REL(_abs, top)   ((_abs)-(top)-1)

/******************************************************************************
    [docimport lext_printstack]
*//**
    @brief Prints the current stack.
******************************************************************************/
void
lext_printstack(lua_State *L)
{
    int i;
    int top = lua_gettop(L);

    printf("Stack: top = %d\n", top);
    for (i = 1; i <= top; i++)
    {
        int t = lua_type(L, i);
        switch (t)
        {
          case LUA_TSTRING:  /* strings */
              printf("[%d|%d]: '%s'\n",
                  i, STACK_ABS2REL(i, top), lua_tostring(L, i));
            break;
    
          case LUA_TBOOLEAN:  /* booleans */
            printf("[%d|%d]: %s\n",
                i, STACK_ABS2REL(i, top), lua_toboolean(L, i) ? "true" : "false");
            break;
    
          case LUA_TNUMBER:  /* numbers */
            printf("[%d|%d]: %g\n",
                i, STACK_ABS2REL(i, top), lua_tonumber(L, i));
            break;
    
          default:  /* other values */
            printf("[%d|%d]: %s\n",
                i, STACK_ABS2REL(i, top), lua_typename(L, t));
            break;
        }
    }
}
