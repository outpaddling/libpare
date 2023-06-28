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
#include <stdlib.h> /* Eliminates strange error in stdlib.h for COHERENT */
#include <ctype.h>
#include "pare.h"

/**************************************************************************
 * Title: re_match()
 * Author: Jason Bacon
 * Created: 12-29-97
 * Modification history:
 * Arguments:
 *      cre:        A compiled regular expression
 *      start:        The starting address of a string to be matched
 *      line_base:  Pointer to beginning of line.  Updated by
 *                  atom_match() to beginning of last line in token
 *      end_ptr:    Used to return pointer to end of token
 *      esize:  Size of an object in the list of lines
 * Return values:
 *      A positive return value indicates the number of characters matched
 *      from the starting point.
 *      A negative value to indicates an error.
 * Description:
 **************************************************************************/

#ifdef DEBUG
    char    *Pattern_name[] = 
		{ "CHAR_LITERAL", "STRING_LITERAL", "ANY", "SET",
		    "BEGIN_LINE", "END_LINE" },
	    *Rep_name[] = 
		{ "ONE", "ZERO_OR_ONE", "ZERO_OR_MORE", "ONE_OR_MORE" };
#endif

int     atom_match(
    piece_t *piece,
    char  **cur_char_ptr,
    char ***line_base_ptr,
    size_t  size)

{
    char    *cur_char = *cur_char_ptr,
	    **line_base = *line_base_ptr;
    int     match = FALSE,
	    ch;

#ifdef DEBUG
    fprintf(stderr,"Matching: %s  ",Pattern_name[piece->pattern_type]);
#endif
    
    /*
     * If end of string array, return now since switch below
     * can't handle NULL cur_char.
     */
    if ( cur_char == NULL )
	return FALSE;

    if ( *cur_char == '\0' )
	ch = ' ';
    else
	ch = *cur_char;
    
    /* Try to match one piece of the branch.
     * Note: The cases are ordered statistically, so that the first
     *      (ANY) is the most commonly found.  (ANY applies to most
     *      of the characters in a string or comment for many languages.
     *      This reordering resulted in about 20% increase in performance
     *      over the previous case ordering, which had ANY as the last
     *      case, and BEGIN_LINE (almost never used) as the first.
     */

    switch (piece->pattern_type)
    {
    /* Match any character, denoted by '.' in RE */
    case ANY:
	/* Skip \ for escaped chars */
	if ( ch == '\\' )
	    ++cur_char;
	match = TRUE;
	break;
    
    case CHAR_LITERAL:
	/* Match char exactly, unless escaped with '\' */
	if (ch == piece->atom.ch)
	    match = TRUE;
	break;
    
    case STRING_LITERAL:
	if ( (strlen(cur_char) >= piece->string_len) &&
	     (memcmp(cur_char, piece->atom.string, piece->string_len) == 0) )
	{
	    cur_char += piece->string_len;
	    match = TRUE;
	}
	break;
    
    /* Match any of a set of characters, denoted by [] in RE */
    case SET:
	if ( ((cur_char == *line_base) && IS_MEMBER(piece,SET_BOLN)) ||
	     ((*cur_char == '\0') && IS_MEMBER(piece,SET_EOLN)) )
	    return TRUE;    /* Don't advance cur_char */
	
	if (IS_MEMBER(piece,*cur_char))
	    match = TRUE;
	else
	    return FALSE;   /* Don't advance after a mismatch. */
	break;

    /* Match beginning of line marker '^' */
    case    BEGIN_LINE:
	if (cur_char == *line_base)
	    return TRUE;    /* Don't advance cur_char */
	break;
    
    case    END_LINE:
	if (*cur_char == '\0')
	    return TRUE;    /* Don't advance cur_char */
	break;
    }
    
    /* Advance to next char */
    if ( *cur_char == '\0' )    /* End of this line? */
    {
	/* Advance to next line.  Since line_base points to a pointer,
	   we must cast to (char *) to add size and not 
	   size * sizeof(char *) */
	*line_base_ptr = line_base = (char **)((char *)line_base + size);
	*cur_char_ptr = cur_char = *line_base;
    }
    else
	*cur_char_ptr = ++cur_char;

    /* Return statement for default case - END_LINE */
    return match;
}

 
int     pare_match(
    cre_t   *cre,
    char    *start,
    char    **list,         /* Actually a void pointer */
    size_t  esize,
    reloc_t *end)

{
    int         count, bcount, branches = cre->branch_count;
    branch_t    *cur_branch;
    
#ifdef DEBUG
    fprintf(stderr,"re_match(): RE has %d branches.\n",branches);
#endif

    /* Try to match each branch of the RE until out of branches or matched */
    /* Need a more efficient way to choose branches! */
    for (bcount=0; bcount<branches; ++bcount)
    {
	cur_branch = cre->branches+bcount;
	/* Don't waste time calling branch_match() unless the current
	   character is a valid starter for the current branch */
	if ( IS_BRANCH_STARTER(cur_branch->pieces[0],*start) )
	{
	    count = branch_match(cur_branch,start,list,esize,end);
	    if ( count > 0 )
		return count;
	}
    }
    
    /* Tried every branch with no luck */
    return NO_MATCH;
}


