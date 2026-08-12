/*
*
* cs_string.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

#include <iostream.h>
#include <stdlib.h>
#include <iomanip.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include "cs_basic.h"
#include "cs_string.h"
#include "cs_list.h"

#include <stdio.h>

#if !defined(_WIN32)
#define _vsnprintf	vsnprintf
#endif

namespace tool
{

  string::data string::null_data;


  /****************************************************************************/
  string::data *
    string::new_data ( int length, int refcount )
  {
    if ( length > 0 )
    {
      int to_allocate = ( length + 0x10 ) & ~0xF;

      //assert ( to_allocate >= length );
      data *dt = (data *) new byte [ sizeof ( data ) + to_allocate ];
      dt->ref_count  = refcount;
      dt->length     = length;
      dt->allocated  = to_allocate;
      dt->chars [ length ] = '\0';
      //memset ( dt->chars, 0, to_allocate + 1 );
      return dt;
    }
    else
      return &null_data;
  }


  void
    string::set_length ( int length, bool preserve_content )
  {
    if ( ( length <= my_data->allocated ) && ( my_data->ref_count <= 1 ) )
    {
      my_data->length = length;
      my_data->chars [ length ] = '\0';
      return;
    }

    data *dt = new_data ( length, 1 );

    if ( preserve_content )
      ::memcpy ( dt->chars, my_data->chars, my_data->length );

    release_data();
    my_data = dt;
  }


  void
    string::release_data ( void )
  {
    if ( ( my_data != &null_data ) && ( --my_data->ref_count == 0 ) )
      delete (byte *) my_data;
  }


  void
    string::set_data ( data *data )
  {
    release_data();
    my_data = data;
    my_data->ref_count++;
  }


  void
    string::make_unique ()
  {
    if ( my_data->ref_count > 1 )
    {
      data *data = new_data ( length(), 1 );
      ::memcpy ( data->chars, chars(), length() );
      my_data->ref_count--;
      my_data = data;
    }
  }


  /****************************************************************************/
  string
    operator+ ( const char *s1, const string &s2 )
  {
    const int s1_length = ::strlen ( s1 );

    if ( s1_length == 0 )
      return s2;
    else
    {
      string newstring;
      newstring.set_length ( s1_length + s2.length() );
      ::memcpy ( newstring.chars(), s1, s1_length );
      ::memcpy ( &( newstring.chars() ) [ s1_length ], s2.chars(), s2.length() );
      return newstring;
    }
  }


  /****************************************************************************/
  string
    string::operator+ ( const string &s ) const
  {
    if ( length() == 0 )
      return s;
    else if (s.length() == 0)
      return *this;
    else
    {
      string newstring;
      newstring.set_length ( length() + s.length() );
      if ( length() )
        ::memcpy ( newstring.chars(), chars(), length() );
      if ( s.length() )
        ::memcpy ( &( newstring.chars() ) [ length() ], s.chars(), s.length() );
      return newstring;
    }
  }


  /****************************************************************************/
  string
    string::operator+ ( const char *s ) const
  {
    const int s_length = ::strlen ( s );

    if ( s_length == 0 )
      return *this;
    else
    {
      string newstring;
      newstring.set_length ( length() + s_length );
      ::memcpy ( newstring.chars(), chars(), length() );
      ::memcpy ( &( newstring.chars() ) [ length() ], s, s_length );
      return newstring;
    }
  }


  /****************************************************************************/
  bool
    string::is_whitespace () const
  {
    if ( my_data == &null_data )
      return false;

    for ( register const char *p = chars(); *p; p++ )
      if ( !isspace ( *p ) )
        return false;

    return true;
  }


  /****************************************************************************/
  string
    string::substr ( int index, int len ) const
  {
    // a negative index specifies an index from the right of the string.
    if ( index < 0 )
      index += length();

    // a length of -1 specifies the rest of the string.
    if ( len == -1 )
      len = length() - index;

    string newstring;
    if ( index < 0 || index >= length() || len < 0 || len > length() - index )
      return newstring; // error in parameters;

    newstring.set_length ( len );
    ::memcpy ( newstring.chars(), &chars() [ index ], len );

    return newstring;
  }


  /****************************************************************************/
  string &
    string::cut ( int index, int len )
  {
    if ( len == 0 )
      return *this;

    // a negative index specifies an index from the right of the string.
    if ( index < 0 )
      index += length();

    // a length of -1 specifies the rest of the string.
    if ( len == -1 )
      len = length() - index;

    make_unique();

    assert ( ( index >= 0 ) && ( index < length() ) &&
             ( len > 0 ) && ( len <= ( length() - index ) ) );

    assert ( index + my_data->length - index - len <= my_data->allocated );

    ::memmove ( my_data->chars + index, my_data->chars + index + len,
                my_data->length - index - len );

    set_length ( my_data->length - len );

    return *this;
  }


  /****************************************************************************/
  string&
    string::replace_substr ( const char *s, int index, int len )
  {
    assert ( s );
    return replace_substr ( s, strlen ( s ), index, len );
  }


  /****************************************************************************/
  string &
    string::replace_substr ( const char *s, int s_len, int index, int len )
  {
    // a negative index specifies an index from the right of the string.
    if ( index < 0 )
      index += length();

    // a length of -1 specifies the rest of the string.
    if ( len == -1 )
      len = length() - index;

    assert ( index >= 0 && index < length() && len >= 0 || len < ( length() - index ) );

    make_unique();

    if ( len == s_len && my_data->ref_count == 1 )
      ::memcpy ( &chars() [ index ], s, len );
    else
    {
      int prev_len = length();
      set_length ( prev_len - len + s_len, true );
      ::memmove ( &chars() [ index + s_len ], &chars() [ index + len ], prev_len - len - index );
      if ( s_len > 0 )
      {
        ::memcpy ( &chars() [ index ], s, s_len );
      }
    }

    return *this;
  }


  /****************************************************************************/
  string &
    string::insert ( const char *s, int s_length, int index )
  {
    // a negative index specifies an index from the right of the string.
    if ( index < 0 )
      index += length();

    assert ( index >= 0 && index < length() );

    if ( s_length > 0 )
    {
      make_unique();
      int prev_len = length();
      set_length ( prev_len + s_length, true );
      ::memmove ( &chars() [ index + s_length ], &chars() [ index ],
                  prev_len - index );
      ::memcpy ( &chars() [ index ], s, s_length );
    }

    return *this;
  }


  /****************************************************************************/
  string&
    string::trim()
  {
    int start = 0;
    int end   = length() - 1;
    const unsigned char *p;

    for ( p = (unsigned char *) chars(); *p; p++ )
      if ( isspace ( *p ) )
        start++;
      else
        break;

    for ( p = (unsigned char *) chars() + length() - 1;
          p >= ( (unsigned char *) chars() + start ); p-- )
      if ( isspace ( *p ) )
        end--;
      else
        break;

    make_unique();

    if ( start > end )
    {
      set_length ( 0 );
      return *this;
    }

    int newlen = end - start + 1;
    if ( start )
      ::memmove ( chars(), chars() + start, newlen );

    set_length ( newlen, true );

    return *this;
  }


  /****************************************************************************/
  string &
    string::printf ( const char *fmt, ... )
  {
    char buffer [ 2049 ];
    va_list args;
    va_start ( args, fmt );
    int len = _vsnprintf ( buffer, 2048, fmt, args );
    va_end ( args );
    buffer [ 2048 ] = 0;

    if ( len > 0 )
    {
      set_length ( len );
      ::memcpy ( chars(), buffer, len );
    }
    else
      clear();
    return *this;
  }


  string string::format ( const char *fmt, ... )
  {
    char buffer [ 2049 ];
    va_list args;
    va_start ( args, fmt );
    int len = _vsnprintf( buffer, 2048, fmt, args );
    va_end ( args );
    buffer [ 2048 ] = 0;
    return buffer;
  }


  /****************************************************************************/
  string &
    string::to_upper ()
  {
    make_unique();

#ifdef _WIN32
    _strupr ( chars() );
#else
    for ( register char *p = chars(); *p; p++ )
    *p = toupper ( *p );
#endif

    return *this;
  }


  /****************************************************************************/
  string &
    string::to_lower ()
  {
    make_unique();

#ifdef _WIN32
    _strlwr ( chars() );
#else
    for ( register char *p = chars(); *p; p++ )
    *p = tolower ( *p );
#endif

    return *this;
  }


  /****************************************************************************/
  int
    string::index_of ( const char *s, int start_index ) const
  {
    // a negative index specifies an index from the right of the string.
    if ( start_index < 0 )
      start_index += length();

    if ( start_index < 0 || start_index >= length() )
      return -1;
    //invalid_index_error("index_of()");

    const char *index;
    if ( !( index = strstr ( &chars() [ start_index ], s ) ) )
      return -1;
    else
      return index - chars();
  }


  /****************************************************************************/
  int
    string::index_of ( char c, int start_index ) const
  {
    // a negative index specifies an index from the right of the string.
    if ( start_index < 0 )
      start_index += length();

    if ( start_index < 0 || start_index >= length() )
      return -1;

    const char *index;

    if ( c == '\0' )
      return -1;
    else if ( !( index = (char *) ::memchr ( &chars() [ start_index ], c,
                                             length() - start_index ) ) )
      return -1;
    else
      return index - chars();
  }


  /****************************************************************************/
  int string::last_index_of ( char c, int start_index ) const
  {
    // a negative index specifies an index from the right of the string.
    if ( start_index < 0 )
      start_index = length() - 1;

    if ( start_index < 0 || start_index >= length() )
      return -1;

    if ( c == '\0' )
      return -1;

    const char *p = chars();
    for ( int i = start_index; i >= 0; i-- )
      if( p [ i ] == c )
        return i;

    return -1;
  }


