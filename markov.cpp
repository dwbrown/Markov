// Markov.cpp : Defines the entry point for the console application.
//


#include "misc.h"
#include "cmd_line.h"
#include "instr.h"
#include "driver.h"
#include "work_status.h"
#include <vector>

using namespace std;


#define EXIT_OK    0
#define EXIT_ERROR 100



int main(int argc, char* argv[])
{
    CmdLine cmd_line;

    if ( !cmd_line.ProcessArguments( argc, argv ) )
    {
        cmd_line.DoPrintHelp( cerr );
        return EXIT_ERROR;
    }
    else if ( SET_IN(cmd_line.CmdFlags(), CMDFLGS_HELP) )
    {
        cmd_line.DoPrintHelp( cout );
        return EXIT_OK;
    }
    else if ( SET_IN(cmd_line.CmdFlags(), CMDFLGS_OPTIONS) )
    {
        cmd_line.DoPrintOptions( cout );
        return EXIT_OK;
    }

    vector<Instr> program;

    bool ok = ReadProgram( program, cmd_line.ProgramFileName() );

    if ( ok && SET_IN(cmd_line.CmdFlags(), CMDFLGS_PRINT) )
    {
        PrintProgram( program, cmd_line.ThisProgramName(),
                      cmd_line.WriteToStdout() ? 
                         0 : cmd_line.OutputFileName() );
        return EXIT_OK;
    }

    if ( ok )
    {
        Driver driver( cmd_line, program );

        WorkStatus_t ws = driver.Run();

        if ( ws != WS_OK )
        {
            ok = false;

            cerr << "Driver returns error " <<
                     GetWorkStatusStr(ws) << endl;
        }
    }

    return ok ? EXIT_OK : EXIT_ERROR;
}
