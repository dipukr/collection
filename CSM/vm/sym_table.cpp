/*
*
* sym_table.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
//|
//| the module is part of CoolWind project v.1.0
//| written by Andrew Fedoniouk, december 1998.
//|

#include "sym_table.h"

namespace c_smile
{
  //using namespace TOOL;
  sym_table::~sym_table ()
  {
    clear ();
    delete _table;
  }

  symbol_t
    sym_table::operator [] ( const char *the_key  )
  {
    return get_index ( the_key, true );
  }

  bool
    sym_table::find ( const char *the_key, symbol_t& the_element )
  {
    symbol_t index = get_index ( the_key, false );
    if ( index ==  undefined_symbol )
      return false;
    the_element = index;
    return true;
  }

  symbol_t
    sym_table::get_index ( const char *the_key, bool create )
  {
    size_t h = hash<const char *> ( the_key ) % _hash_size;
    for ( hash_item* ip = _table [ h ];
          ip != NULL;
          ip = ip->_next  )
      if ( strcmp ( ip->_key, the_key ) == 0 )
        return ip->_index;

    if ( create )
    {
      symbol_t ni = _array.size ();
      hash_item *hi = new hash_item ( the_key, ni, _table [ h ] );
      _table [ h ] = hi;
      _array.push ( hi );
      return ni;
    }
    return undefined_symbol;
  }

  void
    sym_table::clear ()
  {
    for ( int i = _hash_size; --i >= 0;  )
    {
      hash_item* ip = _table [ i ];
      while ( ip != NULL  )
      {
        hash_item* _next = ip->_next;
        delete ip;
        ip = _next;
      }
      _table [ i ] = 0;
    }
    _array.clear ();
  }

  const char *
    sym_table::operator [] ( symbol_t sym  )
  {
    if ( (int) sym < 0 || ( int ) sym >= _array.size () )
      return "undefined";
    return _array [ sym ]->_key;
  }

};
