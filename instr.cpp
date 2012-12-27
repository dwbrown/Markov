// FILE: instr.cpp
//
// DESCRIPTION:
//      Implements the module described in instr.h
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#include "instr.h"
#include "tagged_char.h"
#include "misc.h"
#include <vector>
#include <bitset>
#include <iostream>
#include <fstream>
#include <iomanip>


using namespace std;

#define BACKSLASH       '\\'
#define BEGIN_COMMENT   ';'


static const char * g_TransitionStr = "->";



Instr::Instr() :
    myLineNumber( 0 )
{
}



Instr::~Instr()
{
}



Instr::Instr( const Instr & theOther )
{
    *this = theOther;
}



const Instr & Instr::operator = ( const Instr & theOther )
{
    myLineNumber = theOther.myLineNumber;

    myPattern = theOther.myPattern;
    myPatternCharsUsed = theOther.myPatternCharsUsed;

    myReplacement = theOther.myReplacement;

    return *this;
}



void Instr::SetLineNumber( unsigned theLineNumber )
{
    myLineNumber = theLineNumber;
}



unsigned Instr::GetLineNumber() const
{
    return myLineNumber;
}



const TaggedString & Instr::GetPatternStr() const
{
    return myPattern;
}



// Set myPattern to ts
// Also set myPatternCharsUsed to all chars in ts other than wildcard chars.
void Instr::PutPatternStr( const TaggedString & ts )
{
    myPattern = ts;

    myPatternCharsUsed.reset();

    size_t len = ts.size();

    for ( size_t i = 0; i < len; i++ )
    {
        TaggedChar_t tc = ts[i];

        if ( !IsWildcard(tc) )
        {
            myPatternCharsUsed.set( tc );
        }
    }
}



const TaggedString & Instr::GetReplacementStr() const
{
    return myReplacement;
}



void Instr::PutReplacementStr( const TaggedString & ts )
{
    myReplacement = ts;
}



const bitset<TAGGED_CHAR_END> & Instr::GetPatternCharsUsed() const
{
    return myPatternCharsUsed;
}



void Instr::Print( ostream & out,
                   const char * margin ) const
{
    out << margin << setw(6) <<  myLineNumber << ": ";
    PrintTaggedString( out, myPattern );
    out << " " << g_TransitionStr << " ";
    PrintTaggedString( out, myReplacement );
    out << endl;
}


static char ReadAndSkipWhiteSpace( ifstream & thePgmFile,
                                  unsigned & theLineNumber )
{
    char c;
    bool in_comment = false;
    bool done = false;
    
    while ( !done )
    {
        thePgmFile.get( c );

        if ( !thePgmFile )
        {
            done = true;
        }
        else if ( IsEndOfLine(thePgmFile, c) )
        {
            theLineNumber++;
            in_comment = false;
        }
        else if ( in_comment )
        {
            // ignore char
        }
        else if ( c == BEGIN_COMMENT )
        {
            in_comment = true;
        }
        else if ( c == ' ' )
        {
            // blank is the only printing char which is white space
        }
        else if ( c >= FIRST_PRINTING_CHAR && c <= LAST_PRINTING_CHAR )
        {
            done = true;
        }
    }

    return c;
}



// reads a string beginning with delimiter theDelimiter
// and stores it in theStr.
// returns true = ok, false = error
static bool ReadTaggedString( ifstream & thePgmFile,
                              TaggedString & theStr,
                              unsigned & theLineNumber,
                              char theDelimiter )
{
    theStr.clear();

    char c;
    bool ok = true;
    bool done = false;
    bool got_backslash = false;

    while ( ok && !done )
    {
        thePgmFile.get( c );

        if ( thePgmFile && c == '\t' )
        {
            c = ' ';
        }

        if ( !thePgmFile || IsEndOfLine(thePgmFile, c) )
        {
            ok = false;
        }
        else if ( c == theDelimiter )
        {
            ok = !got_backslash;
            done = true;
        }
        else if ( !got_backslash && c == BACKSLASH )
        {
            got_backslash = true;
        }
        else if ( c >= FIRST_PRINTING_CHAR && c <= LAST_PRINTING_CHAR )
        {
            if ( got_backslash )
            {
                theStr.push_back( ToUntaggedChar( c ) );
                got_backslash = false;
            }
            else
            {
                theStr.push_back( ToTaggedChar( c ) );
            }
        }
        // else discard nonprinting char
    }

    return ok;
}