/*
 * Match a single branch of the RE
 */
 
int     branch_match(
    branch_t    *cur_branch,
    char    *start,
    char    **list,
    size_t  esize,
    reloc_t *end)

{
    size_t  total = 0;
    int     c, mismatch = FALSE, m1, m2;
    piece_t *cur_piece;
    char    *cur_char = start, *temp_ch1, **temp_lb1,
	    *temp_ch2, **temp_lb2, **line_base = list;

#ifdef DEBUG
    fprintf(stderr,"branch_match(): Branch has %d pieces.\n",
	cur_branch->piece_count);
#endif

    /* Match each piece of the RE one at a time */
    for (c=0, cur_piece=cur_branch->pieces; 
	    !mismatch && (c<cur_branch->piece_count); ++c, ++cur_piece)
    {
#ifdef DEBUG
	fprintf(stderr,"branch_match(): Piece has rep flag %d\n",
		cur_piece->reps);
#endif
	switch(cur_piece->reps)
	{
	    case    ONE:            /* Match one character */
		if ( !atom_match(cur_piece,&cur_char,&line_base,esize) )
		    mismatch = TRUE;
		else
		    INC_TOTAL(total,cur_piece);
		break;

	    /* Count modifier '?'. No mismatch possible. */
	    case    ZERO_OR_ONE:    
		/* Don't update cur_char or line_base unless atom
		   is matched as ONE */
		temp_ch1 = cur_char;
		temp_lb1 = line_base;
		if ( atom_match(cur_piece,&temp_ch1,&temp_lb1,esize) )
		{
		    cur_char = temp_ch1;
		    line_base = temp_lb1;
		    INC_TOTAL(total,cur_piece);
		}
		break;
	    
	    /* Match one piece, then fall into zero_or_more */
	    case    ONE_OR_MORE:    /* count modifier '+' */
		if ( !atom_match(cur_piece,&cur_char,&line_base,esize) )
		{
		    mismatch = TRUE;
		    break;
		}
		else
		    INC_TOTAL(total,cur_piece);
		    /* Fall into ZERO_OR_MORE to continue */

	    /* Count modifier '*'. No mismatch possible. */
	    case    ZERO_OR_MORE:
		/* If no more pieces after this one, match as many as
		   possible.  Note: cur_piece & pieces are pointers */
		if (cur_piece - cur_branch->pieces == cur_branch->piece_count-1)
		{
		    while ( atom_match(cur_piece,&cur_char,&line_base,esize) )
			;
		    break;
		}
		
		/* If more pieces, match the zero-or-more piece until
		   a mismatch, or until the next piece in the RE is matched */
		temp_ch1 = temp_ch2 = cur_char;
		temp_lb1 = temp_lb2 = line_base;
		while ( (*line_base != NULL) &&
			((m1=atom_match(cur_piece,&temp_ch1,&temp_lb1,esize)) &&
			(!(m2=atom_match(cur_piece+1,&temp_ch2,&temp_lb2,
			esize)) )))
		{
		    /*
		     * temp_ch1 is always advanced, and temp_ch2 is never advanced,
		     * so make temp_ch1 the starting position for the next round.
		     */
		    cur_char = temp_ch2 = temp_ch1;
		    line_base = temp_lb2 = temp_lb1;
		    
		    /* Count characters matched */
		    INC_TOTAL(total,cur_piece);
		}
		break;

	    default:
		return INVALID_REP;
	}
    }
    
    /* If we got through the branch OK, return the match count. */
    if ( !mismatch )
    {
	/* Return position of the last character in the matched text */
	if ( end != NULL )
	{
	    if ( cur_char == *line_base )
	    {
		char    *temp;
		
		/* Back up to previous line (line_base - esize bytes) */
		temp = *(char **)((char *)line_base - esize);
		end->end_col = strlen(temp) - 1;
		end->end_char = temp + end->end_col;
		end->lines = (line_base - list) * sizeof(char *) / esize;
	    }
	    else
	    {
		end->end_char = cur_char - 1;
		/* line_base and list are artificially defined as char **
		   when they really could be pointers to anything, hence
		   the funny arithmetic here */
		end->lines = (line_base - list) * sizeof(char *) / esize + 1;
		end->end_col = end->end_char - *line_base;
	    }
	}

	/* Add one little rule: If a token begins or ends in a letter, then
	   it can't be immediately preceded or followed by a letter or _.
	   This prevents matching "int" inside "printf", for example.
	   If it begins with a number, it also cannot be preceded by a
	   letter or _. This prevents 1 from being matched inside x1 */
#ifdef DEBUG
	fprintf(stderr,"cur_char = %p, *line_base = %p, start = %p\n",
		cur_char,*line_base,start);
	fflush(stderr);
#endif
	/* Temp to avert core dump after typing #!/bin/csh */
	if ( cur_char == NULL )
	    return NO_MATCH;
	    
	if ((ISIDENT(*cur_char) &&
	    (cur_char != *line_base) && (isalpha(cur_char[-1])) ) ||
	    (isalnum(*start) &&
	    (start != *line_base) && ISIDENT(start[-1])) )
	    return NO_MATCH;
	else
	    return total;
    }
    else
	return NO_MATCH;
}

