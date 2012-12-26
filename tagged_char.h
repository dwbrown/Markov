// FILE:  tagged_char.h
//
// DESCRIPTION:
//     This module defines three types and their helper functions:
//          TaggedChar_t    - A byte containing a 7-bit ASCII char
//                            plus a tag bit, 0=untagged, 1=tagged
//
//          TaggedString    - A vector of TaggedChar_t
//
//          Wildcard_t      - An enum for a wildcard type "?.$%*".
//                            Note that all wildcard chars are tagged.
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#ifndef TAGGED_CHAR_H
#define TAGGED_CHAR_H


#include <vector>
#include <bitset>
#include <limits>
#include <string>


typedef unsigned char TaggedChar_t;
typedef std::vector<TaggedChar_t> TaggedString;


#define TAGGED_CHAR_END 256  // # elements in an array indexed by TaggedChar_t

#define FIRST_PRINTING_CHAR 0x20        // space
#define LAST_PRINTING_CHAR  0x7E        // ~


enum Wildcard_t
{
    WC_QM,                  // ? - matches 1 untagged char, unique
    WC_DOT,                 // . - matches 1 untagged char, unique
    WC_DS,                  // $ - matches 0 or more untagged chars, unique
    WC_PCT,                 // % - matches 0 or more untagged chars, unique
    WC_STAR,                // * - matches 0 or more tagged or untagged chars, nonunique

    WC_END
};


TaggedChar_t ToTaggedChar( char c );            // set the tag bit

TaggedChar_t ToUntaggedChar( char c );          // clear the tag bit

char FromTaggedChar( TaggedChar_t tc );         // clear the tag bit

bool IsTagged( TaggedChar_t tc );               // is tag bit iset

TaggedChar_t GetWildcardChar( Wildcard_t wc );  // wildcards are tagged


// if this is a wildcard char, returns it otherwise returns WC_END
Wildcard_t ToWildcard( TaggedChar_t tc );

bool IsWildcard( TaggedChar_t tc );             // is this a wildcard?

bool WildcardMatches1Char( Wildcard_t wc );

bool WildcardMatchesString( Wildcard_t wc );

bool WildcardMatchesOnlyUntagged( Wildcard_t wc );

bool WildcardMatchesAny( Wildcard_t wc );

bool WildcardIsUnique( Wildcard_t wc );

// sets bitset bs to the characters contained in tagged string ts
void TaggedCharsUsed( std::bitset<TAGGED_CHAR_END> & bs,
                      const TaggedString & ts );

// Prints a tagged string.
// Tries to chose a delimiter which is not used in the string, but
// if all delimiters are used, prints in double quotes.
// All tagged chars in the string will be written without the tag,
// and all untagged chars will be preceeded by a backslash.
void PrintTaggedString( std::ostream & out,
                        const TaggedString & ts,
                        size_t max_length = std::numeric_limits<size_t>::max());

// returns a character from theStr, for debugging.
// If theIx isn't a valid index returns "?" else returns "'T" or "'\n'"
// where T is a tagged char and n is a non-tagged char.
std::string TaggedStringChar( const TaggedString & theStr,
                              int theIx );

#endif // TAGGED_CHAR_H
