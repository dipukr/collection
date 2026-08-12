/*
*
* sym_table.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __sym_table_h
#define __sym_table_h

//|
//| the module is part of CoolWind project v.1.0
//| written by Andrew Fedoniouk, december 1998.
//|

#include "tool.h"

namespace c_smile
{
  typedef unsigned int symbol_t;
#define undefined_symbol ( (symbol_t) ( -1 ) )

  class sym_table
  {
  public:
    sym_table ( size_t hash_size = 128 )
    {
      _hash_size = hash_size;
      _table = new hash_item* [ hash_size ];
      for ( unsigned i = 0; i < hash_size; i++ )
        _table [ i ] = 0;
    }
    virtual       ~sym_table ();

    bool
      exists ( const char *the_key )
    {
      return get_index ( the_key, false ) != undefined_symbol;
    }

    //
    // Add to hash table association of object with specified name.
    //
    symbol_t      operator [] ( const char * the_key );

    //
    // Return name of the given symbol.
    //
    const char *  operator [] ( symbol_t sym );

    //
    // Search for object with specified name in hash table.
    // return true if object is found.
    //
    bool          find ( const char * the_key, symbol_t& the_symbol );

    void          clear ();

    int
      size ()
    {
      return _array.size ();
    }

    bool
      is_empty ()
    {
      return _array.size () == 0;
    }

  protected:
    struct hash_item
    {
      symbol_t            _index;
      char *              _key;
      hash_item*          _next;

      hash_item ()
      {
        _next = 0;
      }

      hash_item ( const char *the_key, symbol_t the_index, hash_item* the_next )
      {
        _key = strdup ( the_key );
        _index = the_index;
        _next = the_next;
      }

      ~hash_item ()
      {
        free ( _key );
      }

    };

    size_t                    _hash_size;
    hash_item**               _table;

    array<struct hash_item *> _array;

  public:
    symbol_t get_index ( const char *the_key, bool create );

  };

};

#endif
