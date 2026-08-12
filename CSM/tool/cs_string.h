/*
*
* cs_string.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

/*
*
* This code derived from kplib
* ( Keith Pomakis, http://www.pomakis.com/~pomakis/kplib/ )
*
*/

#ifndef __cs_STRING_H
#define __cs_STRING_H

#include <iostream.h>
#include <string.h>
#include "cs_basic.h"
#include "cs_array.h"

#if !defined(_WIN32)
#define stricmp	strcasecmp
#endif

namespace tool
{
  // this class uses reference counting and copy-on-write semantics to insure
  // that it as efficient as possible.

  // all indexes are zero-based.  for all functions that accept an index, a
  // negative index specifies an index from the right of the string.  also,
  // for all functions that accept a length, a length of -1 specifies the rest
  // of the string.

  class string
  {
    inline friend istream &operator>> ( istream &stream, string &s );
    inline friend ostream &operator<< ( ostream &stream, const string &s );
    friend string operator+ ( const char *s1, const string &s2 );
    inline friend bool operator== ( const char *s1, const string &s2 );
    inline friend bool operator< ( const char *s1, const string &s2 );
    inline friend bool operator<= ( const char *s1, const string &s2 );
    inline friend bool operator> ( const char *s1, const string &s2 );
    inline friend bool operator>= ( const char *s1, const string &s2 );
    inline friend bool operator!= ( const char *s1, const string &s2 );
    inline friend void swap ( string &s1, string &s2 );
  public:
    string ();
    string ( const string &s );
    string ( const char *s );
    string ( const char *s, int count );
    string ( const uchar *s );
    string ( char c, int n = 1 );
    ~string ();
    operator const char * () const;
    char *buffer ();
    char &operator[] ( int index );
    char operator[] ( int index ) const;
    string &operator= ( const string &s );
    string &operator= ( const char *s );
    string &operator= ( const uchar *s );
    string operator+ ( const string &s ) const;
    string operator+ ( const char *s ) const;
    bool operator== ( const string &s ) const;
    bool operator== ( const char *s ) const;
    bool operator< ( const string &s ) const;
    bool operator< ( const char *s ) const;
    bool operator<= ( const string &s ) const;
    bool operator<= ( const char *s ) const;
    bool operator> ( const string &s ) const;
    bool operator> ( const char *s ) const;
    bool operator>= ( const string &s ) const;
    bool operator>= ( const char *s ) const;
    bool operator!= ( const string &s ) const;
    bool operator!= ( const char *s ) const;
    string &operator+= ( const string &s );
    string &operator+= ( const char *s );

    int length () const;
    bool is_empty () const;
    bool is_whitespace () const;
    string &to_upper ();
    string &to_lower ();
    string &clear ();
    string substr ( int index = 0, int len = -1 ) const;
    string copy () const;
    string &cut ( int index = 0, int len = -1 );
    string &replace_substr ( const string &s, int index = 0, int len=-1 );
    string &replace_substr ( const char *s, int index = 0, int len = -1 );
    string &insert ( const string &s, int index = 0 );
    string &insert ( const char *s, int index = 0 );
    int index_of ( const string &s, int start_index = 0 ) const;
    int index_of ( const char *s, int start_index = 0 ) const;
    int index_of ( char c, int start_index = 0 ) const;
    int last_index_of ( char c, int start_index = -1 ) const;

    string& trim ();
    bool contains ( const string &s ) const;
    bool contains ( const char *s ) const;
    bool contains ( char c ) const;

    bool equals ( const char *s, bool nocase = true ) const;

    //
    // pattern chars:
    //  '*' - any substring
    //  '?' - any one char
    //  '['char set']' = any one char in set
    //    e.g.  [a-z] - all lowercase letters
    //          [a-zA-Z] - all letters
    //          [abd-z] - all lowercase letters except of 'c'
    //          [-a-z] - all lowercase letters and '-'
    // returns:
    //    -1 - no match otherwise start pos of match
    int  match ( const char *pattern ) const;

    string &printf ( const char *fmt, ... );

