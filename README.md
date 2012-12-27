Markov
======

Markov is a string transformation language, written in C++ for the PltGames TuringTarpit.

This program uses string transformations to convert an input string into an output string.  
The input program is a series of transformations like:

    ; reverse a string
    "*"      -> "a[*]"    ; starting transformation (matched once)
    "b[]*"   -> "*"       ; terminating transformation

    "a[*~*]" -> "a[*]"    ; throw away all but the first line
    "a[\ *]" -> "a[*]"    ; throw away leading blanks
    "a[*\ ]" -> "a[*]"    ; throw away trailing blanks
    "a[*]"   -> "b[*]"    ; change to state b
    "b[?$]*" -> "b[$]?*"  ; state b: reverse characters until done

For each transformation, The string before the -> is the pattern string, the string after 
the -> is the replacement string.  The pattern string and the replacement string may use as 
delimiters single quotes like 'abc', double quotes like "abc", or vertical bars like |abc|.   
The delimiter may not appear inside the string, and the pattern and replacement strings may 
not contain end of lines (but may contain the "~" character, which matches an end-of-line in 
the input file).  Comments start with a ";" character and extend to the end of the line.  
Any number of spaces, end of lines or comments may appear before or after the match or 
replacement strings.

At each transformation step, the result of the previous transformation 
(which is the input to the next transformation) is called the working 
string, to distinguish it from the original input string.

The input string may consist only of printing characters between ASCII 
code 32 (blank) and 126 "~", plus end-of-line.   Tabs are converted to 
spaces.   Any other ASCII characters are ignored.   The eighth bit of 
each character is used as a tag bit:  all chararacters in the match 
or replacement strings which are preceeded by a backslash \ are literal 
characters, which match untagged characters in the working string.  
All other characters in the match and replacement string are tagged, 
meaning they have the high byte set, and only match the corresponding 
tagged characters in the working string.
 
An end-of-line in the input string is converted to a tagged "~" 
character.  A tagged "~" character in the final output string is 
converted back to an end of line. 


PATTERN MATCHING:

The following five characters are wildcards: "? . * $ %".   
The "?" and "." wildcards match a single untagged character, and if 
they appear more than once in the pattern string each occurrence must 
be identical.   The "*" wildcard matches zero or more consecutive 
untagged or tagged characters.  If it occurs multiple times in the 
pattern string the occurrences don't have to match.  The "$" and "%" 
characters match zero or more consecutive untagged characters.  
If they appear more than once in the match string, the matched 
substrings must be identical.  An occurrence of a wildcard in the 
replacement string is replaced by the corresponding matched substring.
If there are more "*" wildcards in the replacement string than in 
the pattern string, after each substring matched by "*" is used once, 
the matched substrings will be reused in order.

If there are any "*", "$" or "%" wildcard characters in the pattern
before the first literal character, the matched string will begin
at the beginning of the working string, otherwise the matched string
will begin at (or just before) the first occurrence of the literal
character in the working string, assuming that the rest of the
string matches.  Likewise, if there are any "*", "$" or "%" wildcard
characters after the last literal character in the pattern, the
matched string will end at the end of the working string, otherwise
the matched string will end at (or just after) the character matched
by the last literal character in the pattern.

For example, transformation "\c\d*\g\h" -> "\C\D*\G\H" with input 
string "abcdefghij" will produce "abCDefGHij", as will transformation 
"*\c\d*\g\h*" -> "*\C\D*\G\H*".


ORDER OF TRANSFORMATIONS:

The first transformation in the program file is the starting 
transformation, which is only used once, on the original input string.
It must match, or the Markov program will report a failure.   The 
second transformation in the program file is the ending transformation.
At each step, if this match string matches the working string, the 
Markov program performs the replacement and reports the result as 
the output string.

The rest of the transformations are attempted in a top down manner.
If a transformation fails, the next transformation below is attempted.
If a transformation succeeds, it is repeated as many times as possible 
until it fails, then the program will start again at the top and 
attempt  the second (ending) transformation, and so forth.  If no 
transformation is matched, the MARKOV program will report a failure.

Other than the wildcard characters, the tagged characters in the match 
and replacement strings act as scaffolding to mark various fields and 
subfields in the working string.  Ideally, all the tagged characters 
should be removed by the time the ending transformation is made.  
If not, the tagged characters are written to the result string 
preceeded by a backslash.


