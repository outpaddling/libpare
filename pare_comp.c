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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "pare.h"

/****************************************************************************
 *
 * Author:  Jason Bacon
 * Purpose: Compile a regular expression
 * Similar functions and their authors: regcomp
 * Created: Jan, 1998
 * Return values: 0 on success, error codes defined in pare.h
 * Description:
 *
 ****************************************************************************/

#define CLEAN_RETURN(v) {\
	    free(pattern);\
	    return (v);\
	}

int     pare_compile(cre,pattern_arg)
cre_t   *cre;
const char    *pattern_arg;

{
    branch_t    *cur_branch;
    piece_t *cur_piece;
    size_t  pcount,
	    max_pieces = INIT_PIECES,
	    bcount, total_branches;
    int     c, len;
    char    *pattern, *start_str, *p;
    
#ifdef DEBUG
    char    *Pattern_name[] = 
		{ "CHAR_LITERAL", "STRING_LITERAL", "ANY", "SET",
		    "BEGIN_LINE", "END_LINE" },
	    *Rep_name[] = 
		{ "ONE", "ZERO_OR_ONE", "ZERO_OR_MORE", "ONE_OR_MORE" };
    printf("Compiling RE...\r\n");
    printf("sizeof(branch_t) = %d, sizeof(piece_t) = %d\r\n",
	    sizeof(branch_t),sizeof(piece_t));
#endif

    /* Copy pattern to temp string for preservation */
    pattern = strdup(pattern_arg);

    /* Get branch count and allocate the exact amount of mem needed */
    total_branches = count_branches(pattern);
    cre->branches = PARE_MALLOC(total_branches,branch_t);
    
    /* Starter set - flags all characters that can start the RE */
    memset(cre->starter,FALSE,32);
    
    /* Compile each branch of the RE */
    for (cur_branch = cre->branches, bcount=0, p=pattern;
	*p != '\0'; ++cur_branch, ++bcount)
    {
#ifdef DEBUG
	printf("Compiling branch #%d...\n",bcount);
#endif
	/* Initial allocation */
	cur_branch->pieces = PARE_MALLOC(max_pieces,piece_t);
    
	/* Compile each piece of the branch */
	for (cur_piece=cur_branch->pieces, pcount=0;
		(*p != '\0') && (*p != '|'); ++cur_piece, ++pcount)
	{
	    if ( pcount == max_pieces )
	    {
		max_pieces += MORE_PIECES;
		cur_branch->pieces = (piece_t *)realloc(cur_branch->pieces,
				max_pieces*sizeof(piece_t));
		cur_piece = cur_branch->pieces+pcount; /* Fix cur pointer to new array */
	    }
	    
	    /* Get next piece */
	    switch(*p)
	    {
		case    '[':    /* Create set piece */
		    cur_piece->pattern_type = SET;
		    ++p;
		    cur_piece->atom.ismember = 
			calloc(ISMEMBER_TABLE_SIZE,sizeof(char));
		    SET_MEMBER(cur_piece,*p);
		    ++p;
		    while ( (*p != '\0') && (*p != ']') )
		    {
			switch(*p)
			{
			    case    '-':
				if ( p[1] != '\0' )
				{
				    for (c=p[-1]; c<=p[1]; ++c)
					SET_MEMBER(cur_piece,c);
				    ++p;
				}
				else
				{
				    CLEAN_RETURN(RE_BAD_RANGE);
				}
				break;
			    
			    /* End of line */
			    case    '$':
				SET_MEMBER(cur_piece,SET_EOLN);
				break;
			    
			    case    '^':
				SET_MEMBER(cur_piece,SET_BOLN);
				break;
				
			    /* Escaped special character */
			    case    '\\':
				++p;
				if ( *p != '\0' )
				    SET_MEMBER(cur_piece,*p);
				else
				    CLEAN_RETURN(RE_BAD_SET);
				break;
			    default:
				SET_MEMBER(cur_piece,*p);
			}
			++p;
		    }
		    if ( *p != ']' )
			CLEAN_RETURN(RE_BAD_SET);
		    break;
		case    '(':
		    cur_piece->pattern_type = STRING_LITERAL;
		    ++p;
		    start_str = p;
		    len = 0;
		    while ( (*p != ')') && (*p != '\0') )
		    {
			++p;
			++len;
		    }
		    if ( *p != ')' )
			CLEAN_RETURN(RE_BAD_STRING);
		    cur_piece->atom.string = 
			(char *)malloc(len*sizeof(char)+1);
		    memcpy(cur_piece->atom.string,start_str,len*sizeof(char));
		    start_str[len] = '\0';
		    cur_piece->string_len = len;
		    break;
		case    '.':
		    cur_piece->pattern_type = ANY;
		    break;
		case    '$':
		    cur_piece->pattern_type = END_LINE;
		    break;
		case    '^':
		    cur_piece->pattern_type = BEGIN_LINE;
		    break;
		case    '\\':   /* Fall through to char literal below */
		    ++p;
		default:
		    cur_piece->pattern_type = CHAR_LITERAL;
		    cur_piece->atom.ch = *p;
		    break;
	    }
	    ++p;
	    
	    /* Check for reps modifier */
	    switch(*p)
	    {
		case    '?':
		    cur_piece->reps = ZERO_OR_ONE;
		    ++p;
		    break;
		case    '*':
		    cur_piece->reps = ZERO_OR_MORE;
		    ++p;
		    break;
		case    '+':
		    cur_piece->reps = ONE_OR_MORE;
		    ++p;
		    break;
		default:
		    cur_piece->reps = ONE;
		    break;
	    }
#ifdef DEBUG
	    printf("Piece: %s, %s\r\n",
		    Rep_name[cur_piece->reps],
		    Pattern_name[cur_piece->pattern_type]);
	    if ( cur_piece->pattern_type == CHAR_LITERAL )
		printf(" '%c'",cur_piece->atom.ch);
	    printf("\r\n");
	    fflush(stdout);
#endif
	}
	if ( *p == '|' )
	    ++p;
	cur_branch->piece_count = pcount;

	/* Trim off piece allocation */
	cur_branch->pieces = (piece_t *)realloc(cur_branch->pieces,
				pcount*sizeof(piece_t));
	
	/* Add starter characters ffor this branch to starter set */
	init_starter_set(cre,cur_branch);
    }
    cre->branch_count = bcount;
    CLEAN_RETURN(0);
}


