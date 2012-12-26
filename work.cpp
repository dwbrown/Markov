// FILE:    work.cpp
//
// DESCRIPTION:
//      Implements module described in work.h
//
// HISTORY:
//      14-DEC-12   D.Brown     Created
//      26-DEC-12   D.Brown     If start xform fails then error

#include "work.h"
#include "work_data.h"
#include "instr.h"
#include <assert.h>


using namespace std;


static const size_t START_STEP = 0;     // first step in program
static const size_t EXIT_STEP  = 1;     // second step in program


// set this to myUID value to break at
// or set to 0 for no debug breaking
static const size_t BREAK_AT_UID = 0;  


// this struct contains everything needed for pattern matching on a
// single level.  The Work class will contain a stack of PM_Level_t.
// There will be one PM_Level_t for each of the wildcards in the gap
// between fixed strings, plus one PM_Level_t for the fixed string itself.
struct PM_Level
{
    size_t    myFragIx;         // current fixed fragment
    int       myPatWildIx;      // points to current wildcard in pattern
    int       myPatFixedIx;     // points to start of fixed part in pattern
    int       myFsLeftIx;       // leftmost matched char or -1
    int       myFsWildIx;       // points start of wildcard part in from string
    int       myFsWildEndIx;    // points to max loc of wildcard part in from string
    int       myFsFixedIx;      // points to start of fixed part in from string
    bool      myFixedIsMatched; // myFsFixedIx points to valid matched string
};




Work::Work( const Program & theProgram,
            bool isVerbose,
            bool theDebugToConsole ) :
    myProgram( theProgram ),
    myIsVerbose( isVerbose ),
    myDebugToConsole( theDebugToConsole ),
    myPC( 0 ),
    myUID( 1 ),
    myWorkData( *new WorkData() )
{
}



Work::~Work()
{
    delete &myWorkData;
}



WorkStatus_t Work::DoTransformations(
                         const TaggedString & theInputString,
                         TaggedString & theOutputString,
                         ofstream * theDebug )
{
    myWorkData.ClearToString();
    myWorkData.AppendStringToToString( theInputString );
    myWorkData.MoveToStringToFromString();

    if ( myDebugToConsole )
    {
        DebugPrintInitial( cout );
    }

    if ( theDebug != 0 )
    {
        DebugPrintInitial( *theDebug );
    }

    size_t pgm_size = myProgram.size();
    myPC = START_STEP;
    WorkStatus_t status = WS_CONTINUE;
    bool found_any = false;

    while ( status == WS_CONTINUE )
    {
        if ( myPC == pgm_size )
        {
            if ( found_any )
            {
                myPC = EXIT_STEP;
                found_any = false;
            }
            else
            {
                status = WS_ERROR_NO_MATCHING_XFORMS;
            }
        }
        else
        {
            status = QuickCheckPattern( myProgram[myPC].GetPatternCharsUsed() );

            if ( status == WS_CONTINUE )
            {
                myWorkData.SetCurrentPattern( 
                                myProgram[myPC].GetPatternStr() );

                status = DoPatternMatch( myProgram[myPC].GetPatternStr(),
                                         theDebug );

                if ( status == WS_OK )
                {
                    status = DoReplacement( 
                                    myProgram[myPC].GetReplacementStr() );

                    found_any = true;

                    if ( status == WS_CONTINUE )
                    {
                        if ( myDebugToConsole )
                        {
                            DebugPrintTransition(cout);
                        }

                        if ( theDebug != 0 )
                        {
                            DebugPrintTransition(*theDebug);
                        }

                        if ( myPC == EXIT_STEP )      // exit the program?
                        {
                            status = WS_OK;
                        }
                        else
                        {
                            // next attempt should retry at current step
                            // unless it is currently at the start step
                            myPC = max( myPC, EXIT_STEP );  
                            myWorkData.MoveToStringToFromString();
                            status = WS_CONTINUE;
                        }
                    }
                }

                myWorkData.UnrefCurrentPattern();
            }

            if ( status == WS_NO_MATCH && myPC == START_STEP )
            {
                status = WS_ERROR_START_STEP_NO_MATCH;
            }

            if ( status == WS_NO_MATCH )
            {
                status = WS_CONTINUE;

                if ( found_any )
                {
                    found_any = false;
                    myPC = EXIT_STEP;
                }
                else
                {
                    myPC++;
                }
            }
        }
    }

    if ( myIsVerbose )
    {
        if ( myDebugToConsole )
        {
            DebugPrintFinal( cout, status );
        }

        if ( theDebug != 0 )
        {
            DebugPrintFinal( *theDebug, status );
        }
    }

    theOutputString = myWorkData.GetToStr();

    return status;
}



