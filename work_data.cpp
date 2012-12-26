// FILE:  work_data.cpp
//
// DESCRIPTION:
//      Implements module described in work_data.h
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#include "work_data.h"
#include "tagged_char.h"
#include "misc.h"
#include <assert.h>


using namespace std;


// This identifies a wildcard substring which has already
// been matched in the working string
struct WildcardOccurrence
{
    Wildcard_t myWildcardType;
    int        myStartingIx;
    int        myLength;
};




WorkData::WorkData() :
    myFromStringIsA( false ),
    myFromStringHasInfo( false ),
    myCurPat( 0 )
{
    ClearWildcardOccurrences();
    ClearPrefixAndSuffix();
}



WorkData::~WorkData()
{
}



const TaggedString & WorkData::GetFromStr()
{
    return myFromStringIsA ? myWorkA : myWorkB;
}




const std::bitset<TAGGED_CHAR_END> & WorkData::GetFromStrCharsUsed()
{
    if ( !myFromStringHasInfo )
    {
        GetFromStringInfo();
    }

    return myFromStringCharsUsed;
}



int WorkData::GetFromStrCFirst( TaggedChar_t tc )
{
    if ( !myFromStringHasInfo )
    {
        GetFromStringInfo();
    }

    return myFromStringCFirst[tc];
}



int WorkData::GetFromStrCNext( int i )
{
    if ( !myFromStringHasInfo )
    {
        GetFromStringInfo();
    }

    assert( i >= 0 && i < (int)myFromStringCNext.size() );

    return myFromStringCNext[i];
}



void WorkData::GetSubstringOfFromStr( TaggedString & theStr,
                                      int theStartIx,
                                      int theLen )
{
    theStr.clear();
    const TaggedString & fs = GetFromStr();

    assert( theStartIx >= 0 && theLen >= 0 && 
            theStartIx + theLen <= (int)fs.size() );

    for ( int i = 0; i < theLen; i++ )
    {
        theStr.push_back( fs[theStartIx+i] );
    }
}



const TaggedString & WorkData::GetToStr()
{
    return myFromStringIsA ? myWorkB : myWorkA;
}



void WorkData::ClearWildcardOccurrences()
{
    myFromStringWildcards.clear();

    for ( size_t i = 0; i <= WC_END; i++ )
    {
        myFirstWildcardOccurrence[i] = -1;
    }
}



int WorkData::GetNumWildcardsUsed() const
{
    return (int)myFromStringWildcards.size();
}



void WorkData::GetWildcardSubstring( int theWildcardOccurrence,
                                     TaggedString & theSubstring )
{
    assert( theWildcardOccurrence >= 0 &&
            theWildcardOccurrence < GetNumWildcardsUsed() );

    const TaggedString & fromstr = GetFromStr();

    int start;
    int len;

    GetWildcardSubstringInfo( theWildcardOccurrence, start, len );

    theSubstring.clear();
    theSubstring.reserve( len );

    for ( int i = start; i < start+len; i++ )
    {
        theSubstring.push_back( fromstr[i] );
    }
}



void WorkData::GetWildcardSubstringInfo( int theWildcardOccurrence,
                                         int & theStartIx,
                                         int & theLength )
{
    assert( theWildcardOccurrence >= 0 &&
            theWildcardOccurrence < GetNumWildcardsUsed() );

    theStartIx = 0;
    theLength = 0;

    theStartIx = myFromStringWildcards[theWildcardOccurrence].myStartingIx;
    theLength  = myFromStringWildcards[theWildcardOccurrence].myLength;
}



Wildcard_t WorkData::GetWildcardType( int theWildcardOccurrence )
{
    assert( theWildcardOccurrence >= 0 &&
            theWildcardOccurrence < GetNumWildcardsUsed() );

    return myFromStringWildcards[theWildcardOccurrence].myWildcardType;
}



int WorkData::GetFirstWildcardOccurrence( Wildcard_t theWildcardType )
{
    int len = GetNumWildcardsUsed();

    for ( int i = 0; i < len; i++ )
    {
        if ( GetWildcardType(i) == theWildcardType )
        {
            return i;
        }
    }

    return -1;
}



