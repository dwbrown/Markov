// FILE:    tagged_char.cpp
//
// DESCRIPTION:
//      Implements module described in tagged_char.h
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#include "tagged_char.h"
#include "misc.h"
#include <iostream>
#include <memory.h>

using namespace std;


#define TAG_BIT     0x80
#define NONTAG_BITS 0x7F

#define DQUOTE          '"'
#define SQUOTE          '\''
#define BAR             '|'
#define BACKSLASH       '\\'
#define DEFAULT_DELIM   DQUOTE


// This identifies a fragment of the pattern string
// consisting either of all normal characters or
// a single ?.$% wildcard on the second or subsequent
// occurrence.  myStart is the starting index of the
// fragment in the pattern string.  myLen is either the
// length of the fragment in the pattern string or -1
// to identify this as a wildcard.
struct PatternFragment
{
    int    myStart;
    int    myLen;
};


// This is a quick lookup table for converting between a tagged char
// and a wildcard.  It is initialized automatically the first time by
// ToWildcard.
static UByte_t g_CharToWildcard[TAGGED_CHAR_END];
static bool g_InitializedCharToWildcard = false;


struct WCInfo_t
{
    char myChar;            // needs to be tagged
    bool myIsSingle;        // false = matches 1 char, true = matches 0 or more chars
    bool myIsUnique;        // true = multiple occurrences in same pattern must match
    bool myOnlyUntagged;    // true = matches only untagged chars
};



static const WCInfo_t g_WildcardToChar[WC_END] =
{
    { '?', true, true, true },    // WC_QM   - matches 1 untagged char, unique
    { '.', true, true, true },    // WC_DOT  - matches 1 untagged char, unique
    { '$', false, true, true },   // WC_DS   - matches 0 or more untagged chars, unique
    { '%', false, true, true },   // WC_PCT  - matches 0 or more untagged chars, unique
    { '*', false, false, false }  // WC_STAR - matches 0 or more chars, nonunique
};




static void InitializeCharToWildcard()
{
    memset( g_CharToWildcard, WC_END, sizeof(g_CharToWildcard) );

    for ( int i = 0; i < WC_END; i++ )
    {
        TaggedChar_t tc =  ToTaggedChar(g_WildcardToChar[i].myChar);
        g_CharToWildcard[tc] = (UByte_t)i;
    }

    g_InitializedCharToWildcard = true;
}



TaggedChar_t ToTaggedChar( char c )
{
    return (TaggedChar_t)c | TAG_BIT;
}



TaggedChar_t ToUntaggedChar( char c )
{
    return (TaggedChar_t)c & NONTAG_BITS;
}



char FromTaggedChar( TaggedChar_t tc )
{
    return (char)(tc & NONTAG_BITS);
}



bool IsTagged( TaggedChar_t tc )
{
    return (tc & TAG_BIT) != 0;
}



TaggedChar_t GetWildcardChar( Wildcard_t wc )
{
    return ToTaggedChar( g_WildcardToChar[wc].myChar );
}




Wildcard_t ToWildcard( TaggedChar_t tc )
{
    if ( !g_InitializedCharToWildcard )
    {
        InitializeCharToWildcard();
    }

    return (Wildcard_t)g_CharToWildcard[tc];
}



bool IsWildcard( TaggedChar_t tc )
{
    return ToWildcard(tc) != WC_END;
}



bool WildcardMatches1Char( Wildcard_t wc )
{
    return g_WildcardToChar[wc].myIsSingle;
}



bool WildcardMatchesString( Wildcard_t wc )
{
    return !g_WildcardToChar[wc].myIsSingle;
}



bool WildcardMatchesOnlyUntagged( Wildcard_t wc )
{
    return g_WildcardToChar[wc].myOnlyUntagged;
}



bool WildcardMatchesAny( Wildcard_t wc )
{
    return !g_WildcardToChar[wc].myOnlyUntagged;
}



bool WildcardIsUnique( Wildcard_t wc )
{
    return g_WildcardToChar[wc].myIsUnique;
}



void TaggedCharsUsed( std::bitset<TAGGED_CHAR_END> & bs,
                      const TaggedString & ts )
{
    bs.reset();

    size_t len = ts.size();

    for ( size_t i = 0; i < len; i++ )
    {
        bs.set( ts[i], true );
    }
}



static bool DelimiterUsed( const bitset<TAGGED_CHAR_END> & chars_used,
                           char delim )
{
    return chars_used.test( ToUntaggedChar(delim) ) ||
           chars_used.test( ToTaggedChar(delim) );
}



void PrintTaggedString( ostream & out,
                        const TaggedString & ts,
                        size_t max_length )
{
    bitset<TAGGED_CHAR_END> chars_used;

    TaggedCharsUsed( chars_used, ts );

    char delim = DEFAULT_DELIM;

    if ( !DelimiterUsed( chars_used, DQUOTE ) )
    {
        delim = DQUOTE;
    }
    else if ( !DelimiterUsed( chars_used, SQUOTE ) )
    {
        delim = SQUOTE;
    }
    else if ( !DelimiterUsed( chars_used, BAR ) )
    {
        delim = BAR;
    }

    out << delim;

    size_t len = ts.size();

    for ( size_t i = 0; i < len && i < max_length; i++ )
    {
        TaggedChar_t tc = ts[i];

        if ( !IsTagged(tc) )
        {
            out << BACKSLASH;
        }

        out << FromTaggedChar(tc);
    }

    out << delim;

    if ( max_length < len )
    {
        out << "...";
    }
}




string TaggedStringChar( const TaggedString & theStr,
                         int theIx )
{
    string s;

    if ( theIx >= 0 && theIx < (int)theStr.size() )
    {
        s.push_back( '\'' );

        TaggedChar_t tc = theStr[theIx];

        if ( !IsTagged(tc) )
        {
            s.push_back( '\\' );
        }
        
        s.push_back( FromTaggedChar(tc) );
        s.push_back( '\'' );
    }
    else
    {
        s.push_back( '?' );
    }

    return s;
}




