/*******************************************************************************
 *  @file: lext_timerlib.c
 *  
 *  @brief: Lua extension library: Timer
*******************************************************************************/
#include "SwTimer.h"
#include "RtosUtils.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "LogPrint.h"
#include "LogPrint_local.h"

static const char *TAG = "lext_timerlib";

/** @brief Create a new timer object and associate a metatable. */
static int
timer_new(lua_State *L)
{
    /*  Create a new SwTimer object.
        This allows lua to be responsible for the allocated memory.
        Puts the new allocated object on the stack.
     */
    lua_newuserdatauv(L, sizeof(SwTimer), 0);

    /* Get the metatable to associate with the new object. This allows us to
        check for a Timer type on methods. */
    /* Stack: -1: newtimer */
    luaL_getmetatable(L, "Timer");

    /* Stack: -1: "Timer", -2: newtimer */
    /* Pop table from the stack and set it as the metatable for t */
    lua_setmetatable(L, -2);

    /* Stack: -1: newtimer */
    return 1;
}

/** @brief Checks that a Timer object is passed. */
static SwTimer *
checktimer(lua_State *L)
{
    void *obj = luaL_checkudata(L, 1, "Timer");
    luaL_argcheck(L, obj != NULL, 1, "Timer object expected");
    return (SwTimer *)obj;
}

static int
timer_tic(lua_State *L)
{
    SwTimer *t = checktimer(L);
    SwTimer_tic(t);
    return 0;
}

static int
timer_toc(lua_State *L)
{
    SwTimer *t = checktimer(L);
    uint32_t toc = (uint32_t)SwTimer_toc(t)/1000;
    lua_pushinteger(L, (lua_Integer)toc);
    return 1;
}

static int
timer_setMs(lua_State *L)
{
    SwTimer *t = checktimer(L);
    lua_Integer ms = luaL_checkinteger(L, 2);
    SwTimer_setMs(t, ms);
    return 0;
}

static int
timer_test(lua_State *L)
{
    SwTimer *t = checktimer(L);
    bool expired = SwTimer_test(t);
    lua_pushboolean(L, expired);
    return 1;
}

static int
sleep_ms(lua_State *L)
{
    lua_Integer ms = luaL_checkinteger(L, 1);
    if (ms < 0)
    {
        LOGPRINT_ERROR("Input arg is not an integer.");
        return 0;
    }
    SwTimer_sleepMs(ms);
    return 0;
}

static int
threadsleep_ms(lua_State *L)
{
    lua_Integer ms = luaL_checkinteger(L, 1);
    if (ms < 0)
    {
        LOGPRINT_ERROR("Input arg is not an integer.");
        return 0;
    }
    RTOS_TASK_SLEEP_ms(ms);
    return 0;
}

static const luaL_Reg timerlib[] = {
    {"new", timer_new},
    {"tic", timer_tic},
    {"toc", timer_toc},
    {"set_ms", timer_setMs},
    {"test", timer_test},
    {"sleep_ms", sleep_ms},
    {"threadsleep_ms", threadsleep_ms},
    {NULL, NULL}
};

/*
** Open timer library
*/
LUAMOD_API int
luaopen_timer(lua_State *L)
{
    /* Create a new metatable for the timer objects. */
    luaL_newmetatable(L, "Timer");
    luaL_newlib(L, timerlib);
    return 1;
}
