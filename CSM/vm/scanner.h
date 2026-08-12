/*
*
* scanner.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __scanner_h
#define __scanner_h

#include "tool.h"
#include "c-smile.h"
#include "streams.h"

namespace c_smile
{

#define TKNSIZE		2048	  // maximum token size
#define LSIZE	    2048  // line buffer size

  class scanner
  {
    friend class compiler;
    friend class parse_error;
  public:
    // publics
    VALUE   t_value;		          /* numeric value */
    string  t_token;              /* token string */

  private:
    // locals
    io_stream  *input;
    int         savetkn;	          /* look ahead token */
    int         savech;	            /* look ahead character */
    int         lastch;	            /* last input character */
    char        line [ LSIZE ];     /* last input line */
    char *      lptr;	              /* line pointer */
    int         lnum;	              /* line number */
    compiler *  comp;

    int rtoken       ( void );
    int getstring    ( void );
    int getcharacter ( void );
    int literalch    ( void );
    int getid        ( int ch );
    int getnumber    ( int ch );
    int skipspaces   ( void );
    int isidchar     ( int ch );
    int getch        ( void );
    int getxch       ();

  public:
    void  init ( compiler *c, io_stream  *input );
    int   token ();
    void  stoken ( int tkn );
    char *tkn_name ( int tkn );
  };

  class parse_error
  {
    string          description;
    string          file_name;
    string          line_buf;
    size_t          line_no;
    size_t          position;
  public:
    parse_error ( compiler *c, const char *fmt, ... );
    ~parse_error  ()
    {
    }

    string    report ();
    string    full_report ();
  };

};

#endif
