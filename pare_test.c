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
#include <ctype.h>
#include <stdlib.h>
#include "pare.h"

typedef struct
{
    char    *buff;
    int     junk;
}   junk_t;


int     main(argc,argv)
int     argc;
char    *argv[];

{
    char    *string[] = {
		"/*",
		"*/",
		NULL};
    char    pattern[STR_MAX+1]="/\\*.*\\*/";
    size_t  count;
    cre_t   cre;
    reloc_t end;
    int     status;
    
    if ( (status=pare_compile(&cre,pattern)) != 0 )
    {
	printf("pare_compile returned %d.\n",status);
	return 1;
    }
    if ( (count=pare_match(&cre,string[0],string,sizeof(char *),&end)) )
    {
	printf("%d atoms matched.\n",count);
	printf("lines = %d, end_col = %d\n",end.lines,end.end_col);
    }
    else
	printf("No match.\n");
    fflush(stdout);
    /*pare_free(&cre);*/
    return 0;
}

