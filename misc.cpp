// FILE: misc.cpp
//
// DESCRIPTION:
//      Implements the module described in misc.h
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#include "misc.h"
#include <iostream>
#include <fstream>
#include <string.h>

using namespace std;


static const char * g_Unknown = "(unknown)";


int strtab_StringToValue( const StrTabElement_t * theTable,
                          const char * theString )
{
    while ( theTable->myString != 0 )
    {
        if ( strcmp( theTable->myString, theString ) == 0 )
        {
            return theTable->myValue;
        }

        theTable++;
    }

    return theTable->myValue;
}



const char * strtab_ValueToString( const StrTabElement_t * theTable,
                                   int theValue )
{
    while ( theTable->myString != 0 )
    {
        if ( theTable->myValue == theValue )
        {
            return theTable->myString;
        }
        
        theTable++;
    }

    return strtab_Unknown();
}



// Returns constant string "(unknown)".
const char * strtab_Unknown()
{
    return g_Unknown;
}



bool IsEndOfLine( istream & in,
                  char c )
{
    char next = 0;

    if ( c == '\n' )
    {
        next = '\r';
    }
    else if ( c == '\r' )
    {
        next = '\n';
    }

    if ( next != 0 )
    {
        in.get( c );

        if ( in && c != next )
        {
            in.unget();
        }
    }

    return next != 0;
}


