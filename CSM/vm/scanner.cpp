/*
*
* scanner.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

#include <stdarg.h>
#include "c-smile.h"
#include "tool.h"
#include "scanner.h"
#include "compiler.h"
#include "streams.h"

#ifndef _WIN32
#define _vsnprintf vsnprintf
#endif


namespace c_smile
{

#ifdef COMPILER

  /* keyword table */
  static struct
  {
    char *kt_keyword; int kt_token;
  }
  ktab [] =
  {
    { "class",        T_CLASS       },
    { "static",       T_STATIC      },
    { "if",           T_IF          },
    { "else",         T_ELSE        },
    { "while",        T_WHILE       },
    { "return",       T_RETURN      },
    { "for",          T_FOR         },
    { "break",        T_BREAK       },
    { "continue",     T_CONTINUE    },
    { "do",           T_DO          },
    { "new",          T_NEW         },
    { "null",         T_NULL        },
    { "var",          T_VAR         },
    { "function",     T_FUNCTION    },
    { "package",      T_PACKAGE     },
    { "try",          T_TRY         },
    { "catch",        T_CATCH       },
    { "throw",        T_THROW       },
    { "property",     T_PROPERTY    },
    { "import",       T_IMPORT      },
    { "instanceof",   T_INSTANCEOF  },
    { "undefined",    T_UNDEFINED   },
    { "switch",       T_SWITCH      },
    { "case",         T_CASE        },
    { "default",      T_DEFAULT     },
    { "synchronized", T_SYNCHRO     },
    { "const",        T_CONST       },
    { NULL,           0             }
  };

  /* token name table */
  static char *t_names  [] =
  {
    "<string>",
    "<identifier>",
    "<number>",
    "class",
    "static",
    "if",
    "else",
    "while",
    "return",
    "for",
    "break",
    "continue",
    "do",
    "new",
    "null",
    "<=",
    "==",
    "!=",
    ">=",
    "<<",
    ">>",
    "&&",
    "||",
    "++",
    "--",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "&=",
    "|=",
    "^=",
    "<<=",
    ">>=",
    "::",
    ".",
    "var",
    "function",
    "package",
    "try",
    "catch",
    "throw",
    "property",
    "import",
    "instanceof",
    "undefined",
    "$",
    "$$",
    "switch",
    "case",
    "default",
    "@",
    "synchronized",
    "const",
  };


  // init_scanner - initialize the scanner
  void
    scanner::init ( compiler *c, io_stream  *input )
  {
    comp = c;
    // remember the input
    this->input = input;

    // setup the line buffer
    lptr = line; *lptr = '\0';
    lnum = 0;

    // no lookahead yet
    savetkn = T_NOTOKEN;
    savech = '\0';

    // no last character
    lastch = '\0';
  }

  // token - get the next token
  int
    scanner::token ()
  {
    int tkn;

    if ( ( tkn = savetkn ) != T_NOTOKEN )
      savetkn = T_NOTOKEN;
    else
      tkn = rtoken ();
    return ( tkn );
  }

  // stoken - save a token
  void
    scanner::stoken ( int tkn )
  {
    savetkn = tkn;
  }

  // tkn_name - get the name of a token
  char *
    scanner::tkn_name ( int tkn )
  {
    static char tname [ 2 ];

    if ( tkn == T_EOF )
      return ( "<eof>" );
    else if ( tkn >= _TMIN && tkn <= _TMAX )
      return ( t_names [ tkn - _TMIN ] );
    tname [ 0 ] = tkn;
    tname [ 1 ] = '\0';
    return ( tname );
  }

  // rtoken - read the next token
  int
    scanner::rtoken ()
  {
    int ch, ch2;

    // check the next character
    for ( ; ; )
      switch ( ch = skipspaces  () )
      {
      case EOF:
        return ( T_EOF );
      case '"':
        return ( getstring () );
      case '\'':
        return ( getcharacter () );
      case '$':
        if ( (ch = getch () ) == '$' )
          return T_ARGUMENTS;
        savech = ch;
        return T_ARGUMENT;
      case '@':
        return T_MAKEREF;

      case '<':
        switch ( ch = getch () )
        {
        case '=':
          return ( T_LE );
        case '<':
          if ( (ch = getch () ) == '=' )
            return ( T_SHLEQ );
          savech = ch;
          return ( T_SHL );
        default:
          savech = ch;
          return ('<');
        }
      case '=':
        if ( (ch = getch () ) == '=' )
          return ( T_EQ );
        savech = ch;
        return ( '=' );
      case '!':
        if ( ( ch = getch () ) == '=' )
          return ( T_NE );
        savech = ch;
        return ('!');
      case '>':
        switch ( ch = getch () )
        {
        case '=':
          return ( T_GE );
        case '>':
          if ( ( ch = getch () ) == '=' )
            return ( T_SHREQ );
          savech = ch;
          return ( T_SHR );
        default:
          savech = ch;
          return ( '>' );
        }
      case '&':
        switch ( ch = getch () )
        {
        case '&':
          return ( T_AND );
        case '=':
          return ( T_ANDEQ );
        default:
          savech = ch;
          return ('&');
        }
      case '|':
        switch ( ch = getch () )
        {
        case '|':
          return ( T_OR );
        case '=':
          return ( T_OREQ );
        default:
          savech = ch;
          return ('|');
        }
      case '^':
        if ( ( ch = getch () ) == '=' )
          return ( T_XOREQ );
        savech = ch;
        return ('^');
      case '+':
        switch ( ch = getch () )
        {
        case '+':
          return ( T_INC );
        case '=':
          return ( T_ADDEQ );
        default:
          savech = ch;
          return ( '+' );
        }
      case '-':
        switch ( ch = getch () )
        {
        case '-':
          return ( T_DEC );
        case '=':
          return ( T_SUBEQ );
        default:
          savech = ch;
          if ( isdigit ( ch ) )
            return ( getnumber ( '-' ) );
          return ('-');
        }
      case '.':
        return ( T_MEMREF );
      case '*':
        if ( (ch = getch () ) == '=')
          return ( T_MULEQ );
        savech = ch;
        return ('*');
      case '/':
        switch ( ch = getch () )
        {
        case '=':
          return ( T_DIVEQ );
        case '/':
          while ( ( ch = getch () ) != EOF )
            if ( ch == '\n' )
              break;
          break;
        case '*':
          ch = ch2 = EOF;
          for ( ; ( ch2 = getch () ) != EOF; ch = ch2 )
            if ( ch == '*' && ch2 == '/')
              break;
          break;
        default:
          savech = ch;
          return ('/');
        }
        break;
      case ':':
        if ( ( ch = getch () ) == ':' )
          return ( T_CC );
        savech = ch;
        return ( ':' );
      default:
        if ( isdigit ( ch ) )
          return ( getnumber ( ch ) );
        else if ( isidchar ( ch ) )
          return ( getid ( ch ) );
        else
        {
          t_token.clear ();
          t_token += ch;
          return ( ch );
        }
      }
  }

  int
    scanner::getxch ()
  {
    int value, ch;

    if ( ( ch = getch () ) == EOF || !isxdigit ( ch ) )
    {
      savech = ch;
      return 0;
    }

    value = isdigit ( ch ) ? ch - '0' : tolower ( ch ) - 'a' + 10;
    if ( ( ch = getch  () ) == EOF || !isxdigit ( ch ) )
    {
      savech = ch;
      return value;
    }
    return ( value << 4 ) | ( isdigit ( ch ) ? ch - '0'
                                             : tolower ( ch ) - 'a' + 10 );
  }

  // getstring - get a string
  int
    scanner::getstring ()
  {
    int ch;

    /* get the string */
    t_token.clear ();
    while ( ( ch = getch () ) != EOF && ch != '"' )
    {
      if ( ch == '\\')
        switch ( ch = getch () )
      {
      case 'a':  ch = '\a'; break;
      case 'b':  ch = '\b'; break;
      case 'f':  ch = '\f'; break;
      case 'n':  ch = '\n'; break;
      case 'r':  ch = '\r'; break;
      case 't':  ch = '\t'; break;
      case 'v':  ch = '\v'; break;
      case '"':  ch = '"';  break;
      case 'x':  ch = getxch (); break;
      case '\\': ch = '\\'; break;
      case EOF:  goto  goteof;
      }
      t_token += ch;
    }
  goteof:
    if ( ch == EOF )
      savech = EOF;

    return ( T_STRING );
  }

  // getcharacter - get a character constant
  int
    scanner::getcharacter ()
  {
    t_value = literalch ();
    t_token.clear ();
    t_token += (char) int ( t_value );

    if ( getch ()  != '\'' )
      throw parse_error ( comp, "Expecting a closing single quote" );

    return ( T_NUMBER );
  }

  // literalch - get a character from a literal string
  int
    scanner::literalch ()
  {
    int ch;
    if ( ( ch = getch () ) == '\\' )
      switch ( ch = getch () )
      {
      case 'a':  ch = '\a'; break;
      case 'b':  ch = '\b'; break;
      case 'f':  ch = '\f'; break;
      case 'n':  ch = '\n'; break;
      case 'r':  ch = '\r'; break;
      case 't':  ch = '\t'; break;
      case 'v':  ch = '\v'; break;
      case '"':  ch = '"';  break;
      case '\\': ch = '\\'; break;
      case 'x':  ch = getxch (); break;
      case EOF:  ch = '\\'; savech = EOF; break;
      }

    return ( ch );
  }

  // getid - get an identifier
  int
    scanner::getid ( int ch )
  {
    int i;

    // get the identifier
    t_token.clear (); t_token += ch;

    while ( (ch = getch () ) != EOF && isidchar ( ch ) )
      t_token += ch;
    savech = ch;

    // check to see if it is a keyword
    for ( i = 0; ktab [ i ].kt_keyword != NULL; ++i )
      if ( strcmp ( ktab [ i ].kt_keyword, t_token ) == 0 )
        return ( ktab [ i ].kt_token );

    return ( T_IDENTIFIER );
  }

  bool
    is_num_char ( int ch, bool& dp, bool& dc )
  {
    if ( isdigit ( ch ) )
      return true;

    if ( !dp && ch == '.' )
    {
      dp = true;
      return true;
    }

    if ( !dc && ( (ch == 'd')|| ( ch == 'D')|| ( ch == 'e')|| ( ch == 'E') ) )
    {
      dc = true;
      return true;
    }

    return false;
  }

  // getnumber - get a number
  int
    scanner::getnumber ( int ch )
  {
    char *pp;
    bool dp = false, dc = false;

    // get the number
    t_token.clear ();
    t_token += ( char ) ch;
    ch = getch ();
    if ( t_token [ 0 ] == '0' && ( ch == 'x' || ch == 'X') )
    {
      t_token.clear ();
      while ( (ch = getch () ) != EOF && isxdigit ( ch ) )
        t_token += ch;
      t_value = (int) strtoul ( t_token, &pp, 16 );
      if ( *pp != '\0')
        goto error;
    }
    else if ( is_num_char ( ch, dp, dc ) )
    {
      t_token += ch;
      while ( ( ch = getch () ) != EOF && is_num_char ( ch, dp, dc ) )
        t_token += ch;

      if ( dp || dc )
        t_value = strtod ( t_token, &pp );
      else
        t_value = strtol ( t_token, &pp, 10 );

      if ( *pp != '\0')
        goto error;
    }
    else
    {
      t_value = long ( t_token [ 0 ] - '0');
    }
    savech = ch;
    return ( T_NUMBER );
  error:
    throw parse_error ( comp, "Bad number constant '%s'",
                        (const char *) t_token );
    return ( T_NUMBER );
  }

  // skipspaces - skip leading spaces
  int
    scanner::skipspaces ()
  {
    int ch;
    while ( ( ch = getch () ) != '\0' && isspace ( ch ) );

    return ( ch );
  }

  // isidchar - is this an identifier character
  int
    scanner::isidchar ( int ch )
  {
    return ( isupper ( ch ) ||
             islower ( ch ) ||
             isdigit ( ch ) ||
             ch == '_'
           );
  }

  // getch - get the next character
  int
    scanner::getch ()
  {
    int ch;

    // check for a lookahead character
    if ( ( ch = savech ) != '\0' )
      savech = '\0';
    // check for a buffered character
    else
    {
      while ( ( ch = *lptr++ ) == '\0' )
      {
        /* check for being at the end of file */
        if ( lastch == EOF )
          return ( EOF );

        /* read the next line */
        lptr = line;

        char c;
        while ( true )
        {
          if ( !input->get ( c ) )
          {
            lastch = EOF;
            break;
          }
          if ( ( lastch = c ) == '\n' )
            break;
          *lptr++ = lastch;
        }

        *lptr++ = '\n'; *lptr = '\0';
        lptr = line;
        ++lnum;
      }
    }

    // return the current character
    return ( ch );
  }

  // parse_error - error class
  parse_error::parse_error ( compiler *c, const char *fmt, ... )
  {
    char buffer [ 2049 ];
    va_list args;
    va_start ( args, fmt );
    int len = _vsnprintf ( buffer, 2048, fmt, args );
    va_end ( args );
    buffer [ 2048 ] = 0;

    description = buffer;
    line_no = c->scan.lnum;
    position = c->scan.lptr - c->scan.line;
    file_name = c->file_name;
    line_buf = c->scan.line;
  }

  string parse_error::report ()
  {
    string out; out.printf ( "%s ( %d ) :parse error:%s\n",
                             (const char *) file_name,
                             line_no,
                             (const char *) description
                           );
    return out;
  }

  string
    parse_error::full_report ()
  {
    string out = report ();
    out += line_buf;
    for ( int i = 0; i < ( (int) position - 1 ); ++i )
      out += ( line_buf [ i ] == '\t' ) ? '\t' : ' ';
    out += '^'; out += '\n';
    return out;
  }

#endif //COMPILER

};
