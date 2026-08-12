/*
*
* cs_hash.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __cs_hash_h
#define __cs_hash_h

#include "cs_string.h"

namespace tool
{
  template<>
  inline unsigned int
    hash<string> ( const string& the_string )
  {
    unsigned int h = 0, g;
    char *pc = const_cast<char *> ( (const char *) the_string );
    while ( *pc )
    {
      h = ( h << 4 ) + *pc++;
      if ( ( g = h & 0xF0000000 ) != 0 )
        h ^= g >> 24;
      h &= ~g;
    }
    return h;
  }

  typedef const char * const_char_ptr_t;

  template<>
  inline unsigned int
    hash<const_char_ptr_t> ( const const_char_ptr_t& p_string )
  {
    unsigned int h = 0, g;
    char *pc = const_cast<char *> ( p_string );

    while ( *pc )
    {
      h = ( h << 4 ) + *pc++;
      if ( ( g = h & 0xF0000000 ) != 0 )
        h ^= g >> 24;
      h &= ~g;
    }
    return h;
  }


  template<>
  inline unsigned int
    hash<long> ( const long& the_long )
  {
    return (unsigned int) the_long;
  }


  template<>
  inline unsigned int
    hash<int> ( const int& the_int )
  {
    return (unsigned int) the_int;
  }


  template<>
  inline unsigned int
    hash<word> ( const word& the_word )
  {
    return (unsigned int) the_word;
  }

  template<>
  inline unsigned int
    hash<dword> ( const dword& the_dword )
  {
    return (unsigned int) the_dword;
  }

};

#endif
