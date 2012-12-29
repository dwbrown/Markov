// FILE: driver.cpp
//
// DESCRIPTION:
//      Implements the module described in driver.h
//
// HISTORY:
//      14-DEC-12   D.Brown     Created
//      26-DEC-12   D.Brown     Added immediate command mode

#include "driver.h"
#include "work.h"
#include "misc.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace std;


#define DEBUG_FILE_EXTENSION ".log"
#define IMMEDIATE_EXTENSION  ".in"

#define BACKSLASH                 '\\'
#define SPECIAL_END_OF_LINE_CHAR '~'



Driver::Driver( const CmdLine & theCmdLine,
                const Program & theProgram ) :
    myCmdLine( theCmdLine ),
    myProgram( theProgram )
{
}


Driver::~Driver()
{
}


// opens the debug log file, whose name is this_pgm_name + ".log"
static WorkStatus_t OpenDebugFile( ofstream & dbg,
                                   const char * this_pgm_name )
{
    string dbg_filename( this_pgm_name );
    dbg_filename.append( DEBUG_FILE_EXTENSION );

    dbg.open( dbg_filename.c_str() );

    return dbg ? WS_OK : WS_ERROR_CANT_OPEN_DEBUG_FILE;
}


// Reads from theInStrm into theInputString.
// Updates theLineNumber at end-of-lines.
// Converts any end-of-line characters to tagged ~.
// If theStopAtEndOfLine then stops reading at the end of the line,
// and doesn't add the end-of-line character to theInputString.
// If theCvtTildaToTaggedTilda is true converts ~ to tagged ~
// If theCvtToTagged is true \ will cause next char to be tagged.
static WorkStatus_t ReadTaggedString( istream & theInStrm,
                                      TaggedString & theInputString,
                                      unsigned & theLineNumber,
                                      bool theStopAtEndOfLine,
                                      bool theCvtTildaToTaggedTilda,
                                      bool theCvtToTagged )
{
    WorkStatus_t status = WS_CONTINUE;
    bool next_is_tagged = false;

    while ( status == WS_CONTINUE )
    {
        char c;
        theInStrm.get( c );

        if ( theInStrm && c == '\t' )       // convert tabs to spaces
        {
            c = ' ';
        }

        if ( !theInStrm )
        {
            status = WS_END_OF_FILE;     // tbd: check for read errors as well as eof
        }
        else if ( IsEndOfLine( theInStrm, c ) )
        {
            theLineNumber++;

            if ( theStopAtEndOfLine )
            {
                status = WS_OK;
            }
            else 
            {
                theInputString.push_back( 
                       ToTaggedChar(SPECIAL_END_OF_LINE_CHAR) );
            }

            next_is_tagged = false;
        }
        else
        {
            TaggedChar_t tc = ToUntaggedChar( c );

            if ( next_is_tagged )
            {
                tc = ToTaggedChar( c );
                next_is_tagged = false;
            }

            if ( theCvtToTagged && tc == '\\' )
            {
                next_is_tagged = true;
            }
            else
            {
                if ( c == SPECIAL_END_OF_LINE_CHAR && 
                     theCvtTildaToTaggedTilda )
                {
                    theInputString.push_back( ToTaggedChar(c) );
                }
                else if ( c >= FIRST_PRINTING_CHAR && c <= LAST_PRINTING_CHAR )
                {
                    theInputString.push_back( tc );
                }
                // else ignore nonprinting chars

                next_is_tagged = false;
            }
        }
    }

    return status;
}



// skip any lines in the input stream which begin with ';'.
// also skips blank lines as long as they don't contain any spaces.
static WorkStatus_t SkipCommentLines( istream & theInputStream,
                                      unsigned & theLineNumber )
{
    WorkStatus_t status = WS_CONTINUE;
    char c;
    bool in_comment = false;

    while ( status == WS_CONTINUE )
    {
        theInputStream.get( c );

        if ( !theInputStream )
        {
            status = WS_END_OF_FILE;
        }
        else if ( IsEndOfLine( theInputStream, c ) )
        {
            theLineNumber++;
            in_comment = false;
        }
        else if ( c == ';' )
        {
            in_comment = true;
        }
        else if ( !in_comment &&
                  c >= FIRST_PRINTING_CHAR && 
                  c <= LAST_PRINTING_CHAR )
        {
            status = WS_OK;
            theInputStream.unget();
        }
    }

    return status;
}


