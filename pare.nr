PARE()                                                                  PARE()



NNAAMMEE
       libpare - Pointer Array Regular Expression match


SSYYNNOOPPSSIISS
       int      pare_comp(compile_re,pattern,max_pieces) cre_t   *compiled_re;
       char    *pattern; size_t  max_pieces;

       int      pare_match(compiled_re,address,array_address)  cre_t     *com-
       piled_re; char    *address,**array_address;

       void    pare_free(compiled_re) cre_t   *compiled_re;


DDEESSCCRRIIPPTTIIOONN
       ppaarree__mmaattcchh(())  attempts  to  match  the regular expression given in ccoomm--
       ppiilleedd__rree beginning at the character address given by aaddddrreessss..

       If aarrrraayy__aaddddrreessss is non NNUULLLL,, then aaddddrreessss should  specify  an  address
       within  an array of pointers to strings such as argv[] or envp[] (not a
       2-D array), and address and array_address should both refer to the same
       string.   When  address  reached  the ' ' terminator, pare_match() will
       increment array_address and continue with the next string in the array.

       The  pointer  array  should  be  terminated  by a NULL pointer, as with
       argv[] and envp[].

       For example:

           char    *list[] = {"first","second","third",NULL};

           ...
           pare_compile(&cre,pattern,MAX_PIECES);
           if ( pare_match(&cre,list[0],list) > 0 )
           {
            ...
           }


RREETTUURRNN VVAALLUUEESS
                                                                        PARE()