void WorkData::GetNextWildcardOccurrence( Wildcard_t theWildcardType,
                                          int & theOccurrence )
{
    int len = GetNumWildcardsUsed();
    bool found = false;

    for ( int i = 0; i < len && !found; i++ )
    {
        theOccurrence++;

        if ( theOccurrence < 0 || theOccurrence >= len )
        {
            theOccurrence = 0;
        }

        if ( GetWildcardType( theOccurrence ) == theWildcardType )
        {
            found = true;
        }
    }

    if ( !found )
    {
        theOccurrence = -1;
    }
}



bool WorkData::CompareSubstringWithWildcard( Wildcard_t theWildcardType,
                                             int theStartIx,
                                             int theLength )
{
    const TaggedString & from_str = GetFromStr();
    int wo = GetFirstWildcardOccurrence( theWildcardType );

    if ( wo < 0 )
    {
        return true;
    }
    else if ( theStartIx + theLength > (int)from_str.size() )
    {
        return false;
    }
    else
    {
        int prev_start;
        int prev_length;

        GetWildcardSubstringInfo( wo, prev_start, prev_length );

        if ( prev_length != theLength )
        {
            return false;
        }
        else
        {
            for ( int i = 0; i < theLength; i++ )
            {
                if ( from_str[prev_start + i] != from_str[theStartIx + i] )
                {
                    return false;
                }
            }
        }
    }

    return true;
}



void WorkData::FoundWildcard( Wildcard_t theWildcardType,
                              int        theStartingIndex,
                              int        theSize )
{
    if ( myFirstWildcardOccurrence[theWildcardType] < 0 )
    {
        myFirstWildcardOccurrence[theWildcardType] = GetNumWildcardsUsed();
    }

    WildcardOccurrence wo = { theWildcardType, theStartingIndex, theSize };
    myFromStringWildcards.push_back( wo );
}



void WorkData::MoveToStringToFromString()
{
    myFromStringIsA = !myFromStringIsA;
    myFromStringHasInfo = false;
    ClearWildcardOccurrences();
    ClearToString();
    myPatternFragments.clear();
    ClearAllPatFragPosInFromStr();
    ClearPrefixAndSuffix();

    GetFromStringInfo();
}



void WorkData::ClearFromString()
{
    if ( myFromStringIsA )
    {
        myWorkA.clear();
    }
    else
    {
        myWorkB.clear();
    }

    myFromStringHasInfo = false;
    ClearWildcardOccurrences();
    ClearPrefixAndSuffix();
}



void WorkData::ClearToString()
{
    if ( myFromStringIsA )
    {
        myWorkB.clear();
    }
    else
    {
        myWorkA.clear();
    }
}



void WorkData::AppendCharToToString( TaggedChar_t theChar )
{
    TaggedString & ts = myFromStringIsA ? myWorkB : myWorkA;
    ts.push_back( theChar );
}



void WorkData::AppendStringToToString( const TaggedString & theStr )
{
    TaggedString & tostr = myFromStringIsA ? myWorkB : myWorkA;

    size_t len = theStr.size();

    for ( size_t i = 0; i < len; i++ )
    {
        tostr.push_back( theStr[i] );
    }
}



void WorkData::AppendWildcardOccurrenceToToString( int wo )
{
    assert( wo >= 0 && wo < (int)myFromStringWildcards.size() );

    const TaggedString & fromstr = GetFromStr();
    TaggedString & tostr = myFromStringIsA ? myWorkB : myWorkA;

    size_t start = (size_t)myFromStringWildcards[wo].myStartingIx;
    size_t len   = (size_t)myFromStringWildcards[wo].myLength;

    assert( start + len <= fromstr.size() );

    for ( size_t i = start; i < start+len; i++ )
    {
        tostr.push_back( fromstr[i] );
    }
}




