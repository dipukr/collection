/*
*
* rtl_array.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#include <stdio.h>
#include <stdlib.h>
#include "c-smile.h"
#include "vm.h"
#include "arithmetic.h"
#include "rtl.h"
namespace c_smile
{
  // ARRAY instance
  ARRAY::ARRAY ( int n )
  {
    data.size ( n, &VM::undefined );
  }

  // markvector - mark a vector
  void
    ARRAY::mark ()
  {
    THING::mark ();
    int n = data.size ();
    if ( n )
    {
      VALUE *pv = &data [ 0 ];
      while ( --n >= 0 )
        ( pv++ )->mark ();
    }
  }

  VALUE &
    ARRAY::operator[] ( int i )
  {
    assert ( i < size () );
    return data [ i ];
  }

  const VALUE &
    ARRAY::operator[] ( int i ) const
  {
    assert ( i < size () );
    return data [ i ];
  }

  int
    ARRAY::push ( const VALUE& v )
  {
    int pos = size ();
    data.push ( v );
    return pos;
  }

  void
    ARRAY::size ( int newsize )
  {
    data.size ( newsize, &VM::undefined );
  }

  VALUE
    ARRAY::pop ()
  {
    if ( data.size () )
      return data.pop ();
    return VM::undefined;
  }

  void
    ARRAY::sort ( void *param,
                  bool ( *less ) ( void *param, VALUE& a, VALUE& b ),
                  int lo, int hi )
  {
    int i = lo;
    int j = hi;
    VALUE *xs = &data [ 0 ];
    VALUE pivot = xs [ ( i + j ) / 2 ];
    do
    {
      while ( ( *less ) ( param, xs [ i ], pivot ) )
        i++;
      while ( ( *less ) ( param, pivot, xs [ j ] ) )
        j--;
      if ( i <= j )
      {
        VALUE temp = xs [ i ];
        xs [ i ] = xs [ j ];
        xs [ j ] = temp;
        i++;
        j--;
      }
    }
    while ( i <= j );

    if ( lo < j )
      sort ( param, less, lo, j );
    if ( i < hi )
      sort ( param, less, i, hi );
  }

  void
    ARRAY::sort ( void *param,
                  bool ( *less ) ( void *param, VALUE& a, VALUE& b ) )
  {
    if ( size () > 1 )
      sort ( param, less, 0, size () - 1 );
  }

  ARRAY *
    ARRAY::slice ( int start, int end )
  {
    int st = start >= 0 ? start : ( size () + start );
    int en = end >= 0 ? end : ( size () + end );

    if ( en > size () )
      en = size ();

    int length = en - st;

    if ( length <= 0 || st >= size () )
      return new ARRAY ( 0 );

    ARRAY *ar = new ARRAY ( length );

    VALUE *src = &data [ st ];
    VALUE *dst = &ar->data [ 0 ];
    for ( int i = 0; i < length; i++ )
      *dst++ = *src++;
      
    return ar;
  }

  void
    ARRAY::remove ( int start, int end )
  {
    int st = start >= 0 ? start : ( size () + start );
    int en = end   >= 0 ? end   : ( size () + end );

    if ( en > size () )
      en = size ();

    int length = en - st;

    if ( length <= 0 || st >= size () )
      return;

    for ( int i = 0; i < length; i++ )
      data.remove ( st );
  }

  CLASS *
    ARRAY::get_class ()
  {
    return VM::class_array;
  }

    // ARRAY CLASS

  VALUE
    ARRAY_CLASS::ctor ( int argc, VALUE *argv )
  {
    if ( argc == 1 && argv [ 0 ].v_type == DT_INTEGER )
    {
      int sz = argv [ 0 ].v.v_integer;
      return VALUE ( new ARRAY ( sz < 0 ? -sz : sz ) );
    }
    else if ( argc == 1 && argv [ 0 ].v_type == DT_ARRAY )
    {
      ARRAY *src = argv [ 0 ].v.v_vector;
      ARRAY *dst = new ARRAY ( src->size () );
      for ( int i = 0; i < src->size (); i++ )
        ( *dst ) [ i ] = ( *src ) [ i ];
      return VALUE ( dst );
    }
    else
    {
      ARRAY * vec = new ARRAY ( argc );
      for ( int i = 0; i < argc; ++i )
        ( *vec ) [ i ] = argv [ i ];
      return VALUE ( vec );
    }
    return VALUE ( new ARRAY ( 0 ) );
  }

  VALUE
    ARRAY_CLASS::literal ( int argc, VALUE *argv )
  {
    ARRAY * vec = new ARRAY ( argc );
    for ( int i = 0; i < argc; ++i )
      ( *vec ) [ i ] = argv [ i ];

    return VALUE ( vec );
  }

  VALUE
    ARRAY_CLASS::length ( int argc, VALUE *argv )
  {
    argcount ( argc, 0 );
    return VALUE ( _this_.v.v_vector->size () );
  }

  VALUE ARRAY_CLASS::push ( int argc, VALUE *argv )
  {
    ARRAY *me = _this_.v.v_vector;
    for ( int i = 0; i < argc; i++ )
      me->push ( argv [ i ] );

    return me->size ();
  }

  VALUE
    ARRAY_CLASS::pop ( int argc, VALUE *argv )
  {
    argcount ( argc, 0 );
    return _this_.v.v_vector->pop ();
  }

  struct value_less_params
  {
    VM *vm;
    VALUE sort_func;
  };

  bool
    value_less_udf ( void *param, VALUE& a, VALUE& b )
  {
    value_less_params *p = ( value_less_params * ) param;
    VALUE ar [ 2 ];
    ar [ 0 ] = a;
    ar [ 1 ] = b;
    return bool ( p->vm->call ( p->sort_func, 2, ar ) );
  }

  bool
    value_less ( void *param, VALUE& a, VALUE& b )
  {
    VALUE v = op_lt ( (VM * ) param, a,b );
    return bool ( v );
  }


  VALUE
    ARRAY_CLASS::sort ( int argc, VALUE *argv )
  {
    if ( argc == 0 )
    {
      _this_.v.v_vector->sort ( VM::current (), value_less );
    }
    else if ( argc == 1 )
    {
      value_less_params p;
      p.vm = VM::current ();
      p.sort_func = *argv;
      _this_.v.v_vector->sort ( &p, value_less_udf );
    }
    return VM::undefined;
  }

  VALUE
    ARRAY_CLASS::slice ( int argc, VALUE *argv )
  {
    ARRAY *me = _this_.v.v_vector;
    if ( argc == 1 && argv [ 0 ].is_number () )
      return VALUE ( me->slice ( int ( argv [ 0 ] ), me->size () ) );
    else if ( argc == 2 && argv [ 0 ].is_number () && argv [ 1 ].is_number () )
      return VALUE ( me->slice ( int ( argv [ 0 ] ), int ( argv [ 1 ] ) ) );

    error_parameters ();

    return VM::undefined;
  }

  VALUE
    ARRAY_CLASS::concat ( int argc, VALUE *argv )
  {
    ARRAY *me = _this_.v.v_vector;
    for ( int i = 0; i < argc; i++ )
    {
      if ( argv [ i ].is_array () )
      {
        ARRAY *a = argv [ i ].v.v_vector;
        for ( int j = 0; j < a->size (); j++ )
          me->push ( (*a ) [ j ] );
      }
      else
        me->push ( argv [ i ] );
    }
    return me->size ();
  }

  VALUE
    ARRAY_CLASS::shift ( int argc, VALUE *argv )
  {
    argcount ( argc, 0 );
    ARRAY *me = _this_.v.v_vector;
    VALUE v;
    if ( me->size () )
    {
      v = ( *me ) [ 0 ];
      for ( int i = 0; i < me->size () - 1; i++ )
        me->data [ i ] = me->data [ i + 1 ];
      me->size ( me->size () - 1 );
    }
    return v;
  }

  VALUE
    ARRAY_CLASS::unshift ( int argc, VALUE *argv )
  {
    if ( argc == 0 )
      return VM::undefined;

    ARRAY *me = _this_.v.v_vector;
    int i;
    int oldsize = me->size ();
    me->size ( oldsize + argc );
    VALUE *src = &me->data [ oldsize - 1 ];
    VALUE *dst = &me->data [ me->size () - 1 ];
    for ( i = 0; i < oldsize ;i++ )
      *dst-- = *src--;
    dst = &me->data [ 0 ];
    for ( i = 0; i < argc; i++ )
      *dst++ = *argv++;

    return VM::undefined;
  }

  VALUE
    ARRAY_CLASS::tostring ( int argc, VALUE *argv )
  {
    ARRAY *me = _this_.v.v_vector;
    string dlmtr;
    if ( argc == 0 )
      dlmtr = ",";
    else if ( argc == 1 && argv [ 0 ].v_type == DT_STRING )
      dlmtr = CSTR ( argv [ 0 ].v.v_string );
    else
      error_parameters ();

    string s;
    int lastel = me->data.size () - 1;
    for ( int i = 0; i <= lastel; i++ )
    {
      VALUE v = me->data [ i ];
      s += string ( v );
      if ( i != lastel )
        s += dlmtr;
    }
    return VALUE ( new STRING ( s ) );
  }

  VALUE
    ARRAY_CLASS::item ( int argc, VALUE *argv )
  {
    ARRAY *me = _this_.v.v_vector;
    chktype ( 0, DT_INTEGER );
    int idx = int ( argv [ 0 ] );
    if ( idx < 0 || idx >= me->size () )
      VM::error ( "Index %d is out of bounds", idx );

    if ( argc == 2 ) // set
      me->data [ idx ] = argv [ 1 ];
    return me->data [ idx ];
  }

  VALUE
    ARRAY_CLASS::clear ( int argc, VALUE *argv )
  {
    ARRAY *me = _this_.v.v_vector;

    if ( argc )
      error_parameters ();
    me->size ( 0 );

    return VM::undefined;
  }

  VALUE
    ARRAY_CLASS::remove ( int argc, VALUE *argv )
  {
    ARRAY *me = _this_.v.v_vector;
    int start = 0;
    int end = 0;
    if ( argc == 1 && argv [ 0 ].is_number () )
      me->remove ( int ( argv [ 0 ] ), me->size () );
    else if ( argc == 2 && argv [ 0 ].is_number () && argv [ 1 ].is_number () )
      me->remove ( int ( argv [ 0 ] ), int ( argv [ 1 ] ) );
    else
      error_parameters ();

    return VM::undefined;
  }

};
