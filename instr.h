// FILE instr.h
//
// DESCRIPTION:
//      The Instr class contains a single transformation from the program.
//      This file also defines a Program as a vector of Instr.
//
//      This module is also responsible for reading the program
//      file and creating the vector of Instr objects, and for
//      printing a program.
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#ifndef INSTR_H
#define INSTR_H

#include "tagged_char.h"
#include <vector>
#include <bitset>
#include <iostream>
#include <stdio.h>



class Instr
{
public:
    Instr();

    ~Instr();

    Instr( const Instr & theOther );

    const Instr & operator = ( const Instr & theOther );

    void SetLineNumber( unsigned theLineNumber );

    unsigned GetLineNumber() const;

    const TaggedString & GetPatternStr() const;

    void PutPatternStr( const TaggedString & ts );

    const TaggedString & GetReplacementStr() const;

    void PutReplacementStr( const TaggedString & ts );

    const std::bitset<TAGGED_CHAR_END> & GetPatternCharsUsed() const;

    void Print( std::ostream & out,
                const char * margin = "" ) const;

private:
    unsigned myLineNumber;                                  // line # in program file

    TaggedString myPattern;
    std::bitset<TAGGED_CHAR_END> myPatternCharsUsed;        // excludes wildcards

    TaggedString myReplacement;
};



typedef std::vector<Instr> Program;



// reads a program, storing the instructions in theProgram.
// returns true = ok, false = error.
bool ReadProgram( Program & theProgram,
                  const char * theProgramFileName );


// write the program to a file, or if theOutFileName = 0, to stdout
void PrintProgram( const Program & theProgram,
                   const char * thisProgramName,
                   const char * theOutFileName = 0 );

#endif // INSTR_H
