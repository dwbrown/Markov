// FILE: work.h
//
// DESCRIPTION:
//      The Work class is responsible for performing transformations
//      on an input string to produce an output string.
//      All the low level data is stored in the WorkData class so
//      Work is simply the high level algorithm.
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#ifndef WORK_H
#define WORK_H


#include "tagged_char.h"
#include "work_status.h"
#include "instr.h"
#include <vector>
#include <bitset>
#include <iostream>
#include <fstream>
#include <stdio.h>


class WorkData;
struct PM_Level;


class Work
{
public:
    Work( const Program & theProgram,
          bool isVerbose,
          bool theDebugToConsole );

    ~Work();

private:
    Work( const Work & theOther );

    const Work & operator = ( const Work & theOther );

public:
    WorkStatus_t DoTransformations(
                    const TaggedString & theInputString,
                    TaggedString & theOutputString,
                    std::ofstream * theDebug = 0 );    // 0 if not debugging

private:
    // This is a quick check to see if a pattern cannot be matched with
    // the from string.  If any characters in thePatternCharsUsed are not
    // in the from string's characters used, returns WS_NO_MATCH, otherwise
    // returns WS_NOT_DONE (in this case, DoPatternMatch should be called to
    // do the actual matching).
    WorkStatus_t QuickCheckPattern(
                    const std::bitset<TAGGED_CHAR_END> & thePatternCharsUsed );

    WorkStatus_t DoReplacement( const TaggedString & theReplacementStr );

    // Does a pattern match, storing the matched substrings in the WorkData.
    // Returns WS_OK if the pattern successfully matched, or
    // WS_NO_MATCH if not successfully matched.
    WorkStatus_t DoPatternMatch( const TaggedString & thePatternStr,
                                 std::ofstream * theDebug = 0 );

    WorkStatus_t DoPatternMatch1();

    WorkStatus_t PlaceFixedFragment( PM_Level & top );

    WorkStatus_t TryToFillGap( PM_Level & top );
    
    // if the wildcard cannot be matched in this sub-gap, returns WS_NO_MATCH,
    // otherwise adds wildcard substring to myWorkData's wildcard occurrences.
    WorkStatus_t CheckAndHandleWildcard( PM_Level & top );

    void PushAdvanceToNextFrag( PM_Level & top );

    void MaybePushNextWildcardAttempt( PM_Level & top );

    void PushNextWildcard( PM_Level & prev );

    // how many wildcards in this series are single-character wildcards?
    int MinWildcardSpan( int pat_from,
                         int pat_to );

    // if any of these are *$% return -1 otherwise return count of ?.
    int MaxWildcardSpan( int pat_from,
                         int pat_to );

    Wildcard_t GetCurWildcardType( PM_Level & top );

private:        // for debug printing
    std::string FromStringChar( int theIx ) const;

    std::string PatStringChar( int theIx ) const;

    void PrintWorkState( std::ostream & out,
                         WorkStatus_t status ) const;

    void DebugPrintInitial( std::ostream & out ) const;

    void DebugPrintTransition( std::ostream & out ) const;

    void DebugPrintFinal( std::ostream & out,
                          WorkStatus_t status ) const;

private:
    void TriggerBreakpoint();

private:
    const Program & myProgram;
    bool myIsVerbose;
    bool myDebugToConsole;
    size_t myPC;                    // current instruction in myProgram
    size_t myUID;                   // unique id of pattern match for debugging
    WorkData & myWorkData;
    std::vector<PM_Level> myStack;
};


#endif // WORK_H