// Reads an instruction: <pattern_string> '->' <replacement_string>
// Returns true if ok, false if error (end-of-file returns true).
// On error, also prints an error msg to stderr
static bool ReadAndAppendInstr( Program & theProgram,
                                ifstream & thePgmFile,
                                unsigned & theLineNumber,
                                const char * theFileName )
{
    bool done = false;
    bool ok = true;
    size_t ix = theProgram.size();
    TaggedString temp_str;
    const char * errinfo = "Skipping white space before pattern";

    char c = ReadAndSkipWhiteSpace( thePgmFile, theLineNumber );

    if ( !thePgmFile )
    {
        done = true;
    }
    else
    {
        Instr temp;
        temp.SetLineNumber( theLineNumber );
        theProgram.push_back( temp );

        errinfo = "Reading pattern string";

        ok = ReadTaggedString( thePgmFile, temp_str, theLineNumber, c );

        if ( ok )
        {
            theProgram[ix].PutPatternStr( temp_str );
        }
    }

    if ( !done && ok )
    {
        errinfo = "Reading transition operator";

        c = ReadAndSkipWhiteSpace( thePgmFile, theLineNumber );

        if ( !thePgmFile )
        {
            ok = false;
        }
        else if ( c == g_TransitionStr[0] )
        {
            for ( int i = 1; g_TransitionStr[i] != 0 && ok; i++ )
            {
                thePgmFile.get( c );

                ok = (thePgmFile && c == g_TransitionStr[i]);
            }
        }
    }

    if ( !done && ok )
    {
        errinfo = "Skipping to replacement string";

        c = ReadAndSkipWhiteSpace( thePgmFile, theLineNumber );

        if ( !thePgmFile )
        {
            ok = false;
        }
    }

    if ( !done && ok )
    {
        errinfo = "Reading replacement string";

        ok = ReadTaggedString( thePgmFile, temp_str, theLineNumber, c );

        if ( ok )
        {
            theProgram[ix].PutReplacementStr( temp_str );
        }
    }

    if ( !ok )
    {
        cerr << "ERROR: Syntax error at line " << theLineNumber << 
                " of program file" << endl << 
                ". " << theFileName << endl <<
                ". " << errinfo << endl;
    }

    return ok;
}



bool ReadProgram( Program & theProgram,
                  const char * theProgramFileName )
{
    bool ok = true;

    ifstream in( theProgramFileName );

    if ( !in )
    {
        cerr << "ERROR: Unable to open program file '" << 
                 theProgramFileName << "'" << endl;
        ok = false;
    }
    else
    {   
        unsigned line_number = 1;

        while ( ok && in )
        {
            ok = ReadAndAppendInstr( theProgram, in, line_number,
                                     theProgramFileName );
        }
    }

    return ok;
}


static void PrintProgram1( ostream & out,
                           const Program & theProgram,
                           const char * thisProgramName )
{
    out << thisProgramName << " program:" << endl;

    size_t len = theProgram.size();

    for ( size_t i = 0; i < len; i++ )
    {
        theProgram[i].Print( out, "   " );
    }

    out << "end" << endl;
}



void PrintProgram( const Program & theProgram,
                   const char * thisProgramName,
                   const char * theOutFileName )
{
    if ( theOutFileName == 0 )
    {
        PrintProgram1( cout, theProgram, thisProgramName );
    }
    else
    {
        ofstream out( theOutFileName );

        if ( !out )
        {
            cerr << "ERROR: Unable to open program file '" <<
                     theOutFileName << "'" << endl;
        }
        else
        {
            PrintProgram1( out, theProgram, thisProgramName );
        }
    }
}