void    init_starter_set(cre,branch)
cre_t   *cre;
branch_t    *branch;

{
    int     c;
    
    switch(branch->pieces[0].pattern_type)
    {
	case    CHAR_LITERAL:
	    SET_STARTER(cre,branch->pieces[0].atom.ch);
	    break;
	case    STRING_LITERAL:
	    SET_STARTER(cre,branch->pieces[0].atom.string[0]);
	    break;
	case    ANY:
	case    BEGIN_LINE:
	case    END_LINE:
	    memset(cre->starter,TRUE,32); /* TRUE should be -1 here */
	    break;
	case    SET:
	    for (c=0; c<256; ++c)
		if ( IS_MEMBER(&branch->pieces[0],c) )
		    SET_STARTER(cre,c);
    }
}


int     count_branches(char *pattern)

{
    char    *p;
    int     total_branches;
    
    for (p=pattern+1, total_branches = 1; *p != '\0';)
    {
	switch(*p)
	{
	    case    '\\':   /* Ignore \| literal */
		if ( *++p != '\0' )
		    ++p;
		break;
	    case    '[':    /* Ignore | in a set */
		while ( (*p != ']') && (*p != '\0') )
		    ++p;
		if ( *p == ']' )
		    ++p;
		break;
	    case    '|':
		++total_branches;
		++p;
		break;
	    default:
		++p;
		break;
	}
    }
    return total_branches;
}