void WorkData::UnmatchFromString( int theNewMatchLength )
{
    TaggedString & ts = myFromStringIsA ? myWorkA : myWorkB;

    assert( theNewMatchLength >= 0 && theNewMatchLength <= (int)ts.size() );

    int wo = GetNumWildcardsUsed();

    for ( --wo; wo >= 0; wo-- )
    {
        if ( myFromStringWildcards[wo].myStartingIx +
             myFromStringWildcards[wo].myLength > theNewMatchLength )
        {
            myFromStringWildcards.pop_back();
        }
        else
        {
            break;
        }
    }

    for ( int w = 0; w < WC_END; w++ )
    {
        myFirstWildcardOccurrence[w] = -1;
    }

    int womax = GetNumWildcardsUsed();

    for ( wo = 0; wo < womax; wo++ )
    {
        Wildcard_t w = myFromStringWildcards[wo].myWildcardType;

        if (  myFirstWildcardOccurrence[w] == -1 )
        {
            myFirstWildcardOccurrence[w] = wo;
        }
    }
}



// split a pattern string into fragments which consist either of
// the largest number of consecutive non-wildcard characters, or
// a single fixed wildcard ?.$% character (meaning the caller
// at this point can figure out the characters in the search
// string by consulting the wildcard fragment table).
// a wildcard other than * becomes fixed after the first
// non-wildcard character after the first occurrence of that
// wildcard character.  Until then, it should be included in a
// floating fragment with other contigous wildcards.
static void SplitPatternIntoFragments( const TaggedString & pat,
                                       std::vector<PatternFragment> & vpatfrag )
{
    int patlen = (int)pat.size();
    int start = 0;
    int len = 0;
    BitSet_t wildcard_used = 0;
    BitSet_t not_yet_fixed = 0;
    vpatfrag.clear();

    for ( int pi = 0; pi < patlen; pi++ )
    {
        Wildcard_t wt = ToWildcard( pat[pi] );

        if ( wt == WC_END )
        {
            len++;
            not_yet_fixed = 0;  // all wildcards previously seen are fixed
        }
        else
        {
            if ( len != 0 )
            {
                PatternFragment ff = { start, len };
                vpatfrag.push_back( ff );
            }

            start = pi + 1;
            len = 0;

            if ( WildcardIsUnique(wt) )
            {
                if ( (wildcard_used & SET_BIT(wt)) != 0 &&
                     (not_yet_fixed & SET_BIT(wt)) == 0 )
                {   // we have seen this before, and it is fixed
                    PatternFragment ff = { pi, -1 };
                    vpatfrag.push_back( ff );
                }
                else
                {   // this wildcard is not yet fixed
                    wildcard_used |= SET_BIT(wt);
                    not_yet_fixed |= SET_BIT(wt);
                }
            }
        }
    }

    if ( len != 0 )
    {
        PatternFragment pf = { start, len };
        vpatfrag.push_back( pf );
    }
}



void WorkData::SetCurrentPattern( const TaggedString & thePattern )
{
    myCurPat = &thePattern;
    SplitPatternIntoFragments( thePattern, myPatternFragments );
    ClearAllPatFragPosInFromStr();
}



void WorkData::UnrefCurrentPattern()
{
    myCurPat = 0;
    myPatternFragments.clear();
    myPatFragCurrentPos.clear();
    ClearWildcardOccurrences();
}



const TaggedString & WorkData::GetCurPat() const
{
    assert( myCurPat != 0 );
    return *myCurPat;
}



bool WorkData::HasCurPat() const
{
    return myCurPat != 0;
}


int WorkData::GetNumPatternFragments() const
{
    return (int)myPatternFragments.size();
}



TaggedChar_t WorkData::GetPatFragFirstChar( size_t frag_ix )
{
    TaggedChar_t rtv = (TaggedChar_t)0;

    if ( frag_ix < myPatternFragments.size() )
    {
        const TaggedString & pat = GetCurPat();

        int start = myPatternFragments[frag_ix].myStart;
        int len   = myPatternFragments[frag_ix].myLength;

        if ( len < 0 )
        {   // this fragment is a single wildcard
            TaggedChar_t tc = GetPatFragFirstCharInPat( frag_ix );
            Wildcard_t wt = ToWildcard(tc);
            int wo = myFirstWildcardOccurrence[wt];

            if ( wo >= 0 && myFromStringWildcards[wo].myLength > 0 )
            {   // has been matched, and is not null
                const TaggedString & from_str = GetFromStr();
                int pos = myFromStringWildcards[wo].myStartingIx;
                rtv = from_str[pos];
            }
        }
        else if ( len >= 0 )
        {   // ordinary fragment in pattern
            assert( start >= 0 && start < (int)pat.size() );
            rtv = pat[start];
        }
    }

    return rtv;
}