    static string format ( const char *fmt, ... );

#ifndef NO_ARRAY
    array<string> tokens ( const char *separators = " \t\n\v\r\f" ) const;
    array<string> tokens ( char separator ) const;
#endif
    string& read_line ( istream &stream );
    bool    read_token ( istream &stream, const char *separators = " \t\n\v\r\f" );
    bool    read_until ( istream &stream, const char *separators );

    int replace ( const char *from,const char *to );

  protected:
    class data
    {
    public:
      data () : ref_count ( 0 ), length ( 0 )
      {
        chars [ 0 ] = '\0';
      }
      unsigned int ref_count;
      int allocated;
      int length;
      char chars [ 1 ];
    };
    static data *new_data ( int length, int ref_count );
    void set_length ( int length, bool preserve_content = false );
    void set_data ( data *data );
    void release_data ();
    void make_unique ();

    char *chars ();
    const char *chars () const;
    string& replace_substr ( const char *s, int s_len, int index, int len );
    string& insert ( const char *s, int s_length, int index );

  protected:
    data *my_data;
    static data null_data;
  };

  /***************************************************************************/

#ifndef NO_LIST
#include "cs_list.h"
#endif

  inline char *
    string::chars ()
  {
    return my_data->chars;
  }

  inline const char *
    string::chars() const
  {
    return my_data->chars;
  }

  inline istream &
    operator>> ( istream &stream, string &s )
  {
    s.read_token ( stream );
    return stream;
  }

  inline ostream &
    operator<< ( ostream &stream, const string &s )
  {
    stream << s.chars ();
    return stream;
  }

  inline bool
    operator== ( const char *s1, const string &s2 )
  {
    return ( strcmp ( s1, s2.chars () ) == 0 );
  }

  inline bool
    operator< ( const char *s1, const string &s2 )
  {
    return ( strcmp ( s1, s2.chars () ) < 0 );
  }

  inline bool
    operator<= ( const char *s1, const string &s2 )
  {
    return ( strcmp ( s1, s2.chars () ) <= 0 );
  }

  inline bool
    operator> ( const char *s1, const string &s2 )
  {
    return ( strcmp ( s1, s2.chars () ) > 0 );
  }

  inline bool
    operator>= ( const char *s1, const string &s2 )
  {
    return ( strcmp ( s1, s2.chars () ) >= 0 );
  }

  inline bool
    operator!= ( const char *s1, const string &s2 )
  {
    return ( strcmp ( s1, s2.chars () ) != 0 );
  }

  inline void 
    swap ( string &s1, string &s2 )
  {
    string::data *tmp = s1.my_data;
    s1.my_data = s2.my_data;
    s2.my_data = tmp;
  }

  inline bool
    string::equals ( const char *s, bool nocase ) const
  {
    if ( nocase )
      return ( stricmp ( my_data->chars, s ) == 0 );
    else
      return ( strcmp ( my_data->chars, s ) == 0 );
  }

  inline
    string::string () : my_data ( &null_data )
  {
    /* do nothing */
  }

  inline
    string::string ( const string &s ) : my_data ( &null_data )
  {
    set_data ( s.my_data );
  }

  inline
    string::string ( const char *s ) : my_data ( &null_data )
  {
    if ( s )
    {
      const int length = ::strlen ( s );
      set_length ( length );
      ::strncpy ( chars (), s, length );
    }
  }

  inline
    string::string ( const uchar *s )
  {
    size_t slen = wcslen ( s );
    my_data = new_data ( slen, 1 );
    wcstombs ( chars (), s, slen );
  }

  inline
    string::string ( const char *s, int count ) : my_data ( &null_data )
  {
    set_length ( count );
    ::strncpy ( chars (), s, count );
  }

  inline
    string::string ( char c, int n ) : my_data ( &null_data )
  {
    set_length ( n );
    ::memset ( chars (), c, n );
  }

  inline
    string::~string ()
  {
    release_data ();
  }

  inline int
    string::length () const
  {
    return my_data->length;
  }

  inline
    string::operator const char * () const
  {
    return my_data->chars;
  }

  inline char *
    string::buffer ()
  {
    make_unique ();
    return my_data->chars;
  }

