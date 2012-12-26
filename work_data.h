// FILE: work_data.h
//
// DESCRIPTION:
//      Defines class WorkData, which contains the data and low level
//      data manipulation functions for the Work class.
//
//      There are several different kinds of data managed by this class,
//      and they are logically connected.
//
//      To String:          This is the string being constructed during
//                          this transformation.
//
//      From String:        This is the working string which is being
//                          pattern matched.  Last time through the main
//                          Work loop it was the To String.
//
//      The To String and From String are actually stored in strings
//      myWorkA and myWorkB, and myFromStringIsA identifies which of these
//      is the From String and which is the To String.
//      Note that before each pass the To string is moved to the From string
//      and the To string is cleared.  Instead of actually moving the strings
//      (which could be large), the myFromStringIsA is complemented.
//
//      The From String has the following sub-data items:
//
//          myFromStringCharsUsed:  This is a bit set of characters,
//                                  which indicates whether a particular
//                                  is somewhere in the From String.  This
//                                  permits rapid determination of whether
//                                  a pattern could be matched by a string:
//                                  if not all non-wildcard chars in the
//                                  pattern are in the From String, it
//                                  can't match.
//
//          myFromStringCFirst:     This is a vector indexed by char which
//                                  indicates the first position of that
//                                  character in From String, or -1 if not used.
//
//          myFromStringCNext:      This is an int vector with as many elements
//                                  as there are characters in the From String,
//                                  and indicates the position of the next
//                                  occurrence of this character.  This permits
//                                  rapid searching for pattern fragments in
//                                  the From Str, since we only need to check
//                                  substrings beginning with the first character
//                                  of the pattern fragment, and can advance the
//                                  From String pointer directly to the next
//                                  occurrence of this starting char if no match.
//
//          myFromStringWildcards:  This is a vector whose elements
//                                  describe substrings in From String
//                                  which have been matched by wildcards
//                                  in the pattern.
//
//          myFirstWildcardOccurrence:  
//                                  This is a vector indexed by wildcard type
//                                  which contains the pointer in From String
//                                  of the first occurrence of that wildcard,
//                                  or -1 if not found yet (or not in pattern).
//                                  This is for quickly determining whether
//                                  a unique wildcard has been seen before in
//                                  this substring, and if so, where it is
//                                  so it can be quickly compared.
//
//      myCurPat:               This is a pointer to the current pattern
//                              string we are doing pattern matching on.
//                              If we call GetCurPat() and there is no
//                              pattern string set, we will assert.
//                              The pattern string has these sub-data items:
//                  
//          myPatternFragments:     This is a vector which describes the
//                                  start and length in the pattern string
//                                  of each contiguous set of non-wildcard
//                                  characters, or of a single unique
//                                  wildcard character we have already seen
//                                  and whose position is fixed (by having
//                                  at least one non-wildcard fragment
//                                  between this fragment and the first
//                                  occurrence of the wildcard.  Once the
//                                  unique wildcard is fixed, we can find
//                                  its substring just like it was a
//                                  non-wildcard fragment.
//
//          myPatFragCurrentPos:    This is an int vector with elements that
//                                  correspond to the elements of
//                                  myPatternFragments, that indicates the
//                                  position of that fragment in the
//                                  From String, or -1 if not found yet.
//
// HISTORY:
//      14-DEC-12   D.Brown     Created

#ifndef WORK_DATA_H
#define WORK_DATA_H


#include "tagged_char.h"
#include <vector>
#include <bitset>
#include <iostream>
#include <string>



struct WildcardOccurrence;


// This identifies either a set of contiguous non-wildcard characters
// in the current pattern, or a single already-matched unique wildcard char.
struct PatternFragment
{
    int     myStart;
    int     myLength;       // -1 means this is a wildcard char
};


class WorkData
{
public:
    WorkData();

    ~WorkData();

private:
    WorkData( const WorkData & theOther );

    const WorkData & operator = ( const WorkData & theOther );

public:
    const TaggedString & GetFromStr();

