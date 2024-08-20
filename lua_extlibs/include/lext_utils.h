/*******************************************************************************
 *  @file: lext_utils.h
 *   
 *  @brief: Header for common lua extension utilities.
*******************************************************************************/
#ifndef LEXT_UTILS_H
#define LEXT_UTILS_H

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"


/******************************************************************************
    [docexport lext_printstack]
*//**
    @brief Prints the current stack.
******************************************************************************/
void
lext_printstack(lua_State *L);
#endif