WorkStatus_t Work::QuickCheckPattern(
                    const bitset<TAGGED_CHAR_END> & thePatternCharsUsed )
{
    const bitset<TAGGED_CHAR_END> & from_chars =
        myWorkData.GetFromStrCharsUsed();

    bitset<TAGGED_CHAR_END> tmp = thePatternCharsUsed & (~from_chars);

    return tmp.any() ? WS_NO_MATCH : WS_CONTINUE;
}




WorkStatus_t Work::DoReplacement( const TaggedString & theReplacementStr )
{
    int cur_wildcard_occurrence[WC_END];
    myWorkData.ClearToString();

    for ( size_t i = 0; i < WC_END; i++ )
    {
        cur_wildcard_occurrence[i] = -1;
    }

    int prefix_len;
    int suffix_start;
    int suffix_len;
    myWorkData.GetPrefixAndSuffix( prefix_len,
                                   suffix_start, suffix_len );

    const TaggedString & fromstr = myWorkData.GetFromStr();

    for ( int i = 0; i < prefix_len; i++ )
    {
        TaggedChar_t tc = fromstr[i];
        myWorkData.AppendCharToToString( tc );
    }

    for ( size_t i = 0; i < theReplacementStr.size(); i++ )
    {
        TaggedChar_t tc = theReplacementStr[i];
        Wildcard_t wt = ToWildcard(tc);

        if ( wt == WC_END )
        {   // ordinary character
            myWorkData.AppendCharToToString( tc );
        }
        else
        {   // wildcard char
            myWorkData.GetNextWildcardOccurrence( 
                            wt, cur_wildcard_occurrence[wt] );

            if ( cur_wildcard_occurrence[wt] < 0 )
            {
                return WS_ERROR_REPLACE_STR_BAD_WILDCARD;
            }
            else
            {
                myWorkData.AppendWildcardOccurrenceToToString( 
                                        cur_wildcard_occurrence[wt] );
            }
        }
    }

    for ( int i = 0; i < suffix_len; i++ )
    {
        TaggedChar_t tc = fromstr[suffix_start + i];
        myWorkData.AppendCharToToString( tc );
    }

    return WS_CONTINUE;
}



WorkStatus_t Work::DoPatternMatch( const TaggedString & pat,
                                   ofstream * theDebug )
{
    WorkStatus_t status = WS_CONTINUE;

    PM_Level top = { 0, 0, 0, -1, 0, 0, 0, false  };
    myStack.clear();
    myStack.push_back( top );
    while ( status == WS_CONTINUE && !myStack.empty() )
    {
        status = DoPatternMatch1();

        if ( myIsVerbose )
        {
            if ( myDebugToConsole )
            {
                PrintWorkState( cout, status );
            }

            if ( theDebug != 0 )
            {
                PrintWorkState( *theDebug, status );
            }
        }

        if ( (theDebug != 0 || myDebugToConsole) &&
            BREAK_AT_UID != 0 && BREAK_AT_UID == myUID )
        {
            TriggerBreakpoint();
        }

        myUID++;
    }

    if ( status == WS_CONTINUE && myStack.empty() )
    {
        status = WS_ERROR_STACK_EMPTY;
    }

    return status;
}



