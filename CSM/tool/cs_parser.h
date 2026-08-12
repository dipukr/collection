/*
*
* cs_parser.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __cs_parser_h
#define __cs_parser_h

//|
//|
//| (semi)universal lexical scanner
//|
//|

#include "cs_string.h"

namespace tool
{
  class parser
  {
  public:
    enum cvt_flag
    {
      cvt_no = 0,
      cvt_to_upper = 1,
      cvt_to_lower = 2
    };

  protected:
    int       _p_state;       // current  state
    cvt_flag  _p_flag;        // option   flag
    char      _p_curquote;    // current  quote char

    string    _token;
    string    _line;
    string    _white;
    string    _brkchar;
    string    _quote;
    char      _eschar;
    char      _brkused;
    int	      _next;
    char      _quoted;

  public:

    parser ( const char * parsingline, cvt_flag flag = cvt_no );

    void
      white_chars ( const char * ps )
    {
      _white = ps;
    };

    void
      break_chars ( const char * ps )
    {
      _brkchar = ps;
    };

    void
      quote_chars ( const char * ps )
    {
      _quote = ps;
    };

    bool		parse ( void );
    string  token ( void );

    char
      break_char ( void )
    {
      return _brkused;
    };

    char
      quoted_char ( void )
    {
      return _quoted;
    };

  protected:

    int sindex ( char ch, const char *str );

  };

  /*
usage:
  parser p("var a = 1+1; b = \"string\"");
  while(p.parse())
  {
    if(p.quoted_char())
      printf("string literal = %s\n",(const char*)p.token());
    else
      printf("token = %s\n",(const char*)p.token());
  }
  */

};

#endif //__cs_parser_h
