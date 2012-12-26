// FILE:  work_status.cpp
//
// DESCRIPTION:
//      Implements module described in work_status.h
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#include "work_status.h"
#include "misc.h"


static const StrTabElement_t g_WorkStatusNames[] =
{
    { WS_OK,                                "OK" },
    { WS_CONTINUE,                          "CONTINUE" },
    { WS_NO_MATCH,                          "NO_MATCH" },
    { WS_END_OF_FILE,                       "END_OF_FILE" },
    { WS_ERROR_CANT_OPEN_DEBUG_FILE,        "ERROR_CANT_OPEN_DEBUG_FILE" },
    { WS_ERROR_CANT_OPEN_INPUT_FILE,        "ERROR_CANT_OPEN_INPUT_FILE" },
    { WS_ERROR_CANT_OPEN_OUTPUT_FILE,       "ERROR_CANT_OPEN_OUTPUT_FILE" },
    { WS_ERROR_DOESNT_MATCH_EXPECTED,       "ERROR_DOESNT_MATCH_EXPECTED" },
    { WS_ERROR_REPLACE_STR_BAD_WILDCARD,    "ERROR_REPLACE_STR_BAD_WILDCARD" },
    { WS_ERROR_NO_MATCHING_XFORMS,          "ERROR_NO_MATCHING_XFORMS" },
    { WS_ERROR_STACK_EMPTY,                 "ERROR_STACK_EMPTY" },
    { -1,                                   0 }
};



const char * GetWorkStatusStr( WorkStatus_t status )
{
    return strtab_ValueToString( g_WorkStatusNames, status );
}

