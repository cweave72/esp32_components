#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

static char *code =
    "avg, sum = average(10, 20, 30, 40,  50)"
    "print(\"The average is \", avg)"
    "print(\"The sum is \", sum)"
    "print(\"Version is \", VERSION)";

/* The function we'll call from the lua script */
static int average(lua_State *L)
{
    /* get number of arguments */
    int n = lua_gettop(L);
    double sum = 0;
    int i;

    /* loop through each argument */
    for (i = 1; i <= n; i++)
    {
        if (!lua_isnumber(L, i)) 
        {
            lua_pushstring(L, "Incorrect argument to 'average'");
            lua_error(L);
        }

        /* total the arguments */
        sum += lua_tonumber(L, i);
    }

    /* push the average */
    lua_pushnumber(L, sum / n);

    /* push the sum */
    lua_pushnumber(L, sum);

    /* return the number of results */
    return 2;
}

#define VERSION  "1.0"

void script_demo1(void)
{
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    // Our Lua code, it simply prints a Hello, World message
    //char * code = "print('Hello, World')";

    // Here we load the string and use lua_pcall for run the code
    if (luaL_loadstring(L, code) == LUA_OK)
    {
        lua_pushstring(L, VERSION);
        lua_setglobal(L, "VERSION");
        lua_register(L, "average", average);

        if (lua_pcall(L, 0, 0, 0) == LUA_OK)
        {
            // If it was executed successfuly we 
            // remove the code from the stack
            lua_pop(L, lua_gettop(L));
        }
    }

    lua_close(L);
}
