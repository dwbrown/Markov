// FILE: cmd_line.cpp
//
// DESCRIPTION:
//      Implements the module described in cmd_line.h
//
// HISTORY:
//      14-DEC-12   D.Brown     Created
//      26-DEC-12   D.Brown     Added immediate command mode

#include "cmd_line.h"
#include "misc.h"
#include <string.h>


using namespace std;


const char * g_ProgramNameStr = "markov";

static const char * g_StdInName  = "<stdin>";
static const char * g_StdOutName = "<stdout>";
static const char * g_NoneStr    = "<none>";


static const StrTabElement_t g_OptionNames[] =
{
    { CMDFLGS_TEST,       "-test" },    // unit test mode
    { CMDFLGS_TEST,       "-t" },       // 
    { CMDFLGS_IMMEDIATE,  "-imm" },     // immediate mode 
    { CMDFLGS_IMMEDIATE,  "-i" },       // 
    { CMDFLGS_DEBUG,      "-debug" },   // debug mode
    { CMDFLGS_DEBUG,      "-d" },       // 
    { CMDFLGS_VERBOSE,    "-verbose" }, // verbose debug mode
    { CMDFLGS_VERBOSE,    "-v" },       // 
    { CMDFLGS_CONSOLE,    "-console" }, // debug to console
    { CMDFLGS_CONSOLE,    "-c" },       // 
    { CMDFLGS_PRINT,      "-print" },   // print program
    { CMDFLGS_PRINT,      "-p" },       // 
    { CMDFLGS_OPTIONS,    "-options" }, // print options
    { CMDFLGS_HELP,       "-help" },    // help mode
    { CMDFLGS_HELP,       "-h" },       // 
    { CMDFLGS_HELP,       "?" },        // 
    { -1,                 0 }
};



static const StrTabElement_t g_CmdModeNames[] =
{
    { CMDMODE_FULL_FILE,        "FULL_FILE" },
    { CMDMODE_UNIT_TEST,        "UNIT_TEST" },
    { -1,                       0 }
};



inline const char * YES_OR_NO( bool flag )
{
    return flag ? "YES" : "NO";
}



CmdLine::CmdLine() :
  myCmdMode(CMDMODE_FULL_FILE),
  myFlags(0)
{
    memset( myFilenames, 0, sizeof(myFilenames) );
}



// verify argument compatibility and set myCmdMode to correct mode
bool CmdLine::PostProcessArguments()
{
    myCmdMode = CMDMODE_FULL_FILE;
    int num_mode_flags = 0;

    if ( SET_IN( myFlags, CMDFLGS_IMMEDIATE ) )
    {
        myCmdMode = CMDMODE_IMMEDIATE;
        num_mode_flags++;
    }

    if ( SET_IN( myFlags, CMDFLGS_TEST ) )
    {
        myCmdMode = CMDMODE_UNIT_TEST;
        num_mode_flags++;
    }

    if ( num_mode_flags > 1 )
    {
        fprintf( stderr, "ERROR: Incompatible command line flags\n" );
        return false;
    }

    if ( !SET_IN( myFlags, CMDFLGS_HELP ) &&
         !SET_IN( myFlags, CMDFLGS_OPTIONS ) )
    {
        if ( myFilenames[FNID_PROGRAM_FILE] == 0 )
        {
            fprintf( stderr, "ERROR: Progam name is required\n" );
            return false;
        }
        else if ( SET_IN(myFlags, CMDFLGS_IMMEDIATE) &&
                  myFilenames[FNID_INPUT_FILE] == 0 )
        {
            fprintf( stderr, "ERROR: -i option requires input string\n" );
        }
    }

    return true;
}