    const std::bitset<TAGGED_CHAR_END> & GetFromStrCharsUsed();

    // return index in first string of first occurrence of char tc,
    // or -1 if there are no occurrences.
    int GetFromStrCFirst( TaggedChar_t tc );

    // returns index in first string of the next occurrence of
    // the character at index i, or -1 if there are no next chars
    int GetFromStrCNext( int i );

    void GetSubstringOfFromStr( TaggedString & theStr,
                                int theStartIx,
                                int theLen );

    const TaggedString & GetToStr();

    void ClearWildcardOccurrences();

    int GetNumWildcardsUsed() const;

    void GetWildcardSubstring( int theWildcardOcccurence,
                               TaggedString & theSubstring );

    void GetWildcardSubstringInfo( int theWildcardOccurrence,
                                   int & theStartIx,
                                   int & theLength );

    Wildcard_t GetWildcardType( int theWildcardOccurrence );

    // if this wildcard hasn't been seen before, returns -1
    // otherwise returns the occurrence # of the first occurrence
    int GetFirstWildcardOccurrence( Wildcard_t theWildcardType );

    // before first call, theOccurrence should be -1.
    // Gets the next wildcard occurrence of this kind of wildcard,
    // wrapping around at the end of the array.  If there are no
    // occurrences of this wildcard, will return theOccurrence = -1.
    void GetNextWildcardOccurrence( Wildcard_t theWildcardType,
                                    int & theOccurrence );

    // Found a new occurrence of theWildcardType in the from string.
    // If this wildcard type has already been found, compare its string
    // with this string.  Returns true if they match, or if this is
    // the first occurrence of this type.
    bool CompareSubstringWithWildcard( Wildcard_t theWildcardType,
                                       int        theStartIx,
                                       int        theLength );

    // Stores an occurrence of a wildcard of type theWildcardType which
    // matches the From String substring starting at theStartingIndex and
    // of size theSize.
    void FoundWildcard( Wildcard_t theWildcardType,
                        int        theStartingIndex,
                        int        theSize );

    // Moves the To string to the From String, clears the To String and
    // all of the other attributes associated with the From String.
    void MoveToStringToFromString();

    // Clear the From String and all associated attributes
    void ClearFromString();

    void ClearToString();

    void AppendCharToToString( TaggedChar_t theChar );

    void AppendStringToToString( const TaggedString & theStr );

    void AppendWildcardOccurrenceToToString( int wildcard_occurrence );

    // used to backtrack during pattern matching of the from string.
    // deletes any wildcard occurrences whose substrings are not
    // fully contained in the first theNewMatchLength chars of from string.
    void UnmatchFromString( int theNewMatchLength );

    // Sets the current pattern to thePattern.
    void SetCurrentPattern( const TaggedString & thePattern );

    // Sets the current pattern to nothing.
    void UnrefCurrentPattern();

    // Gets the current pattern string, or asserts if there is no 
    // current pattern.
    const TaggedString & GetCurPat() const;

    // Returns true if there is a current pattern set by SetCurrentPattern
    // or false if the current pattern has been unreferenced by 
    // UnrefCurrentPattern.
    bool HasCurPat() const;

    // Gets the number of fragments the pattern has been split into.
    // Note that gaps containing wildcards before the fragments are not counted.
    int GetNumPatternFragments() const;

    // returns the first character of the fragment or the
    // substring matched by the wildcard if the fragment is a wildcard.
    // if length of fragment is 0, or this is a wildcard which
    // hasn't been matched yet, returns (TaggedChar_t)0
    TaggedChar_t GetPatFragFirstChar( size_t frag_ix );

    // If this is a wildcard fragment, returns the start & len
    // of the matched substring, or start=-1 and len=-1;
    // If this is a normal fragment, returns the start & len of
    // the substring from the myPatternFragments array,
    // which means that it must be already matched.
    // returns true if start and len are >= 0.
    bool GetPatFragStartLengthInFromStr( size_t frag_ix,
                                         int & start,
                                         int & len );