// This is the meat of the pattern matching.
// It has two phases for each fragment, the first advances the
// pattern fragment to the next matching substring in the from string,
// and the second process the gap before the fragment (which are the
// consecutive wildcards before the start of the fragment).  It will
// return between the phases, so DoPatternMatch1 can print out the stack.
WorkStatus_t Work::DoPatternMatch1()
{
    PM_Level top = myStack.back();
    myStack.pop_back();
    WorkStatus_t status = WS_CONTINUE;
    size_t num_frags = myWorkData.GetNumPatternFragments();

    if ( !top.myFixedIsMatched )
    {
        status = PlaceFixedFragment( top );

        myWorkData.UnmatchFromString( top.myFsWildIx );

        if ( status == WS_CONTINUE &&
             top.myFragIx < num_frags )
        {   // push following occurrence of fragment onto stack below top
            int len = myWorkData.GetPatFragLengthInFromStr(top.myFragIx);
            assert( len >= 0 ); // if wildcard it should be matched
            const TaggedString & fromstr = myWorkData.GetFromStr();

            if ( top.myFsFixedIx + len < (int)fromstr.size() )
            {   // we have characters in FromStr after this fragment
                PM_Level next = top;
                next.myFsFixedIx++;
                next.myFixedIsMatched = false;
                myStack.push_back( next );
            }
        }

        if ( status == WS_CONTINUE )
        {
            myStack.push_back( top );
        }
    }
    else 
    {
        bool skip_advance_to_next_frag = false;

        if ( top.myPatWildIx < top.myPatFixedIx )
        {
            status = TryToFillGap( top );

            if ( status == WS_CONTINUE )
            {
                MaybePushNextWildcardAttempt( top );
            }

            if ( status == WS_CONTINUE )
            {
                status = CheckAndHandleWildcard( top );
            }

            if ( status == WS_CONTINUE &&
                 top.myPatWildIx < top.myPatFixedIx )
            {   // at least one more wildcard in this gap
                myStack.push_back(top);
                skip_advance_to_next_frag = true;
            }
        }

        if ( status == WS_CONTINUE && !skip_advance_to_next_frag &&
             top.myFragIx <= num_frags && num_frags != 0 )
        {
            myStack.push_back( top );
            PushAdvanceToNextFrag( top );
        }

        if ( status == WS_CONTINUE && 
             top.myFragIx == num_frags &&
             top.myPatWildIx == top.myPatFixedIx )
        {
            status = WS_OK;
            myWorkData.SetPrefixAndSuffix( 
                    top.myFsLeftIx >= 0 ? top.myFsLeftIx : top.myFsWildIx, 
                    top.myFsFixedIx );
        }
    }

    return status;
}



// This method will advance the top.myFsFixedIx pointer until it points
// to the next occurrence of this fixed fragment in the from string.
// This also will determine the size of the gap before the first fragment,
// after the last fragment and in the case where there are no fragments.

// If there are no fragments, and there are any *$% wildcards, the gap
// will consist of the entire From String, otherwise the gap will start at
// the beginning of the From String and its length will be the number
// of ?. wildcards.  i.e. "*" matches the entire string, "?" matched
// only the first char of the string.

// If there are any *$% wildcards in the gap before the first fragment,
// the gap will consist of everything from the beginning of the From String 
// to the start of the fragment, otherwise the gap will consist of the count 
// of the number of ?. wildcards just before the fragment.
// ie: for "*A" the * will match everything before the first occurrence of A,
// and for ".A" the . will match the character before the first A.

// If there are any *$% wildcards in the gap after the last fragment,
// the gap will consist of everything from the end of the last fragment to
// the end of the string, otherwise the gap will consist of only the count of
// ?. wildcards just after the last fragment.
// ie: for "A*" the * will match everything from the first A on,
// and for "A." the . will match the character after the first A.

