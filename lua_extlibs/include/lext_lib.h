/*******************************************************************************
 *  @file: lext_lib.h
 *   
 *  @brief: Header for lua extension libraries
*******************************************************************************/
#ifndef LEXT_LIB_H
#define LEXT_LIB_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


#define LEXT_LOAD_LIB(L, name, func)     luaL_requiref(L, (name), (func), 1)

#define LEXT_LOAD_TIMER(L)         LEXT_LOAD_LIB(L, "timer", luaopen_timer)

LUAMOD_API int luaopen_timer(lua_State *L);


#endif
