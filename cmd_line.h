// FILE:  cmd_line.h
//
// DESCRIPTION:
//      Defines class CmdLine which parses the command line and
//      is a repository for the command line arguments.
//      Defines helper enums:
//          CmdLineFlags_t  - Flags: bits of CmdLine.CmdFlags()
//          CmdMode_t       - Operation mode:
//                            - full file mode reads entire file,
//                            - single line mode reads and processes
//                              each line separately,
//                            - unit test mode reads and processes
//                              each line and compares it to the
//                              next line.
//          FileId_t        - Index for accessing the file name strings
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#ifndef CMD_LINE_H
#define CMD_LINE_H

#include "misc.h"
#include <iostream>
#include <stdio.h>


#define MAX_FILENAME 256


enum CmdLineFlags_t
{
    CMDFLGS_SINGLE,     // -1 : single line mode
    CMDFLGS_TEST,       // -test or -t : unit test mode
    CMDFLGS_DEBUG,      // -debug or -d : debug info to markov.log
    CMDFLGS_VERBOSE,    // -verbose or -v : verbose debug mode
    CMDFLGS_CONSOLE,    // -console -r -c : debug & verbose info to console
    CMDFLGS_PRINT,      // -print : print program
    CMDFLGS_OPTIONS,    // -options : print options
    CMDFLGS_HELP,       // -help or -h or ? : help mode

    CMDFLGS_END
};


enum CmdMode_t
{
    CMDMODE_FULL_FILE,          // read entire file then do transformation, output
    CMDMODE_SINGLE_LINE,        // read line from stdin, transform, output, loop
    CMDMODE_UNIT_TEST,          // read line, transform, compare with next line, loop

    CMDMODE_END
};


enum FileNameId_t
{
    FNID_PROGRAM_FILE,          // contains Markov program
    FNID_INPUT_FILE,            // input string
    FNID_OUTPUT_FILE,           // output string

    FNID_END
};



class CmdLine
{
public:
    CmdLine();

private:
    CmdLine( const CmdLine & theOther );

    CmdLine & operator = ( const CmdLine & theOther );

public:
    bool ProcessArguments( int argc,
                           char * argv[] );

    CmdMode_t CmdMode() const;

    BitSet_t CmdFlags() const;

    const char * ThisProgramName() const;

    const char * ProgramFileName() const;

    const char * InputFileName() const;

    const char * OutputFileName() const;

    bool ReadFromStdin() const;

    bool WriteToStdout() const;

    bool WriteToDebug() const;

    void DoPrintHelp( std::ostream & theOutputFile ) const;

    void DoPrintOptions( std::ostream & theOutputFile ) const;

private:
    bool PostProcessArguments();
    
private:
    CmdMode_t    myCmdMode;
    BitSet_t     myFlags;
    const char * myFilenames[FNID_END];
};


#endif // CMD_LINE_H