WorkStatus_t Work::PlaceFixedFragment( PM_Level & top )
{
    WorkStatus_t status = WS_CONTINUE;
    const TaggedString & pat = myWorkData.GetCurPat();
    const TaggedString & fs = myWorkData.GetFromStr();
    size_t num_frags = myWorkData.GetNumPatternFragments();

    if ( num_frags == 0 )
    {
        int max_span = MaxWildcardSpan( 0, pat.size() );
        top.myPatWildIx = 0;
        top.myPatFixedIx = (int)pat.size();
        top.myFsFixedIx = (int)fs.size();
        top.myFsWildIx = max_span == -1 ? 0 : max_span;
        top.myFsWildEndIx = top.myFsFixedIx;
        top.myFixedIsMatched = true;
    }
    else if ( top.myFragIx == num_frags )
    {   // after last fragment.
        // set myPatWildIx to end of previous fragment, and
        // myPatFixedIx to the end of the pattern
        int prev_pat_fixed_start = 
                myWorkData.GetPatFragStartInPat( top.myFragIx-1 );
        int prev_pat_fixed_len = 
                myWorkData.GetPatFragLengthInPat( top.myFragIx-1 );
        top.myPatWildIx = prev_pat_fixed_start + prev_pat_fixed_len;
        top.myPatFixedIx = (int)pat.size();

        // Set myFsWildIx to the end of the previous fragment.
        // If any *$% wildcards, then gap will end at the
        // end of the from string, otherwise gap will be the number of
        // ?. wildcards
        int prev_fs_frag_start;
        int prev_fs_frag_len;
        myWorkData.GetPatFragStartLengthInFromStr(
                                    top.myFragIx-1, prev_fs_frag_start,
                                    prev_fs_frag_len );
        assert( prev_fs_frag_start >= 0 && prev_fs_frag_len >= 0 );
        top.myFsWildIx = prev_fs_frag_start + prev_fs_frag_len;
        int max_span = MaxWildcardSpan( top.myPatWildIx, top.myPatFixedIx );
        top.myFsFixedIx = 
                max_span == -1 ? (int)fs.size() : top.myFsWildIx + max_span;
        top.myFsWildEndIx = top.myFsFixedIx;
        top.myFixedIsMatched = true;
    }
    else
    {
        int pat_wild_start = myWorkData.GetPatFragStartInPat( top.myFragIx);
        bool matched;
        if ( top.myFragIx > 0 && top.myPatWildIx == pat_wild_start )
        {   // no wildcards in gap before fixed string
            matched = myWorkData.VerifyPatFragPos( top.myFragIx, top.myFsFixedIx );
        }
        else
        {
            matched = myWorkData.AdvancePatFragPos( top.myFragIx, top.myFsFixedIx );
        }

        if ( matched )
        {
            top.myFsFixedIx = myWorkData.GetPatFragPosInFromStr( top.myFragIx );
            int len = myWorkData.GetPatFragLengthInFromStr( top.myFragIx );
            assert( top.myFsFixedIx >= 0 && len >= 0 ); 

            top.myFixedIsMatched = true;

            if ( top.myFsLeftIx < 0 || top.myFsLeftIx > top.myFsFixedIx )
            {
                top.myFsLeftIx = top.myFsFixedIx;
            }
       
            if ( top.myFragIx == 0 )
            {   // first fragment.
                // If any *$% wildcards, gap will be from the beginning of the
                // From String to the beginning of the fragment,
                // otherwise gap will be the count of ?. wildcards before the
                // beginning of the fragment.
                top.myPatWildIx = 0;
                top.myPatFixedIx = myWorkData.GetPatFragStartInPat(top.myFragIx);
                int max_span = MaxWildcardSpan( 0, top.myPatFixedIx );
                top.myFsWildIx = max_span == -1 ? 0 : top.myFsFixedIx - max_span;
            }

            top.myFsWildEndIx = top.myFsFixedIx;
            top.myPatFixedIx = myWorkData.GetPatFragStartInPat(top.myFragIx);
        }
        else
        {
             status = WS_NO_MATCH;
        }
    }

    return status;
}



// If this is a ?. wildcard, set its max FromString span to 1 char.
// For *$% wildcards, to save unnecessary checking which would immediately
// cause the following wildcards in the gap to fail, reduce the size of
// the maximum span for this wildcard to allow 1 char for each ?.
// wildcard to the right
WorkStatus_t Work::TryToFillGap( PM_Level & top )
{
    WorkStatus_t status = WS_CONTINUE;

    Wildcard_t wt = GetCurWildcardType( top );

    if ( WildcardMatches1Char( wt ) )
    {
        if ( top.myFsWildIx == top.myFsWildEndIx ||
             top.myFsWildIx == top.myFsFixedIx )
        {
            status = WS_NO_MATCH;
        }
        else
        {
            top.myFsWildEndIx = top.myFsWildIx + 1;
        }
    }
    else
    {
        int span = MinWildcardSpan( top.myPatWildIx + 1, top.myPatFixedIx );

        top.myFsWildEndIx = max( top.myFsWildIx,
                                 min( top.myFsWildEndIx,
                                      top.myFsFixedIx - span ) );
    }

    return status;
}



