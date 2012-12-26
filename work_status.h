// FILE:  work_status.h
//
// DESCRIPTION:
//      This file defines enum WorkStatus_t which specifies an error code
//      or one of several kinds of OK status.
//      It defines function GetWorkStatusStr which returns the error
//      message string associated with a WorkStatus_t element.
//
// HISTORY:
//      14-DEC-12   D.Brown     Created
//      26-DEC-12   D.Brown     Added WS_ERROR_CANT_CREATE_IMMEDIATE_FILE

#ifndef WORK_STATUS_H
#define WORK_STATUS_H


enum WorkStatus_t
{
    WS_OK,                              // completed with no errors
    WS_CONTINUE,                        // no errors but not complete yet
    WS_NO_MATCH,                        // this transform didn't match
    WS_END_OF_FILE,                     // end of input file
    WS_ERROR_CANT_OPEN_DEBUG_FILE,      // unable to open debug log file
    WS_ERROR_CANT_OPEN_INPUT_FILE,      // unable to open input file
    WS_ERROR_CANT_CREATE_IMMEDIATE_FILE,// unable to create the immediate file
    WS_ERROR_CANT_OPEN_OUTPUT_FILE,     // unable to open output file
    WS_ERROR_DOESNT_MATCH_EXPECTED,     // doesn't match the expected string
    WS_ERROR_REPLACE_STR_BAD_WILDCARD,  // replacement string contains a wildcard not
                                        // defined in the corresponding pattern
    WS_ERROR_NO_MATCHING_XFORMS,        // no matching xforms found in program
    WS_ERROR_START_STEP_NO_MATCH,       // start step must succeed
    WS_ERROR_STACK_EMPTY,               // stack is unexpectedly empty

    WS_END
};


const char * GetWorkStatusStr( WorkStatus_t status );


#endif // WORK_STATUS_H