// Writes theString to theOutputStream.
// If theConvertTildaToEoln is true, any tagged ~ characters are converted
// to end-of-lines.  Any tagged characters (other than ~) are prefixed with
// a backspace.  If the string doesn't end with an end-of-line, 
// terminates the line.
static void WriteTaggedString( ostream & theOutputStream,
                               const TaggedString & theString,
                               bool theConvertTildaToEoln )
{
    size_t len = theString.size();
    char c = 0;

    for ( size_t i = 0; i < len; i++ )
    {
        TaggedChar_t tc = theString[i];
        c = ToUntaggedChar( tc );

        if ( c == '\n' || 
             ( c == SPECIAL_END_OF_LINE_CHAR && IsTagged(tc) && 
               theConvertTildaToEoln ) )
        {
            theOutputStream << endl;
            c = '\n';
        }
        else
        {
            if ( c != SPECIAL_END_OF_LINE_CHAR && IsTagged( tc ) )
            {
                theOutputStream.put( BACKSLASH );
            }

            theOutputStream.put( c );
        }
    }

    if ( c != '\n' )
    {
        theOutputStream << endl;
    }
}


// compares the two strings, and if they are the same returns WS_OK
// otherwise returns WS_ERROR_DOESNT_MATCH_EXPECTED
static WorkStatus_t CompareWithExpected( TaggedString & output_string,
                                         TaggedString & expected_string )
{
    WorkStatus_t status = WS_OK;

    size_t outlen = output_string.size();

    if ( outlen != expected_string.size() )
    {
        status = WS_ERROR_DOESNT_MATCH_EXPECTED;
    }
    else
    {
        for ( size_t i = 0; i < outlen && status == WS_OK; i++ )
        {
            if ( output_string[i] != expected_string[i] )
            {
                status = WS_ERROR_DOESNT_MATCH_EXPECTED;
            }
        }
    }

    return status;
}


static void DebugWriteInputString( ostream & theStream,
                                   const TaggedString & theInputString,
                                   unsigned theLineNumber )
{
    theStream << "## Input String, line " << theLineNumber << ", length " <<
              theInputString.size() << ":" << endl;
    WriteTaggedString( theStream, theInputString, false );
}



static void DebugWriteOutputString( ostream & theStream,
                                    const TaggedString & theOutputString )
{
    theStream << "## Output String, length " << theOutputString.size() << 
                 ":" << endl;
    WriteTaggedString( theStream, theOutputString, false );
}



// writes a debug trace for output string doesn't match expected to theStream.
static void DebugDoesntMatchExpected( ostream & theStream, 
                                      const TaggedString & theInputString,
                                      const TaggedString & theOutputString,
                                      const TaggedString & theExpectedString,
                                      unsigned input_line_number )
{
    theStream << "#####" << endl;
    theStream << "##### Output String doesn't match expected value at line " <<
                 input_line_number << endl;

    theStream << "##### Last Input String, length " << theInputString.size() << 
                 ":" << endl;
    WriteTaggedString( theStream, theInputString, false );
    theStream << "##### Last Output String, length " << theOutputString.size() <<
                 ":" << endl;
    WriteTaggedString( theStream, theOutputString, false );
    theStream << "##### Expected String, length " << theExpectedString.size() <<
                 ":" << endl;
    WriteTaggedString( theStream, theExpectedString, false );
    theStream << "#####" << endl;
}