WorkStatus_t Work::CheckAndHandleWildcard( PM_Level & top )
{
    if ( top.myPatWildIx == top.myPatFixedIx )
    {   // no more wildcards in gap
        return WS_CONTINUE;
    }

    if ( top.myPatWildIx + 1 == top.myPatFixedIx &&
         top.myFsWildEndIx != top.myFsFixedIx )
    {   // this is the last wildcard in gap, and gap is not filled
        return WS_NO_MATCH;
    }

    int matched_len = top.myFsWildEndIx - top.myFsWildIx;
    Wildcard_t wt = GetCurWildcardType( top );

    assert( wt != WC_END );     // gap should only contain wildcards

    if ( WildcardMatches1Char( wt ) )
    {   // make sure ?. only match 1 character
        if ( matched_len != 1 )
        {
            return WS_NO_MATCH;
        }
    }

    const TaggedString & fs = myWorkData.GetFromStr();

    if ( WildcardMatchesOnlyUntagged( wt ) )
    {   // make sure $%?. match only untagged characters
        for ( int i = top.myFsWildIx; i < top.myFsWildEndIx; i++ )
        {
            if ( IsTagged( fs[i] ) )
            {
                return WS_NO_MATCH;
            }
        }
    }

    if ( WildcardIsUnique( wt ) )
    {   // make sure subsequent $%?. match first occurrence
        int wo = myWorkData.GetFirstWildcardOccurrence(wt);

        if ( wo >= 0 )
        {
            if ( !myWorkData.CompareSubstringWithWildcard( 
                                wt, top.myFsWildIx, matched_len ) )
            {
                return WS_NO_MATCH;
            }
        }
        else
        {   // save first substring matched by $%?.
            myWorkData.FoundWildcard( wt, top.myFsWildIx, matched_len );
        }
    }
    else
    {   // save substring matched by *
        myWorkData.FoundWildcard( wt, top.myFsWildIx, matched_len );
    }

    if ( top.myFsLeftIx < 0 || top.myFsLeftIx > top.myFsWildIx )
    {   // mark leftmost matched character in this From String
        top.myFsLeftIx = top.myFsWildIx;
    }

    // advance to next wildcard in gap
    top.myPatWildIx++;
    top.myFsWildIx = top.myFsWildEndIx;
    top.myFsWildEndIx = top.myFsFixedIx;

    return WS_CONTINUE;
}


// at this point we know that top.myFragIx < number of fragments
// on exit, myPatWildIx and myPatFixedIx (of the top of the stack)
// will point to just after the end of the previous fragment
// in the pattern, and myFsWildIx, myFsWildEndIx and myFsFixedIx will 
// point to just after the end of the previous fragment in the from string.
// We don't know exactly where the fragment will start in the from string,
// but we know it can't be before this point.
void Work::PushAdvanceToNextFrag( PM_Level & top )
{
    PM_Level & prev = myStack.back();
    PM_Level next = prev;

    next.myFragIx++;
    int prev_pat_frag_len = myWorkData.GetPatFragLengthInPat( prev.myFragIx );
    assert( prev_pat_frag_len >= 0 );

    next.myPatWildIx = prev.myPatFixedIx + prev_pat_frag_len;
    next.myPatFixedIx = next.myPatWildIx;

    int prev_fs_frag_len = myWorkData.GetPatFragLengthInFromStr( prev.myFragIx );
    assert( prev_fs_frag_len >= 0 );

    next.myFsWildIx = prev.myFsFixedIx + prev_fs_frag_len;
    next.myFsWildEndIx = next.myFsWildIx;
    next.myFsFixedIx = next.myFsWildIx;

    next.myFixedIsMatched = false;

    myStack.push_back( next );
}



// If this wildcard isn't ?. then push another try at
// this same wildcard reducing the span by 1 char
void Work::MaybePushNextWildcardAttempt( PM_Level & top )
{
    Wildcard_t wt = GetCurWildcardType( top );

    if ( !WildcardMatches1Char(wt) && top.myFsWildEndIx > top.myFsWildIx &&
         top.myPatWildIx + 1 < top.myPatFixedIx )
    {
        PM_Level next = top;
        next.myFsWildEndIx--;
        myStack.push_back( next );
    }
}



int Work::MinWildcardSpan( int pat_from,
                           int pat_to )
{
    int len_so_far = 0;
    const TaggedString & pat = myWorkData.GetCurPat();

    for ( int i = pat_from; i < pat_to; i++ )
    {
        Wildcard_t wt = ToWildcard( pat[i] );

        if ( WildcardMatches1Char(wt) )
        {
            len_so_far++;
        }
    }

    return len_so_far;
}




