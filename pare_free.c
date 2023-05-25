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

#include <stdlib.h>
#include "pare.h"

/****************************************************************************
 *
 * Title: pare_free()
 * Author: Jason Bacon
 * Purpose: Free memory allocated by pare_compile()
 * Similar functions: regfree()
 * Created: 12-29-97
 * Modification history:
 *
 * Arguments: cre: a pointer to a compiled regular expression
 * Return values: none
 *
 * Description:
 *
 ****************************************************************************/

void    pare_free(cre_t *cre)

{
    branch_t    *cur_branch;
    piece_t     *cur_piece;
    int         c, b;

    for (b=0; b<cre->branch_count; ++b)
    {
	cur_branch = cre->branches + b;
	
	for (c=0, cur_piece=cur_branch->pieces; c<cur_branch->piece_count;
	    ++c, ++cur_piece)
	{
	    switch(cur_piece->pattern_type)
	    {
		case    SET:
		    free(cur_piece->atom.ismember);
		    break;
		case    STRING_LITERAL:
		    free(cur_piece->atom.string);
		    break;
		default:
		    break;
	    }
	}
    }
    free(cre->branches);
}