bool WorkData::GetPatFragStartLengthInFromStr( size_t frag_ix,
                                               int & start,
                                               int & len )
{
    TaggedChar_t tc = GetPatFragFirstCharInPat(frag_ix);
    Wildcard_t wt = ToWildcard(tc);

    if ( wt != WC_END )
    {
        int wo = GetFirstWildcardOccurrence(wt);

        if ( wo >= 0 )
        {
            start = myFromStringWildcards[wo].myStartingIx;
            len = myFromStringWildcards[wo].myLength;
            return true;
        }
        else
        {
            start = -1;
            len = -1;
            return false;          // unknown length
        }
    }
    else if ( frag_ix == myPatternFragments.size() )
    {
        return false;
    }
    else
    {
        start = myPatFragCurrentPos[frag_ix];
        len = myPatternFragments[frag_ix].myLength;
        return true;
    }
}
        


int WorkData::GetPatFragLengthInFromStr( size_t frag_ix )
{
    TaggedChar_t tc = GetPatFragFirstCharInPat(frag_ix);
    Wildcard_t wt = ToWildcard(tc);

    if ( wt != WC_END )
    {
        int wo = GetFirstWildcardOccurrence(wt);

        if ( wo >= 0 )
        {
            return myFromStringWildcards[wo].myLength;
        }
        else
        {
            return -1;          // unknown length
        }
    }
    else if ( frag_ix >= myPatternFragments.size() )
    {
        return false;
    }
    else
    {
        return myPatternFragments[frag_ix].myLength;
    }
}



int WorkData::GetPatFragStartInPat( size_t frag_ix )
{
    if ( frag_ix == myPatternFragments.size() )
    {
        return GetCurPat().size();
    }
    else
    {
        assert( frag_ix < myPatternFragments.size() );

        return myPatternFragments[frag_ix].myStart;
    }
}



int WorkData::GetPatFragLengthInPat( size_t frag_ix )
{
    int len = 0;

    if ( frag_ix < myPatternFragments.size() )
    {
        len = myPatternFragments[frag_ix].myLength;

        if ( len < 0 )
        {
            len = 1;
        }
    }

    return len;
}



TaggedChar_t WorkData::GetPatFragFirstCharInPat( size_t frag_ix )
{
    TaggedChar_t tc = (TaggedChar_t)0;

    if ( frag_ix < myPatternFragments.size() )
    {
        const TaggedString & pat = GetCurPat();
        int start = GetPatFragStartInPat( frag_ix );

        if ( start >= 0 && start < (int)pat.size() )
        {
            tc = pat[start];
        }

    }

    return tc;
}



bool WorkData::PatFragIsWildcard( size_t frag_ix )
{
    if ( frag_ix == myPatternFragments.size() )
    {
        return false;
    }
    else
    {
        assert( frag_ix < myPatternFragments.size() );
        return myPatternFragments[frag_ix].myLength < 0;
    }
}



void WorkData::ClearAllPatFragPosInFromStr()
{
    size_t len = myPatternFragments.size();

    if ( myPatFragCurrentPos.size() != len )
    {
        myPatFragCurrentPos.resize(len);
    }

    for ( size_t i = 0; i < len; i++ )
    {
        myPatFragCurrentPos[i] = -1;
    }
}



int WorkData::GetPatFragPosInFromStr( size_t frag_ix )
{
    assert( frag_ix < myPatternFragments.size() );
    return myPatFragCurrentPos[frag_ix];
}



bool WorkData::AdvancePatFragPos( size_t frag_ix,
                                  int fromstr_min_pos )
{
    size_t num_fixed_frags = myPatFragCurrentPos.size();

    if ( frag_ix >= num_fixed_frags )
    {
        return false;
    }

    int pos = myPatFragCurrentPos[frag_ix];
    int len = GetPatFragLengthInPat( frag_ix );

    if ( pos > fromstr_min_pos )
    {
        // we have backtracked to before this position, so
        // need to restart searching for matching char from
        // beginning of FromString
        myPatFragCurrentPos[frag_ix] = -1;
        pos = -1;
    }

    if ( pos < 0 )
    {   // at beginning of string: start at first matching char
        TaggedChar_t tc = GetPatFragFirstChar( frag_ix );

        if ( tc != 0 )
        {
            pos = GetFromStrCFirst(tc);
        }
    }

    while ( pos != -1 &&
            ( pos < fromstr_min_pos ||
              !CompareSubstringWithFragment( pos, len, frag_ix ) ) )
    {
        pos = GetFromStrCNext(pos);
    }

    myPatFragCurrentPos[frag_ix] = pos;

    return pos >= 0;
}



