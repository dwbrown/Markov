// FILE: driver.h
//
// DESCRIPTION:
//      The Driver class is responsible for reading the input file,
//      calling the Work module to perform transformations,
//      then handling the output string.
//      The command line argument CmdMode will specify whether the entire
//      input file is processed at once, or if it is processed line by line,
//      or in unit test mode where it reads a line and process it, then compares
//      result with the next line from the input file, and reports an error
//      if they don't match.
//      If there are errors opening the input or output files or creating the
//      immediate file, writes an error message to cerr as well as returning
//      an error status, so it can specify the name of the file.
//
// HISTORY:
//      14-DEC-12   D.Brown     Created
//      26-DEC-12   D.Brown     Added immediate command mode

#ifndef DRIVER_H
#define DRIVER_H


#include "tagged_char.h"
#include "cmd_line.h"
#include "instr.h"
#include "work_status.h"
#include <vector>
#include <iostream>
#include <fstream>


class Driver
{
public:
    Driver( const CmdLine & theCmdLine,
            const Program & theProgram );

    ~Driver();

private:
    Driver( const Driver & theOther );

    const Driver & operator = ( const Driver & theOther );

public:
    WorkStatus_t Run();

private:
    WorkStatus_t WriteImmediateFile( const char * immediate_filename,
                                     const char * immediate_string );

    WorkStatus_t RunSub( std::istream  & theInputStream,
                         std::ostream  & theOutputStream,
                         CmdMode_t       theCmdMode,
                         bool            theWriteToDebug,
                         bool            isVerbose,
                         bool            theDebugToConsole );


private:
    const CmdLine & myCmdLine;
    const Program & myProgram;
};


#endif // DRIVER_H