    // If this is a wildcard fragment, returns the length of the
    // matched substring, or -1 if there is no occurrence for this
    // wildcard yet.  If this is a normal fragment, return the length
    // of the fragment in the pattern.
    int GetPatFragLengthInFromStr( size_t frag_ix );

    // Returns the first character of the fragment in the pattern
    // even if it is a wildcard fragment (returns the wildcard char)
    TaggedChar_t GetPatFragFirstCharInPat( size_t frag_ix );

    // Returns the starting position of the fragment in the pattern string.
    int GetPatFragStartInPat( size_t frag_ix );

    // Gets the length of the fragment in the pattern string.
    // If this fragment is a single wildcard character, returns -1.
    int GetPatFragLengthInPat( size_t frag_ix );

    // Returns true if fragment frag_ix is a single wildcard character.
    // Returns false if fragment frag_ix is a substring in the pattern.
    bool PatFragIsWildcard( size_t frag_ix );

    void ClearAllPatFragPosInFromStr();

    int GetPatFragPosInFromStr( size_t frag_ix );

    // Advances the substring matched by pattern fragment frag_ix
    // to the next matching position in the from string which is
    // >= fromstr_min_pos.  The pattern fragment may be in the pattern
    // or an already matched wildcard.  Returns true if a match was found.
    // Store the new position in myPatFragCurrentPos[frag_ix].
    bool AdvancePatFragPos( size_t frag_ix,
                            int fromstr_min_pos );

    // Verifies that the substring matched by pattern fragment frag_ix
    // matches the From String starting at index fromstr_ix.
    // The pattern fragment may be in the pattern or an already 
    // matched wildcard.  If match, store the position in 
    // myPatFragCurrentPos[frag_ix].
    bool VerifyPatFragPos( size_t frag_ix,
                           int fromstr_pos );

    // Compares substring in FromString from index ss_start of length ss_len
    // with the fragment, which may be in the pattern or an
    // already matched wildcard.  Returns true if they match.
    bool CompareSubstringWithFragment( int ss_start,
                                       int ss_len,
                                       size_t frag_ix );

    // Clears the prefix and suffix strings
    void ClearPrefixAndSuffix();

    // Sets the prefix and suffix strings to the portion of the From String
    // not matched by the pattern, which is everything before first_match_ix
    // or after last_match_ix.
    void SetPrefixAndSuffix( int first_match_ix,
                             int last_match_ix );

    // Returns the prefix and suffix strings.  The prefix string always starts
    // at index 0.  The suffix string always ends at the end of the From String.
    void GetPrefixAndSuffix( int & prefix_len,
                             int & suffix_start,
                             int & suffix_len );

private:
    // gets myFromStringCharsUsed, myFromStringCNext,
    // and myFromStringCNext and sets myFromStringHasInfo to true
    void GetFromStringInfo();

private:
    TaggedString myWorkA;
    TaggedString myWorkB;

    bool myFromStringIsA;

    std::bitset<TAGGED_CHAR_END> myFromStringCharsUsed;
    std::vector<int> myFromStringCNext;
    int myFromStringCFirst[TAGGED_CHAR_END];
    bool myFromStringHasInfo;

    // identifies substrings of myFromStr which are matched by wildcards
    std::vector<WildcardOccurrence> myFromStringWildcards;
    int myFirstWildcardOccurrence[WC_END+1];  // -1 if no wildcards of this type yet

    const TaggedString * myCurPat;            // current pattern

    // spans of contiguous non-wildcard characters in current pattern
    // or single already matched wildcard characters in pattern
    std::vector<PatternFragment> myPatternFragments;

    PatternFragment myPrefix;
    PatternFragment mySuffix;

    // positions in myFromStr of each fragment in myPatternFragment
    std::vector<int> myPatFragCurrentPos;   // -1 if not known yet
};



#endif // WORK_DATA_H
