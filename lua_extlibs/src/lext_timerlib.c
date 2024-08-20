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

/** @brief Static timer for the library. */
static SwTimer timer;

static int
timer_tic(lua_State *L)
{
    SwTimer_tic(&timer);
    return 0;
}

static int
timer_toc(lua_State *L)
{
    uint64_t toc = SwTimer_toc(&timer);
    lua_pushinteger(L, (lua_Integer)toc);
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
    RTOS_TASK_SLEEP_ms(ms);
    return 0;
}

static const luaL_Reg timerlib[] = {
    {"tic", timer_tic},
    {"toc", timer_toc},
    {"sleep_ms", sleep_ms},
    {NULL, NULL}
};

/*
** Open timer library
*/
LUAMOD_API int
luaopen_timer(lua_State *L)
{
    luaL_newlib(L, timerlib);
    return 1;
}