int Work::MaxWildcardSpan( int pat_from,
                           int pat_to )
{
    int len_so_far = 0;
    const TaggedString & pat = myWorkData.GetCurPat();

    for ( int i = pat_from; i < pat_to; i++ )
    {
        Wildcard_t wt = ToWildcard( pat[i] );

        if ( WildcardMatches1Char(wt) )
        {
            len_so_far++;
        }
        else
        {
            return -1;
        }
    }

    return len_so_far;
}




Wildcard_t Work::GetCurWildcardType( PM_Level & top )
{
    if ( top.myPatWildIx == top.myPatFixedIx )
    {
        return WC_END;
    }

    const TaggedString & pat = myWorkData.GetCurPat();
    return ToWildcard( pat[top.myPatWildIx] );
}



string Work::FromStringChar( int theIx ) const
{
    return TaggedStringChar( myWorkData.GetFromStr(), theIx );
}



string Work::PatStringChar( int theIx ) const
{
    if ( myWorkData.HasCurPat() )
    {
        return TaggedStringChar( myWorkData.GetCurPat(), theIx );
    }
    else
    {
        return string( "?" );
    }
}



void Work::PrintWorkState( ostream & out,
                           WorkStatus_t status )  const
{
    static const size_t MAX_PAT_STRING = 30;
    static const size_t MAX_FROM_STRING = 30;
    static const size_t MAX_TO_STRING = 30;
    static const size_t MAX_WILDCARD_STRING =  40;
    static const size_t MAX_STACK_TO_PRINT = 10;

    out << "############# Work State #############" << endl;
    
    if ( status != WS_CONTINUE )
    {
        out << "### Pattern Match Status " << GetWorkStatusStr(status) << endl;
    }

    out << "### PC: " << myPC << "  src_line: " << 
        myProgram[myPC].GetLineNumber() << "  uid: " << myUID << endl;

    if ( myWorkData.HasCurPat() )
    {
        const TaggedString & pat = myWorkData.GetCurPat();
        out << "### PAT: " << pat.size();
        PrintTaggedString( out, pat, MAX_PAT_STRING );
        out << endl;
    }

    const TaggedString & fs = myWorkData.GetFromStr();

    out << "### FROM: " << fs.size();
    PrintTaggedString( out, fs, MAX_FROM_STRING );
    out << endl;

    size_t num_frag = myWorkData.GetNumPatternFragments();

    if ( num_frag > 0 )
    {
        out << "### PATTERN FRAGMENTS:" << endl;

        for ( size_t i = 0; i < num_frag; i++ )
        {
            int pat_start = myWorkData.GetPatFragStartInPat( i );
            int pat_len = myWorkData.GetPatFragLengthInPat( i );
            out << "###   " << i << ": patstart " << pat_start << " patlen " <<
                pat_len << " = " << PatStringChar(pat_start);

            if ( pat_len > 1 )
            {
                out << " .. " << PatStringChar( pat_start+pat_len-1 );
            }

            int fs_start;
            int fs_len;
            myWorkData.GetPatFragStartLengthInFromStr( i, fs_start, fs_len );

            if ( fs_start < 0 || fs_len < 0 )
            {
                out << " not_matched";
            }
            else
            {
                out << " fs_start " << fs_start << " fs_len " << fs_len <<
                    " = " << FromStringChar(fs_start);

                if ( fs_len > 1 )
                {
                    out << " .. " << FromStringChar(fs_start+fs_len-1);
                }
            }

            out << endl;
        }
    }

    int numwild = myWorkData.GetNumWildcardsUsed();

    if ( numwild > 0 )
    {
        out << "### WILD:" << endl;

        for ( int w = 0; w < numwild; w++ )
        {
            out << "###   " << w << ": ";
            Wildcard_t wt = myWorkData.GetWildcardType(w);
            if ( wt == WC_END )
            {
                out << "??? bad wildcard occurrence" << endl;
            }
            else
            {
                int start;
                int len;
                myWorkData.GetWildcardSubstringInfo( w, start, len );
                out << FromTaggedChar( GetWildcardChar(wt) ) << ": @" <<
                    start << " " << len;
                TaggedString ts;
                myWorkData.GetSubstringOfFromStr( ts, start, len );
                PrintTaggedString( out, ts, MAX_WILDCARD_STRING );
                out << endl;
            }
        }
    }

    if ( !myStack.empty() )
    {
        out << "### STACK <" << myStack.size() << ">:" << endl;
        size_t stacksize = myStack.size();
        size_t limit = min( stacksize, MAX_STACK_TO_PRINT );
        size_t si = stacksize - 1;
        for ( size_t i = 0; i < limit; i++ )
        {
            out << "###   " << si << ": ";
            const PM_Level & e = myStack[si];

            out << "#:" << e.myFragIx;
            out << " PAT w:" << e.myPatWildIx << "=" << PatStringChar(e.myPatWildIx);
            out << " f:" << e.myPatFixedIx << "=" << PatStringChar(e.myPatFixedIx);
            out << " FS <:" << e.myFsLeftIx;
            out << " w:" << e.myFsWildIx << "=" << FromStringChar(e.myFsWildIx);
            out << " we:" << e.myFsWildEndIx << "=" << FromStringChar(e.myFsWildEndIx);
            out << " fx:" << e.myFsFixedIx << "=" << FromStringChar(e.myFsFixedIx);
            out << (e.myFixedIsMatched ? "" : " !m") << endl;
            si--;
        }
    }

    int prefix_len;
    int suffix_start;
    int suffix_len;
    myWorkData.GetPrefixAndSuffix( prefix_len, suffix_start, suffix_len );

    if ( prefix_len > 0 || suffix_len > 0 )
    {
        TaggedString str;
        myWorkData.GetSubstringOfFromStr( str, 0, prefix_len );
        out << "### PREFIX: " << str.size();
        PrintTaggedString( out, str, MAX_WILDCARD_STRING );
        out << endl;
        myWorkData.GetSubstringOfFromStr( str, suffix_start, suffix_len );
        out << "### SUFFIX: " << str.size();
        PrintTaggedString( out, str, MAX_WILDCARD_STRING );
        out << endl;
    }

    const TaggedString & ts = myWorkData.GetToStr();

    if ( !ts.empty() )
    {
        out << "### TO: " << ts.size();
        PrintTaggedString( out, ts, MAX_TO_STRING );
        out << endl;
    }


    out << "######################################" << endl;
}