  inline char &
    string::operator[] ( int index )
  {
    if ( index < 0 )
      index += length();
    assert ( index >= 0 && index < length () );
    make_unique ();
  return chars() [ index ];
  }


  inline char
    string::operator[] ( int index ) const
  {
    if ( index < 0 )
      index += length ();
    assert ( index >= 0 && index < length() );

    return chars() [ index ];
  }


  inline string &
    string::operator= ( const string &s )
  {
    set_data ( s.my_data );
    return *this;
  }

  inline string &
    string::operator=(const char *s)
  {
    const int length = ::strlen ( s );
    set_length ( length );
    ::memcpy ( chars(), s, length );
    return *this;
  }

  inline string &
    string::operator= ( const uchar *s )
  {
    const int length = ::wcslen ( s );
    int mblength = ::wcstombs ( 0, s, length );
    set_length ( mblength );
    ::wcstombs ( chars(), s, length );
    return *this;
  }

  inline bool
    string::operator== ( const string &s ) const
  {
    return ( length() == s.length() ) &&
           ( ::memcmp ( chars(), s.chars(), length() ) == 0 );
  }

  inline bool
    string::operator== ( const char *s ) const
  {
    return ( strcmp ( chars(), s ) == 0 );
  }

  inline bool
    string::operator< ( const string &s ) const
  {
    return ( strcmp ( chars(), s.chars() ) < 0 );
  }

  inline bool
    string::operator< ( const char *s ) const
  {
    return ( strcmp ( chars(), s ) < 0 );
  }

  inline bool string::operator<=(const string &s) const
  {
    return ( strcmp ( chars(), s.chars() ) <= 0 );
  }

  inline bool
    string::operator<= ( const char *s ) const
  {
    return (strcmp(chars(), s) <= 0);
  }

  inline bool
    string::operator> ( const string &s ) const
  {
    return ( strcmp ( chars(), s.chars() ) > 0 );
  }

  inline bool
    string::operator> ( const char *s ) const
  {
    return ( strcmp ( chars(), s ) > 0 );
  }

  inline bool
    string::operator>= ( const string &s ) const
  {
    return ( strcmp ( chars(), s.chars() ) >= 0 );
  }

  inline bool
    string::operator>= ( const char *s ) const
  {
    return ( strcmp ( chars(), s ) >= 0 );
  }

  inline bool
    string::operator!= ( const string &s ) const
  {
    return ( length() != s.length() ) ||
           ( memcmp ( chars(), s.chars(), length() ) != 0 );
  }

  inline bool
    string::operator!= ( const char *s ) const
  {
    return ( strcmp ( chars(), s ) != 0 );
  }

  inline string &
    string::operator+= ( const string &s )
  {
    *this = *this + s;
    return *this;
  }

  inline string &
    string::operator+= ( const char *s )
  {
    *this = *this + s;
    return *this;
  }

  inline bool
    string::is_empty () const
  {
    return my_data == &null_data;
  }

  inline string
    string::copy () const
  {
    string newstring ( *this );
    return newstring;
  }

  inline string &
    string::clear()
  {
    set_length ( 0 );
    return *this;
  }

  inline int
    string::index_of ( const string &s, int start_index ) const
  {
    return index_of ( s.chars(), start_index );
  }

  inline bool
    string::contains(const string &s) const
  {
    return ( index_of ( s, 0 ) >= 0 );
  }

  inline bool
    string::contains ( const char *s ) const
  {
    return ( index_of ( s, 0 ) >= 0 );
  }

  inline bool
    string::contains ( char c ) const
  {
    return ( index_of ( c, 0 ) >= 0 );
  }

  inline string &
    string::replace_substr ( const string &s, int index, int len )
  {
    return replace_substr ( (const char *) s, s.length(), index, len );
  }

  inline string &
    string::insert ( const string &s, int index )
  {
    return insert ( s.chars(), s.length(), index );
  }

  inline string &
    string::insert ( const char *s, int index )
  {
    return insert ( s, strlen ( s ), index );
  }

/****************************************************************************/
};

#endif /* string_defined */
