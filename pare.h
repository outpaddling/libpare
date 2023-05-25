/*
    Copyright (c) 1994-2003, Jason W. Bacon, Acadix Software Systems
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer. Redistributions
    in binary form must reproduce the above copyright notice, this list of
    conditions and the following disclaimer in the documentation and/or
    other materials provided with the distribution. 

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#define _PARE_H_

#ifndef _STDLIB_H_
#include <stdlib.h>
#endif

#define STR_MAX 1024

enum { CHAR_LITERAL, STRING_LITERAL, ANY, SET, BEGIN_LINE, END_LINE };
enum { ONE, ZERO_OR_ONE, ZERO_OR_MORE, ONE_OR_MORE };

#ifndef FALSE
#define FALSE   0
#endif

#ifdef TRUE
#undef TRUE
#endif
#define TRUE    -1

#define PARE_MALLOC(count,type) ((type *)malloc((count) * sizeof(type)))

#ifndef ISIDENT
#define ISIDENT(c)  ( isalnum(c) | ((c)=='_') )
#endif

/* pare_compile() errors */
#define RE_BAD_RANGE    -1
#define RE_BAD_SET      -2
#define RE_BAD_STRING   -3

/* pare_match() errors */
#define NO_MATCH        -1
#define INVALID_REP     -2

/*
 * See "man pare_format" on FreeBSD for a complete description of the
 * POSIX regular expression format.
 */

/*
 * A piece is a portion of a regular expression which is treated as
 * a unit during matching.  For example, the expression
 * [a-zA-Z_][a-zA-Z0-9_]*, which matches variable names in most languages,
 * contains two pieces:
 * [a-zA-Z_] is the first piece, and [a-zA-Z0-9_]* is the second piece.
 * pare_match() attempts to match each piece in sequence, and returns the
 * number of characters matched if all the pieces can be used up.
 */
 
typedef struct
{
    int     pattern_type;   /* Exact char, any char, string, or set */
    size_t  reps;           /* 1, 0 or 1, 0 or more, 1 or more */
    size_t  string_len;     /* Length of literal string */
    union
    {
	int     ch;         /* Single literal character */
	char    *string;    /* (xyz) */
	char    *ismember;  /* See macros below */
    }   atom;
}   piece_t;

/* Constants for ISMEMBER() lookup table */
#define MAX_SET_MEMBERS         258
#define ISMEMBER_TABLE_SIZE     (MAX_SET_MEMBERS/8 + 1)

/* Special symbols for sets */
#define SET_EOLN    256
#define SET_BOLN    257

/* Set individual bits in 32 character ismember array, for a total
   of 256 set membership flags */
#define ISMBYTE(c)  ((c) >> 3)          /* Byte = c / 8 */
#define ISMBIT(c)   (1 << ((c) & 7))    /* Bit = c mod 8 */
#define IS_MEMBER(p,c)  ((p)->atom.ismember[ISMBYTE(c)] & ISMBIT(c))
#define SET_MEMBER(p,c) ((p)->atom.ismember[ISMBYTE(c)] |= ISMBIT(c))

/*
 * Branches are sections of the regular expression separated by
 * the | (OR) symbol.  For example, the expression [0-9]+|[0-9]*\.[0-9]+,
 * which matches decimal numbers, contains two branches:
 * [0-9]+ and [0-9]*\.[0-9]+
 * The | symbol must therefore be escaped to be used inside a piece.
 * i.e. | separates branches, whereas \| is a literal '|' character
 * | is also taken literally inside a string such as (|)
 */
 
typedef struct
{
    size_t  piece_count;
    piece_t *pieces;
}   branch_t;

typedef struct
{
    size_t      branch_count;
    branch_t    *branches;
    char        starter[32];   /* starter[c] == 1 if c is a valid starter */
}   cre_t;

typedef struct
{
    char    *end_char;
    size_t  lines;
    size_t  end_col;
}   reloc_t;

#define INIT_PIECES     10
#define MORE_PIECES     5
#define INIT_BRANCHES   10
#define MORE_BRANCHES   10

#define INC_TOTAL(t,p)\
    if ( (p)->pattern_type == STRING_LITERAL )\
	(t) += (p)->string_len;\
    else\
	(t)++

/* pare_match macros */
#define IS_RE_STARTER(cre,c) ((cre)->starter[ISMBYTE(c)] & ISMBIT(c))
	 
#define SET_STARTER(cre,c) ((cre)->starter[ISMBYTE(c)] |= ISMBIT(c))

#define IS_BRANCH_STARTER(p,c)\
	( ((p).pattern_type == ANY) ||\
	  (((p).pattern_type == CHAR_LITERAL) && ((p).atom.ch == (c))) ||\
	  (((p).pattern_type == SET) && IS_MEMBER((&p),(c))) ||\
	  ((p).pattern_type == BEGIN_LINE) ||\
	  ((p).pattern_type == END_LINE) ||\
	  (((p).pattern_type == STRING_LITERAL) && (*(p).atom.string == (c))))

#ifdef SCO_SV
void    *alloca(size_t);
#endif

/* atom_match.c */
int atom_match (
	piece_t *piece, 
	char **cur_char_ptr, 
	char ***line_base_ptr, 
	size_t size);

/* pare_comp.c */
int pare_compile (
	cre_t *cre, 
	const char *pattern_arg);
void init_starter_set (
	cre_t *cre,
	branch_t *branch);

/* pare_free.c */
void pare_free (
	cre_t *cre);

/* pare_match.c */
int pare_match (
	cre_t *cre, 
	char *start, 
	char **list, 
	size_t esize, 
	reloc_t *end);
int branch_match (
	branch_t *cur_branch, 
	char *start, 
	char **list, 
	size_t esize, 
	reloc_t *end);
int odd_count_esc (
	char *,char *);
int count_branches (
	char *);
#undef __PROTO