bool CmdLine::ProcessArguments( int argc,
                                char * argv[] )
{
    int fnid = 0;
    int flagid;

    for ( int i = 1; i < argc; i++ )
    {
        const char * a = argv[i];

        if ( *a == '-' || *a == '?' )
        {
            flagid = strtab_StringToValue( g_OptionNames, a );

            if ( flagid >= 0 )
            {
                myFlags |= SET_BIT( flagid );
            }
            else
            {
                fprintf( stderr, "ERROR: Invalid command option: %s\n", a );
                return false;
            }
        }
        else if ( fnid < FNID_END )
        {
            myFilenames[fnid++] = a;
        }
        else
        {
            fprintf( stderr, "ERROR: Too many filenames in argument list\n" );
            return false;
        }
    }

    return PostProcessArguments();
}



CmdMode_t CmdLine::CmdMode() const
{
    return myCmdMode;
}



BitSet_t CmdLine::CmdFlags() const
{
    return myFlags;
}



const char * CmdLine::ThisProgramName() const
{
    return g_ProgramNameStr;
}



const char * CmdLine::ProgramFileName() const
{
    if ( myFilenames[FNID_PROGRAM_FILE] == 0 )
    {
        return g_NoneStr;
    }
    else
    {
        return myFilenames[FNID_PROGRAM_FILE];
    }
}



const char * CmdLine::InputFileName() const
{
    if ( ReadFromStdin() )
    {
        return g_StdInName;
    }
    else
    {
        return myFilenames[FNID_INPUT_FILE];
    }
}



const char * CmdLine::OutputFileName() const
{
    if ( WriteToStdout() )
    {
        return g_StdOutName;
    }
    else
    {
        return myFilenames[FNID_OUTPUT_FILE];
    }
}



bool CmdLine::ReadFromStdin() const
{
    return myFilenames[FNID_INPUT_FILE] == 0;
}



bool CmdLine::WriteToStdout() const
{
    return myFilenames[FNID_OUTPUT_FILE] == 0;
}


bool CmdLine::WriteToDebug() const
{
    return SET_IN( myFlags, CMDFLGS_DEBUG );
}



void CmdLine::DoPrintHelp( ostream & outfile ) const
{
    outfile << "Syntax: " << ThisProgramName() << 
               " <options> <program_file_name> <input_file_name>" << endl;
    outfile << "        <output_file_name>" << endl << endl;
    outfile << "filenames can be ommitted from the right if not needed." << 
               endl << endl;
    outfile << 
        "options are 0 or more of: (can abbreviate to 1 char after the dash)"
        << endl;
    outfile << "     -1           - single line mode" << endl;
    outfile << "     -test        - unit test mode" << endl;
    outfile << "     -debug       - debug mode" << endl;
    outfile << "     -print       - print program (with line numbers)" << endl;
    outfile << "     -options     - print parsed cmd line args" << endl << endl;
    outfile << "     -help        - print help text" << endl;
    outfile << "     '?'          - print help text" << endl;
}



void CmdLine::DoPrintOptions( ostream & outfile ) const
{
    outfile << g_ProgramNameStr << " Cmd Line Arguments :" << endl;
    outfile << "    program_file_name :  " << ProgramFileName()  << endl;
    outfile << "    input_file_name :    " << InputFileName() << endl;
    outfile << "    output_file_name :   " << OutputFileName() << endl << endl;

    outfile << "Mode :                   " <<
             strtab_ValueToString( g_CmdModeNames, CmdMode() ) << endl << endl;

    outfile << "Flags:" << endl;
    outfile << "    -debug :             " <<
             YES_OR_NO(WriteToDebug()) << endl;
    outfile << "    -print :             " <<
             YES_OR_NO(SET_IN(CmdFlags(), CMDFLGS_PRINT)) << endl;
    outfile << "    -options :           " <<
             YES_OR_NO(SET_IN(CmdFlags(), CMDFLGS_OPTIONS)) << endl;
    outfile << "    -help :              " <<
             YES_OR_NO(SET_IN(CmdFlags(), CMDFLGS_HELP)) << endl;
}
