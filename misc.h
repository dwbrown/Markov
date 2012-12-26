// FILE: misc.h
//
// DESCRIPTION:
//      This file contains non application-specific types and functions.
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#ifndef MISC_H
#define MISC_H


#include <iostream>
#include <fstream>


typedef unsigned char UByte_t;

typedef signed char SByte_t;

typedef unsigned long BitSet_t;

#define SET_BIT(element) (1<<(element))

#define SET_IN(set, element) (((1<<(element)) & (set)) != 0)




typedef struct
{
    int          myValue;
    const char * myString;
} StrTabElement_t;


// Searches the string table for an entry whose myString field matches theString.
// The table ends with an element whose myString field is 0.
// If found, returns its value, otherwise returns the myValue field of the
// element of the table whose myString field is 0 (typically -1).
int strtab_StringToValue( const StrTabElement_t * theTable,
                          const char * theString );

// Searches the string table for an entry whose myValue field matches theValue.
// The table ends with an element whose myString field is 0.
// If found, returns its string, otherwise returns strtab_Unknown().
const char * strtab_ValueToString( const StrTabElement_t * theTable,
                                   int theValue );

// Returns constant string "(unknown)".
const char * strtab_Unknown();
                                   


// returns true if c is an end-of-line character.
// c was just read from stream in.
// if it is an end-of-line char, checks for 2-char eol sequence "\n\r" or "\r\n"
// and if found, skips the second end-of-line character.
bool IsEndOfLine( std::istream & in,
                  char c );


#endif // MISC_H
