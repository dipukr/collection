/*
*
* cs_array.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __cs_array_h
#define __cs_array_h

//#include <iostream.h>
//#include <stdlib.h>
#include <stdio.h>
#include "cs_basic.h"

namespace tool
{
  // assumes element has a default constructor and operator=().

  template <class element>
  class array
  {
  public:
    enum error_type
    {
      out_of_bounds = 1,
      empty_array = 2
    };
    class error
    {
    protected:
      error_type    _type;
    public:
      error ( error_type et, const char *em = 0 ) : _type ( et )
      {
      }
    };

    array ();
    array ( int sz );
    array ( int sz, const element &init_element );
    array ( const array<element> &the_array );

    virtual ~array ();

    array<element> &operator= ( const array<element> &the_array );

    const element &operator[] ( int index ) const;
    element &operator[] ( int index );

    int       size () const;
    bool      is_empty () const;

    void      size ( int new_size, const element * init_element = 0 );
    void      clear ();

    element   remove ( int index );
    void      remove ( int index, int length );
    void      insert ( int index, const element& elem );

    void      set_all_to ( const element &element );

    int		    push ( const element& elem );
    void		  push ( const element* elems, int count )
    {
      for ( int i = 0;
                i < count;
                i++ )
        push ( elems [ i ] );
    }
    element   pop ();
    array<element> &
      operator+= ( const element &e )
    {
      push ( e );
      return *this;
    }

    element&        first ();
    const element&  first () const;
    element&        last  ();

    const element&  last  () const;
    const element&
      last_index () const
    {
      return _size - 1;
    }

  protected:
    int	 	 		_size;
    int				_allocated_size;
    element *	_elements;

  };

  template <class element>
  inline
    array<element>::array ()
  :_size ( 0 ), _allocated_size ( 0 ), _elements ( 0 )
  {
    ;/* do nothing */
  }

  template <class element>
  inline
    array<element>::array ( int sz )
  :_size ( 0 ), _allocated_size ( 0 ), _elements ( 0 )
  {
    size ( sz );
  }

  template <class element>
  inline
    array<element>::array ( int sz, const element &init_element )
  : _size ( 0 ), _allocated_size ( 0 ), _elements ( 0 )
  {
    size ( sz, &init_element );
  }


  template <class element>
  inline
    array<element>::array ( const array<element> &the_array )
  : _size ( 0 ), _allocated_size ( 0 ), _elements ( 0 )
  {
    operator= ( the_array );
  }


  template <class element>
    inline array<element>::~array ()
  {
    if ( _elements )
      delete [] _elements;
  }


  template <class element>
  inline void
    array<element>::clear ()
  {
    size ( 0 );
  }

  template <class element>
  inline
    array<element>& array<element>::operator= ( const array<element> &the_array )
  {
    if ( this == &the_array )
      return *this;
    size ( the_array._size );

    for ( int i = 0; i < _size; i++ )
      _elements [ i ] = the_array._elements [ i ];

    return *this;
  }

  template <class element>
  inline int
    array<element>::push ( const element& elem )
  {
    int pos = _size;
    size ( pos + 1 );
    operator[] ( pos ) = elem;
    return pos;
  }

  template <class element>
  inline element
    array<element>::pop ()
  {
    if ( _size <= 0 )
      throw error ( out_of_bounds );
    element e = _elements [ _size - 1 ];
    size ( _size - 1 );
    return e;
  }

  template <class element>
  inline element
    array<element>::remove ( int index )
  {
    if ( index < 0 || index >= _size )
      throw error ( out_of_bounds );
    element *dst = _elements + index;
    element r = *dst;
    _size--;
    for ( int i = index; i < _size; i++, dst++ )
      *dst = * ( dst + 1 );
    // dst->~element();
    return r;
  }

  template <class element>
  inline void
    array<element>::remove ( int index, int length )
  {
    if ( index < 0 || ( ( index + length ) > _size ) )
      throw error ( out_of_bounds );
    element *dst = _elements + index;
    _size -= length;
    for ( int i = index; i < _size; i++, dst++ )
      *dst = * ( dst + length );
  }


  template <class element>
  inline void
    array<element>::insert ( int index, const element& elem )
  {
    if ( index < 0 )
      index = 0;
    else if ( index >= _size )
      push ( elem );
    else
    {
      size ( _size + 1 );
      element *dst = _elements + _size - 1;
      for ( int i = _size - 1; i > index; i--, dst-- )
        *dst = *( dst - 1 );
      *( _elements + index ) = elem;
    }
  }


  template <class element>
  inline element &
    array<element>::operator[] ( int index )
  {
    if ( index < 0 || index >= _size )
      throw error ( out_of_bounds );
    return _elements [ index ];
  }

  template <class element>
  inline const element &
    array<element>::operator[] ( int index ) const
  {
    if ( index < 0 || index >= _size )
      throw error ( out_of_bounds );
    return _elements [ index ];
  }

  template <class element>
  inline element &
    array<element>::last ()
  {
    if ( _size == 0 )
      throw error ( out_of_bounds );
    return _elements [ _size - 1 ];
  }

  template <class element>
  inline const element &
    array<element>::last () const
  {
    if ( _size == 0 )
      throw error ( out_of_bounds );
    return _elements [ _size - 1 ];
  }

  template <class element>
  inline element &
    array<element>::first ()
  {
    if ( _size == 0 )
      throw error ( out_of_bounds );
    return _elements [ 0 ];
  }

  template <class element>
  inline const element &
    array<element>::first () const
  {
    if ( _size == 0 )
      throw error ( out_of_bounds );
    return _elements [ 0 ];
  }


  template <class element>
  inline int
    array<element>::size () const
  {
    return _size;
  }

  template <class element>
  inline bool
    array<element>::is_empty () const
  {
    return ( _size == 0 );
  }


  template <class element>
  inline void
    array<element>::size ( int new_size, const element * init_element )
  {
    if ( _size == new_size )
      return;
    if ( new_size > _size )
    {
      if ( new_size >  _allocated_size )
      {
        int i;
        _allocated_size = ( new_size < _allocated_size * 2 ) ?
                          _allocated_size * 2 : new_size;
        element *new_space = new element [ _allocated_size ];
        for ( i = 0; i < min ( _size, new_size ); i++ )
          new_space [ i ] = _elements [ i ];

        if ( _elements )
          delete [] _elements;
        _elements = new_space;

        if ( init_element )
          for ( i = _size; i < new_size; i++ )
          {
            //printf ( "array=%d\n", i );
            _elements [ i ] = *init_element;
          }
      }
    }
    else //new_size < _size
    {
      //while ( new_size < _size )
      //{
      //  --_size;
      //  _elements [ _size ].~element();
      //}
    }

    // else delete [] my_elements;
    // just set _size = 0;
    _size = new_size;
  }


  template <class element>
  inline void
    array<element>::set_all_to ( const element &the_element )
  {
    for ( int i = 0; i < _size; i++ )
      _elements [ i ] = the_element;
  }
};

#endif