#ifndef NO_ARRAY
  array<string>
    string::tokens ( const char *separators ) const
  {
    array<string> list;
    int token_length, index = 0;
    do
    {
      index += ::strspn ( &chars() [ index ], separators );
      token_length = ::strcspn ( &chars() [ index ], separators );
      if ( token_length > 0 )
        list.push ( substr ( index, token_length ) );
      index += token_length;
    }
    while ( token_length > 0 );

    return list;
  }
#endif


  /****************************************************************************/
#ifndef NO_ARRAY
  array<string>
    string::tokens ( char separator ) const
  {
    char separators [ 2 ];
    separators [ 0 ] = separator;
    separators [ 1 ] = '\0';
    return tokens ( separators );
  }
#endif


  /****************************************************************************/
  bool
    string::read_until ( istream &stream, const char *separators )
  {
    const int num_of_separators = ::strlen ( separators );
    char buffer [ 256 ];
    bool found_end;
    char c;

    set_length ( 0 );
    if ( stream.eof() )
      return false;

    do
    {
      unsigned int i = 0;
      do
      {
        stream.get ( c );
        found_end = !stream || ::memchr ( separators, c, num_of_separators );
        if ( !found_end && i < sizeof ( buffer ) - 1 )
          buffer [ i++ ] = c;
      }
      while ( !found_end && i < sizeof ( buffer ) - 1 );

      buffer [ i ] = '\0';
      *this += buffer;
    }
    while ( !found_end );

    if ( stream )
      stream.putback ( c );

    return true;
  }


  /****************************************************************************/
  bool
    string::read_token ( istream &stream, const char *separators )
  {
    return read_until ( stream, separators );
  }


  /****************************************************************************/
  int
    string::replace ( const char *from, const char *to )
  {
    int to_length = strlen ( to );
    int from_length = strlen ( from );
    int count = 0, idx = 0;

    while ( true )
    {
      idx = index_of ( from, idx ); // + to_length
      if ( idx < 0 )
        break;
      if ( to_length )
        replace_substr ( to, idx, from_length );
      else
        cut ( idx, from_length );
      ++count;
      idx += to_length;
    }
    return count;
  }


  /****************************************************************************/
  //
  // idea was taken from Konstantin Knizhnik's FastDB
  // see http://www.garret.ru/
  // extended by [] operations
  //

  const char AnySubstring = '*';
  const char AnyOneChar = '?';

  int
    string::match ( const char *pattern ) const
  {
    struct charset
    {
      bool codes [ 0x100 ];

      void set ( int from, int to, bool v )
      {
        for ( int i = from;
                  i <= to;
                  i++)
          codes[i]=v;
      }

      void parse ( const char **pp )
      {
        const unsigned char *p = (const unsigned char *) *pp;
        bool inv = *p == '^';
        if ( inv )
        {
          ++p;
        }
        set ( 0, 0xff, inv );
        if ( *p == '-' )
          codes [ '-' ] = !inv;
        while ( *p )
        {
          if ( p [ 0 ] == ']' )
          {
            p++;
            break;
          }
          if ( p [ 1 ] == '-' && p [ 2 ] != 0 )
          {
            set ( p [ 0 ], p [ 2 ], !inv );
            p += 3;
          }
          else
            codes [ *p++ ] = !inv;
        }
        *pp = (const char *) p;
      }

      bool valid ( unsigned char c )
      {
        return codes [ c ];
      }
    };

    const char *str = chars();
    const char *wildcard = 0;
    const char *strpos = 0;
    const char *matchpos = 0;
    charset     cset;

    while ( true )
    {
      if ( *pattern == AnySubstring )
      {
        wildcard = ++pattern;
        strpos = str;
        if ( !matchpos )
          matchpos = str;
      }
      else if ( *str == '\0' )
      {
        return ( *pattern == '\0' ) ? ( matchpos - chars() ) : -1;

      }
      else if ( *pattern == '[' )
      {
        cset.parse ( &pattern );
        if ( !cset.valid ( (unsigned char) *str ) )
          return -1;
        if ( !matchpos )
          matchpos = str;
        str += 1;
      }
      else if ( *str == *pattern || *pattern == AnyOneChar )
      {
        if ( !matchpos )
          matchpos = str;
        str += 1;
        pattern += 1;
      }
      else if ( wildcard )
      {
        str = ++strpos;
        pattern = wildcard;
      }
      else
      {
        break;
      }
    }
    return -1;
  }

};
