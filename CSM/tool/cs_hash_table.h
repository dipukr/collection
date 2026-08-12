/*
*
* cs_hash_table.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __cs_HASH_TABLE_H
#define __cs_HASH_TABLE_H

#include "cs_basic.h"
#include "cs_string.h"
#include "cs_array.h"
#include "cs_hash.h"

namespace tool
{

  template <class c_key> unsigned int hash ( const c_key &the_key );

  template <class c_key,class c_element>
  class hash_table
  {
  public:
    hash_table ( size_t hash_size = 36 )
    {
      _hash_size = hash_size;
      _table = new array<hash_item> [ hash_size ];
    }

    virtual  ~hash_table()
    {
      clear ();
      delete [] _table;
    }

    bool  exists ( const c_key& the_key );

    //
    // Add to hash table association of object with specified name.
    //
    c_element&  operator[] ( const c_key &the_key );
    c_element&  get ( int the_index );

    //
    // Search for object with specified name in hash table.
    // If object is not found false is returned.
    //
    bool  find ( const c_key& the_key, c_element& the_element ) const;
    bool  find ( const c_key& the_key, c_element* &the_element_ptr ) const;

    //
    // Remove object with specified name from hash table.
    //
    c_element  remove ( const c_key& the_key );

    int   get_index ( const c_key& the_key, bool create );
    void  clear ();

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
      c_key          _key;
      size_t         _index;

      hash_item () : _index ( 0 )
      {
      }

      hash_item ( const c_key& k, size_t i ) : _key ( k ), _index ( i )
      {
      }
    };
    size_t              _hash_size;
    array<hash_item> *  _table;

    array<c_element>    _array;
    c_element*          _get ( const c_key& the_key, bool create );

  public:
    const array<c_element>&
      elements () const
    {
      return _array;
    }
  };


  template <class c_key, class c_element>
  inline c_element&
    hash_table<c_key, c_element>::operator[] ( const c_key &the_key )
  {
    return *_get ( the_key, true );
  }

  template <class c_key, class c_element>
  inline bool
    hash_table<c_key, c_element>::find ( const c_key& the_key,
                                          c_element& the_element ) const
  {
    c_element *pe = const_cast<hash_table<c_key, c_element>*>
                    ( this )->_get ( the_key, false );
    if ( pe )
    {
      the_element = *pe;
      return true;
    }
    return false;
  }


  template <class c_key, class c_element>
  inline bool
    hash_table<c_key, c_element>::find ( const c_key& the_key,
                                          c_element* &the_element_ptr ) const
  {
    c_element *pe = const_cast<hash_table<c_key, c_element>*>
                    ( this )->_get ( the_key, false );
    if ( pe )
    {
      the_element_ptr = pe;
      return true;
    }
    return false;
  }


  template <class c_key, class c_element>
  inline c_element*
    hash_table<c_key, c_element>::_get ( const c_key& the_key, bool create )
  {
    int idx = get_index ( the_key, create );
    if ( idx < 0 )
      return 0;
    return &_array [ idx ];
  }

  template <class c_key, class c_element>
  inline int
    hash_table<c_key, c_element>::get_index ( const c_key& the_key,
                                              bool create )
  {
    size_t h = hash<c_key> ( the_key ) % _hash_size;
    int i;
    array<hash_item> &bucket = _table [ h ];

    for ( i = 0; i < bucket.size (); i++ )
    {
      const hash_item &it = bucket [ i ];
      if ( it._key == the_key )
        return it._index;
    }

    if ( create )
    {
      size_t ni = _array.size ();
      _array.size ( int ( ni + 1 ) );
      bucket.push ( hash_item ( the_key, ni ) );
      return ni;
    }
    return -1;
  }

  template <class c_key, class c_element>
  inline c_element
    hash_table<c_key, c_element>::remove ( const c_key& the_key )
  {
    size_t h = hash<c_key> ( the_key ) % _hash_size;
    c_element t;
    int i;
    array<hash_item> &bucket = _table [ h ];
    
    for ( i = 0; i < bucket.size(); i++ )
    {
      const hash_item &it = bucket [ i ];
      if ( it._key == the_key )
      {
        int index = it._index;
        t = _array.remove ( index );
        bucket.remove ( i );
        // adjust other references
        for ( h = 0; h < _hash_size; h++ )
        {
          array<hash_item> &bucket = _table [ h ];
          for ( i = 0; i < bucket.size(); i++ )
          {
            const hash_item &it = bucket [ i ];
            if ( it._index > index )
              it._index--;
          }
        }
        return t;
      }
    }
    return t;
  }


  template <class c_key, class c_element>
  inline c_element&
    hash_table<c_key, c_element>::get ( int the_index )
  {
    return _array [ the_index ];
  }

  template <class c_key, class c_element>
  inline bool
    hash_table<c_key, c_element>::exists ( const c_key& the_key )
  {
    return ( _get ( the_key, false ) != 0 );
  }

  template <class c_key, class c_element>
  inline void
    hash_table<c_key, c_element>::clear ()
  {
    for ( size_t i = 0; i < _hash_size; ++i )
      _table [ i ].clear ();
    _array.clear ();
  }

};

#endif