void Work::DebugPrintInitial( ostream & out )  const
{
    static const size_t MAX_FROM_STRING = 120;

    out << "########### Initial String ###############" << endl;
    const TaggedString & fs = myWorkData.GetFromStr();
    out << "# " << fs.size();
    PrintTaggedString( out, fs, MAX_FROM_STRING );
    out << endl;
    out << "##########################################" << endl;
}


void Work::DebugPrintTransition( ostream & out ) const
{
    static const size_t MAX_FROM_STRING = 500;
    static const size_t MAX_TO_STRING = 500;

    out << endl;
    out << "################ Transition ##############" << endl;
    const TaggedString & fs = myWorkData.GetFromStr();

    out << "## PC: " << myPC << "  src_line: " << 
        myProgram[myPC].GetLineNumber();
    out << "  uid: " << myUID;

    out << endl;

    out << "## FROM: " << fs.size();
    PrintTaggedString( out, fs, MAX_FROM_STRING );
    out << endl;

    const TaggedString & ts = myWorkData.GetToStr();

    out << "## TO:   " << ts.size();
    PrintTaggedString( out, ts, MAX_TO_STRING );
    out << endl;

    out << "##########################################" << endl;
    out << endl;
}



void Work::DebugPrintFinal( ostream & out,
                            WorkStatus_t status ) const
{
    static const size_t MAX_FROM_STRING = 120;

    out << "############ Final String ################" << endl;
    const TaggedString & ts = myWorkData.GetToStr();
    out << "# " << ts.size();
    PrintTaggedString( out, ts, MAX_FROM_STRING );
    out << endl;
    out << "# STATUS: " << GetWorkStatusStr(status) << endl;
    out << "##########################################" << endl;
    out << endl;
}



void Work::TriggerBreakpoint()
{   // set debugger breakpoint here
    cout << "!!! Breakpoint" << endl;
}