MODES SPECIFIED ON THE COMMAND LINE:

The Markov program may be run in one of three modes, depending on the 
command line arguments.  The full file mode, which is the default, 
reads the entire input file into the input string (or from standard 
input), converts all end-of-lines to tagged "~" characters,
performs the transformations, and writes the output string 
to the output file, converting all tagged "~" characters to end-of-lines.

In Unit Test mode, the input file consists of pairs of lines, the 
first is used as the input string, and the second is the expected 
result, and is compared with the output string.  If the output 
string doesn't match the expected result, an error is reported.  
In Unit Test mode, any number of blank lines or lines containing 
comments (beginning with ";") will be skipped before the pair of 
input and expected output lines.

In Immediate mode, the contents of the input string are given on the 
command line, possibly in quotes, preceeded by the "-i" option.  Note
that on MS Windows, command line arguments cannot contain blanks, and
double quotes are passed to the program as part of the argument, so on 
MS Windows immediate mode is limited to single word inputs, not in quotes.

In Unit Test mode and Immediate mode, all "~" characters in the input 
line will be converted to end-of-lines in the input string (which will 
be converted to a tagged "~" character).  In Unit Test mode, all 
end-of-lines in the output string will be converted to "~" characters 
before comparing with the expected result line.


COMMAND LINE SYNTAX:

    <command_line> ::=
        "MARKOV" {<option>} <program_file_name>
        <input> <opt_output_file_name>

    <option> ::=
        "-test" |               ; unit test mode
        "-debug" |              ; debug mode: write to file markov.log
        "-verbose" |            ; verbose debug mode: write lots more
        "-console" |            ; console debug mode: write to stdout 
        "-print" |              ; print program and exit
        "-options" |            ; print options and exit
        "-help" |               ; print help text and exit
        "?"                     ; same as -help		

    <input> ::=
        <input_file_name>  |            ; read from this file
        -imm <immediate_input_string> | ; use this string as input
        <empty>                         ; read from standard input

    <opt_output_file_name> ::=
        <output_file_name> |
        <empty>                         ; write to standard output

The options may be abreviated to a dash followed by a single letter.
For example "-i" is the same as "-imm".
If the input filename is empty, the output filename must also be empty.

The program result code will be 0 for OK or 100 for any error, plus 
on error an error message will be written to standard error.


EXAMPLES:

The following examples are provided:

        reverse.mkv     - reverses the input string
        reverse_all.mkv - reverses each line of the input file
        add.mkv         - add two arbitrary precision unsigned 
                          integers, separated by a blank
        fib.mkv	        - computes the first N fibonacci numbers, 
                          where N is the input string.

There are unit test input files for each of these, with file prefix 
"ut_" and suffix ".txt" which can be run using the "-test" option.
For example, to test the "add.mkv" program with various inputs,  type command:

        ./markov -test add.mkv ut_add.mkv
        
To print the first 50 Fibonacci numbers, type command:

        ./markov fib.mkv -i 50
        

DEBUGGING:

In Debug Mode, a file "markov.log" will be created in the current 
working directory, which will contain the working string and the 
tranformation used (plus the number of times the transformation 
was used) for each transformation performed.  In Unit Test mode, 
the debug output file will be reset before each input line is 
processed, so when the program exits the debug output file will 
only contain the transformations for the last input line.

If both "-debug" and "-verbose" are specified, every attempted
transformation will be included in file "markov.log" as well as 
the transformations which succeeded.  If "-debug" and "-console"  
are specified (usually with "-verbose" specified as well) the 
debug information is also written to standard output, which can 
make it easier to use a debugger to debug the code.  Each output to 
the debug file will include the program instruction number and 
source line number, plus a unique id "uid" which is incremented 
each attempted transformation.  There is a constant BREAK_AT_UID
in file "work.cpp" which, if nonzero, means to call a breakpoint 
function after the specified attempted transformation.


DEVELOPMENT LANGUAGE:

The Markov program was developed using C++ for Microsoft Visual 
Studio 2010.  It was ported to Ubuntu Linux running gcc 
version 4.7.2.


BUILD INSTRUCTIONS (Linux using g++):

In the directory with the source files and Makefile, type:
    make
This creates file "markov".
To delete all the object files and the "markov" file before building, type:
    make clean

To build on Windows using Visual Studio C++, create a solution and
project, add the .cpp and .h files and build.
