/*
*
* cs_regexp.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __cs_regexp_h
#define __cs_regexp_h

#include "cs_basic.h"
#include "cs_string.h"

namespace tool
{
  /*
  *
  *  Definitions etc. for regexp(3) routines.
  *
  * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
  * not the System V one.
  *
  * Initially written by Henry Spencer.  Not derived from licensed software.
  * modified by myself to fulfil multithreading conditions
  */

#define NSUBEXP  10

  class regexp
  {
  public:
    class error
    {
    public:
      const char *description;
      error ( const char *desc ) : description ( desc )
      {
      }
    };

  protected:
    char *startp [ NSUBEXP ];
    char *endp [ NSUBEXP ];

    char  regstart;		      /* Internal use only. */
    char  reganch;		      /* Internal use only. */
    char *regmust;		      /* Internal use only. */
    int   regmlen;		      /* Internal use only. */
    char *program;	        /* Unwarranted chumminess with compiler. */

    // work variables for compile().
    struct  compiler_vars
    {
      char   *regparse;		/* Input-scan pointer. */
      int     regnpar;		/* () count. */
      char   *regcode;		/* Code-emit pointer; &regdummy = don't. */
      long    regsize;		/* Code size. */
    };

    // compile()'s friends.
    char*   reg ( compiler_vars &cvars, int paren, int* flagp );
    char*   regbranch ( compiler_vars &cvars, int* flagp );
    char*   regpiece  ( compiler_vars &cvars, int* flagp );
    char*   regatom   ( compiler_vars &cvars, int* flagp );
    char*   regnode   ( compiler_vars &cvars, char op );
    char*   regnext   ( char* p );
    void    regc ( compiler_vars &cvars, char c );
    void    reginsert ( compiler_vars &cvars, char op, char* opnd );
    void    regtail   ( compiler_vars &cvars, char* p, char* val );
    void    regoptail ( compiler_vars &cvars, char* p, char* val );

    // exec vars
    struct exec_vars
    {
      char *reginput;		/* String-input pointer. */
      char *regbol;     /* Beginning of input, for ^ check. */
      char **regstartp;	/* Pointer to startp array. */
      char **regendp;		/* Ditto for endp. */
    };

    // regexec's friends
    int regtry    ( exec_vars& evars, char* string );
    int regmatch  ( exec_vars& evars, char* prog );
    int regrepeat ( exec_vars& evars, char* p );

    static char regdummy;

    void clean ();

  public:
     regexp ();
    ~regexp ();
    bool    compile ( const char* re  );
    bool    exec    ( const char* str );
    bool    split   ( const char *str, array<string>& all_matches );

    int     count (); // number of matches
    string  operator[] ( int index ); // n-th match

  };

};
#endif
