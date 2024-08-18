#include <stdio.h>
#include <string.h>
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

static char *code =
    "print('running test')\n"
    "a = {}\n"
    "for i=1, 100 do\n"
    "   a[i] = 5*i\n"
    "end\n"
    "sum = 0\n"
    "for item, value in ipairs(a) do\n"
    "   sum = sum + value\n"
    "   print(string.format(\"[%u]: sum=%u\", item, sum))\n"
    "end\n"
    "l = #a\n"
    "print(\"The length of array is \", l)\n"
    "print(\"The average is \", sum / l)\n"
    "print(\"The sum is \", sum)\n";

#define CHECK_RET(ret, call, jmp)                     \
if ((ret) != LUA_OK) {                                \
    printf("Error (%s): ret=%d\n", (call), (ret));    \
    goto jmp;                                         \
}

/*
** Message handler used to run all chunks
*/
static int
msghandler(lua_State *L)
{
    const char *msg = lua_tostring(L, 1);

    if (msg == NULL)
    {
        /* If the error object has a string method. */
        if (luaL_callmeta(L, 1, "__tostring") &&
            lua_type(L, -1) == LUA_TSTRING)
        {
            return 1;  /* that is the message */
        }
        else
        {
            msg = lua_pushfstring(L, "(error object is a %s value)",
                luaL_typename(L, 1));
        }
    }

    luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
    return 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function.
** Used to run all chunks.
*/
static int docall(lua_State *L, int narg, int nres)
{
    int status;
    int base = lua_gettop(L) - narg;    /* Get base of the stack */
    lua_pushcfunction(L, msghandler);   /* push message handler */
    lua_insert(L, base);                /* put it under function and args */
    status = lua_pcall(L, narg, nres, base);
    lua_remove(L, base);  /* remove message handler from the stack */
    return status;
}

/*
** Prints an error message, adding the program name in front of it (if present).
*/
static void
print_message(const char *pname, const char *msg)
{
    if (pname) {   printf("%s: ", pname); }
    printf("%s\n", msg);
}

/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack.
*/
static int
report(lua_State *L, int status)
{
    if (status != LUA_OK)
    {
        /* Get the message at the top of the stack. */
        const char *msg = lua_tostring(L, -1);
        if (msg == NULL) { msg = "(error message not a string)"; }
        print_message("lua", msg);
        lua_pop(L, 1);  /* remove message */
    }
    return status;
}

static int
dochunk(lua_State *L, int status)
{
    if (status == LUA_OK)
    {
        status = docall(L, 0, 0);
    }
    return report(L, status);
}


static
int dostring(lua_State *L, const char *s)
{
    return dochunk(L, luaL_loadbuffer(L, s, strlen(s), "=(string)"));
}

void script_demo2(void)
{
    int ret;
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    // Here we load the string and use lua_pcall for run the code
    //ret = luaL_loadstring(L, code);
    ret = dostring(L, code);
    CHECK_RET(ret, "dostring", Exit);
    // If it was executed successfuly we remove the code from the stack
    lua_pop(L, lua_gettop(L));

Exit:
    lua_close(L);
}