bool WorkData::VerifyPatFragPos( size_t frag_ix,
                                 int fromstr_pos )
{
    size_t num_fixed_frags = myPatFragCurrentPos.size();

    if ( frag_ix >= num_fixed_frags )
    {
        return false;
    }

    int len = GetPatFragLengthInFromStr( frag_ix );

    if ( len >= 0 &&
         CompareSubstringWithFragment( fromstr_pos, len, frag_ix ) )
    {
        myPatFragCurrentPos[frag_ix] = fromstr_pos;
        return true;
    }

    return false;
}





bool WorkData::CompareSubstringWithFragment( int fs_start,
                                             int fs_len,
                                             size_t frag_ix )
{
    bool match = false;

    const TaggedString & from_str = GetFromStr();

    if ( PatFragIsWildcard(frag_ix) )
    {
        Wildcard_t wt = ToWildcard( GetPatFragFirstCharInPat(frag_ix) );
        int wo = GetFirstWildcardOccurrence(wt);
        if ( wo >= 0 )
        {
            int start;
            int len;
            GetWildcardSubstringInfo( wo, start, len );

            if ( (int)len == fs_len )
            {
                match = (size_t)len+fs_start <= from_str.size() &&
                        (size_t)len+start <= from_str.size();
                for ( int i = 0; i < len && match; i++ )
                {
                    match = from_str[i+fs_start] == from_str[i+start];
                }
            }
        }
    }
    else
    {
        const TaggedString & pat = GetCurPat();
        int start = GetPatFragStartInPat(frag_ix);
        int len = GetPatFragLengthInPat(frag_ix);

        if ( len == fs_len )
        {
            match = (size_t)len+fs_start <= from_str.size() &&
                    (size_t)len+start <= pat.size();
            for ( int i = 0; i < len && match; i++ )
            {
               match = ( from_str[i+fs_start] == pat[i+start] );
            }
        }
    }

    return match;
}



void WorkData::GetFromStringInfo()
{
    const TaggedString & from_str = GetFromStr();
    myFromStringCNext.clear();
    myFromStringCNext.resize( from_str.size() );
    myFromStringCharsUsed.reset();

    for ( size_t ci = 0; ci < TAGGED_CHAR_END; ci++ )
    {
        myFromStringCFirst[ci] = -1;
    }

    for ( int si = (int)from_str.size() - 1; si >= 0; si-- )
    {
        TaggedChar_t tc = from_str[si];

        myFromStringCNext[si]  = myFromStringCFirst[tc];
        myFromStringCFirst[tc] = si;
        myFromStringCharsUsed.set(tc);
    }

    myFromStringHasInfo = true;
}



void WorkData::ClearPrefixAndSuffix()
{
    myPrefix.myStart  = 0;
    myPrefix.myLength = 0;
    mySuffix.myStart  = 0;
    mySuffix.myLength = 0;
}



void WorkData::SetPrefixAndSuffix( int first_match_ix,
                                   int last_match_ix )
{
    const TaggedString & fromstr = GetFromStr();
    assert( first_match_ix >= 0 && first_match_ix <= (int)fromstr.size() );
    assert( last_match_ix >= 0 && last_match_ix <= (int)fromstr.size() ); 
    myPrefix.myStart = 0;
    myPrefix.myLength = first_match_ix;
    mySuffix.myStart = last_match_ix;
    mySuffix.myLength = (int)fromstr.size() - last_match_ix;
}



void WorkData::GetPrefixAndSuffix( int & prefix_len,
                                   int & suffix_start,
                                   int & suffix_len )
{
    prefix_len   = myPrefix.myLength;
    suffix_start = mySuffix.myStart;
    suffix_len   = mySuffix.myLength;
}
