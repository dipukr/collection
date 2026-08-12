/*
*
* cs_parser.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#include "cs_parser.h"

namespace tool
{

  enum parser_states
  {
    IN_WHITE,
    IN_TOKEN,
    IN_QUOTE,
    IN_OZONE
  };


  parser::parser ( const char * parsingline, cvt_flag flag ) :
          _line ( parsingline ),
          _p_flag(flag),
          _p_state(IN_WHITE),
          _p_curquote(0),
          _white(" \t\r"),        // blank and tab
          _brkchar(",;=\n"),      // comma and carriage return
          _quote("'\""),          // single and double quote
          _eschar('\\'),          // "bakslash" is escape
          _next(0)
  {
  }

  // routine to find character in string ... used only by "parser"

  int
    parser::sindex ( char ch, const char * str )
  {
    const char * cp;
    for ( cp = str; *cp; ++cp )
      if ( ch == *cp )
        return (int) ( cp - str );  // return postion of character
    return -1;                 // eol ... no match found
  }


  string
    parser::token ( void )
  {
    if ( _p_state == IN_QUOTE )
      return _token;

    switch ( _p_flag )
    {
    case cvt_to_upper:  // convert to upper
      return _token.to_upper();
    case cvt_to_lower:  // convert to lower
      return _token.to_lower();
    default:            // use as is
      return _token;
    }
  }


  // here it is!
  bool
    parser::parse ( void )
  {
    int qp;
    char c, nc;

    _brkused = 0; // initialize to null
    _quoted  = 0; // assume not quoted

    _token.clear();

    if ( _next >= _line.length() )
      return false;

    _p_state    = IN_WHITE;  // initialize state
    _p_curquote = 0;         // initialize previous quote char

    for ( ; _next < _line.length(); ++_next )       // main loop
    {
      c = _line [ _next ];
      if ( ( qp = sindex ( c, _brkchar ) ) >= 0 )   // break
      {
        switch ( _p_state )
        {
        case IN_WHITE:          // these are the same here ...
        case IN_TOKEN:          // ... just get out
        case IN_OZONE:					// ditto
          ++_next;
          _brkused=_brkchar [ qp ];
          goto byebye;

        case IN_QUOTE:                  // just keep going
          _token += c;
          break;
        }
      }
      else if ( ( qp = sindex ( c, _quote ) ) >= 0 )  // quote
      {
        switch ( _p_state )
        {
        case IN_WHITE:						      // these are identical,
          _p_state    = IN_QUOTE;       // change states
          _p_curquote = _quote [ qp ];  // save quote char
          _quoted     = _p_curquote;    // set to true as long as
          break;                        // something is in quotes

        case IN_QUOTE:
          if ( _quote [ qp ] == _p_curquote )  // same as the beginning quote?
          {
            _p_state    = IN_OZONE;
            _p_curquote = 0;
          }
          else
            _token += c;			             // treat as regular char
          break;

        case IN_TOKEN:
        case IN_OZONE:
          _brkused = c;                    // uses quote as break char
          goto byebye;
        }
      }
      else if ( ( qp = sindex ( c, _white ) ) >= 0 )  // white
      {
        switch ( _p_state )
        {
        case IN_WHITE:
        case IN_OZONE:
          break;                           // keep going

        case IN_TOKEN:
          _p_state = IN_OZONE;
          break;

        case IN_QUOTE:
          _token += c;                     // it's valid here
          break;
        }
      }
      else if ( c == _eschar )	           // escape
      {
        if ( _next == _line.length() - 1 ) // end of line
        {
          _brkused = 0;
          _token += c;
          ++_next;
          goto byebye;
        }
        nc = _line [ _next + 1 ];
        switch ( _p_state )
        {
        case IN_WHITE:
          --_next;
          _p_state = IN_TOKEN;
          break;
        case IN_TOKEN:
        case IN_QUOTE:
          ++_next;
          _token += nc;
          break;
        case IN_OZONE:
          goto byebye;
        }
      }
      else  // anything else is just a real character
      {
        switch ( _p_state )
        {
        case IN_WHITE:
          _p_state = IN_TOKEN;   // switch states

        case IN_TOKEN:           // these too are
        case IN_QUOTE:           // identical here
          _token += c;
          break;

        case IN_OZONE:
          goto byebye;
        }
      }
    }
    // main loop

  byebye:
    return true;
  }

};
