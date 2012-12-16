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
#include "pare.h"

/**************************************************************************
 * Title:
 * Author:
 * Created:
 * Modification history:
 * Arguments:
 * Return values:
 *      TRUE if the character matches the atom
 *      FALSE of it doesn't
 * Description:
 *      Match a single string or char to an atom in the compiled RE.
 **************************************************************************/

int     atom_match(piece, cur_char_ptr, line_base_ptr, size)
piece_t *piece;
char  **cur_char_ptr;
char ***line_base_ptr;
size_t  size;

{
    char    *cur_char = *cur_char_ptr,
	    **line_base = *line_base_ptr;
    int     status;
    
    
    /* Match beginning of line marker '^' */
    if (piece->pattern_type == BEGIN_LINE)
    {
	/*printf("pattern_type = %d, cur_char = %p, *cur_char = %c *line_base = %p.\r\n",
	    piece->pattern_type,cur_char,*cur_char,*line_base);
	printf("Checking for BEGIN_LINE... ");*/
	if (cur_char == *line_base)
	    return TRUE;
	else
	    return FALSE;
    }

    if ( piece->pattern_type == END_LINE )
    {
	if ( *cur_char == '\0' )
	    return TRUE;
	else
	    return FALSE;
    }
    
    /* End of string and there is an array? */
    /* Watch for bugs here - had a problem with APE missing NULL pointer */
    /* Maybe should get last_line argument instead? */
    while ((*line_base != NULL) && (*cur_char == '\0'))
    {
	/* Match end of line marker '$' to a space char in RE */
	switch ( piece->pattern_type )
	{
	case    CHAR_LITERAL:
	    if ( piece->atom.ch != ' ' )
		return FALSE;
	    break;
	case    SET:
	    if ( !IS_MEMBER(piece,' ') )
		return FALSE;
	    break;
	/* String literal won't match anyway - just skip it. */
	/* Putting in the case also caused a seg fault with NULL *line_base */
	/* Didn't figure out why. */
	}
	
	/* Add size (bytes) to line_base to get to next line */
	line_base = NEXT_LINE(line_base,size);
	
	/* Start of next string in array */
	if ( *line_base != NULL )
	    cur_char = *line_base;
	else    /* End of input */
	{
	    *cur_char_ptr = cur_char;
	    *line_base_ptr = line_base;
	    return TRUE;
	}
    }
    
#ifdef DEBUG
    printf("*line_base = %p, cur_char = %p ('\\%o')\r\n",
	    *line_base, cur_char, *cur_char);
    fflush(stdout);
    if ( *cur_char == '\0' )
	getchar();
#endif

    switch (piece->pattern_type)
    {
    case CHAR_LITERAL:
	/* Match char exactly, unless escaped with '\' */
	if (piece->atom.ch == *cur_char)
	{
	    ++cur_char;
	    status = TRUE;
	}
	else
	    status = FALSE;
	break;
    case STRING_LITERAL:
	if (memcmp(cur_char, piece->atom.string, piece->string_len) == 0)
	{
	    cur_char += piece->string_len;
	    status = TRUE;
	}
	else
	    status = FALSE;
	break;
    case ANY:
	++cur_char;
	status = TRUE;
	break;
    case SET:
	if (IS_MEMBER(piece,*cur_char))
	{
	    ++cur_char;
	    status = TRUE;
	}
	else
	    status = FALSE;
	break;
    default:
	status = FALSE;
    }
    *cur_char_ptr = cur_char;
    *line_base_ptr = line_base;
    return status;
}