// This performs the high level work of the driver, reading from
// the input file, performing transformations and writing the results.
// It works slightly differently in the different modes.
WorkStatus_t Driver::RunSub( istream  & theInputStream,
                             ostream  & theOutputStream,
                             CmdMode_t  theCmdMode,
                             bool       theWriteToDebug,
                             bool       isVerbose,
                             bool       theDebugToConsole )
{
    WorkStatus_t status = WS_OK;
    unsigned line_number = 1;
    ofstream dbg;
    ofstream * dbg_ptr = 0;

    if ( theWriteToDebug )
    {
        status = OpenDebugFile( dbg, myCmdLine.ThisProgramName() );
        dbg_ptr = status == WS_OK ? &dbg : 0;
    }

    while ( status == WS_OK && theInputStream )
    {
        TaggedString input_string;

        if ( theCmdMode == CMDMODE_UNIT_TEST )
        {
            status = SkipCommentLines( theInputStream, line_number );

            if ( status == WS_END_OF_FILE )
            {
                break;
            }
        }

        unsigned saved_line_number = line_number;

        status = ReadTaggedString( theInputStream, input_string, 
                                   line_number, 
                                   theCmdMode == CMDMODE_UNIT_TEST,
                                   theCmdMode != CMDMODE_FULL_FILE,
                                   theCmdMode == CMDMODE_UNIT_TEST );

        if ( dbg_ptr != 0 )
        {
            DebugWriteInputString( dbg, input_string, line_number );
        }

        if ( theCmdMode == CMDMODE_UNIT_TEST )
        {
            DebugWriteInputString( theOutputStream, input_string, line_number );
        }

        TaggedString output_string;

        Work work( myProgram, isVerbose, theDebugToConsole );

        status = work.DoTransformations( input_string, output_string, 
                                         dbg_ptr );

        if ( dbg_ptr != 0 )
        {
            DebugWriteOutputString( dbg, output_string );
        }

        if ( theCmdMode == CMDMODE_UNIT_TEST )
        {
            DebugWriteOutputString( theOutputStream, output_string );
        }
        else
        {
            WriteTaggedString( theOutputStream, output_string, true );
        }

        if ( status == WS_OK && theCmdMode == CMDMODE_UNIT_TEST )
        {
            TaggedString expected;

            status = ReadTaggedString( theInputStream, expected, 
                                       line_number, true, true, true );

            if ( status == WS_OK )
            {
                status = CompareWithExpected( output_string, expected );

                if ( status != WS_OK )
                {
                    if ( dbg_ptr != 0 )
                    {
                        DebugDoesntMatchExpected( dbg, input_string,
                                                  output_string, expected,
                                                  saved_line_number );
                    }

                    DebugDoesntMatchExpected( theOutputStream, input_string,
                                              output_string, expected,
                                              saved_line_number );
                }
            }
        }
    }

    if ( status == WS_END_OF_FILE )
    {
        status = WS_OK;
    }

    if ( theCmdMode == CMDMODE_UNIT_TEST )
    {
        const char * errmsg = status <= WS_END_OF_FILE ? "" :
                     GetWorkStatusStr(status);

        theOutputStream << endl <<
            "#### Unit tests completed " << 
            (status == WS_OK ? "OK" : "with error") <<
            " " << errmsg << endl;

        if ( dbg_ptr != 0 )
        {
            dbg << endl << 
            "#### Unit tests completed " << 
            (status == WS_OK ? "OK" : "with error") <<
            " " << errmsg << endl;
        }
    }

    // tbd: check for theInputStream errors

    return status;
}


// Creates a new file with name immediate_filename which contains
// the contents immediate_string.
WorkStatus_t Driver::WriteImmediateFile( const char * immediate_filename,
                                         const char * immediate_string )
{
    WorkStatus_t status = WS_OK;
    ofstream immout( immediate_filename );

    if ( !immout )
    {
        status = WS_ERROR_CANT_CREATE_IMMEDIATE_FILE;
        cerr << "ERROR: Unable to create immediate file " <<
                immediate_filename << endl;
    }
    else
    {
        immout << immediate_string;
    }

    return status;
}


// this opens the input, output and debug files then calls RunSub
WorkStatus_t Driver::Run()
{
    WorkStatus_t status = WS_OK;
    string input_filename( myCmdLine.InputFileName() );

    if ( myCmdLine.CmdMode() == CMDMODE_IMMEDIATE )
    {
        input_filename = myCmdLine.ThisProgramName();
        input_filename.append( IMMEDIATE_EXTENSION );

        status = WriteImmediateFile( input_filename.c_str(),
                                     myCmdLine.InputFileName() );
    }

    ifstream in;

    if ( status == WS_OK && !myCmdLine.ReadFromStdin() )
    {
        in.open( input_filename.c_str() );

        if ( !in )
        {
            status = WS_ERROR_CANT_OPEN_INPUT_FILE;
            cerr << "ERROR: Unable to open input file " <<
                     input_filename << endl;
        }
    }

    ofstream out;

    if ( status == WS_OK && !myCmdLine.WriteToStdout() )
    {
        out.open( myCmdLine.OutputFileName() );
        
        if ( !out )
        {
            status = WS_ERROR_CANT_OPEN_OUTPUT_FILE;
            cerr << "ERROR: Unable to open output file " <<
                    myCmdLine.OutputFileName();
        }
    }

    if ( status == WS_OK )
    {
        status = RunSub( myCmdLine.ReadFromStdin() ? cin : in, 
                         myCmdLine.WriteToStdout() ? cout : out, 
                         myCmdLine.CmdMode(),
                         myCmdLine.WriteToDebug(),
                         SET_IN(myCmdLine.CmdFlags(), CMDFLGS_VERBOSE),
                         SET_IN(myCmdLine.CmdFlags(), CMDFLGS_CONSOLE) ); 
    }

    if ( status == WS_END_OF_FILE )
    {
        status = WS_OK;
    }

    return status;
}

