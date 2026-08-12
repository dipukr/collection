/*
*
* rtl_map.cpp
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
#include "streams.h"

namespace tool
{
  template<>
  unsigned int hash<c_smile::VALUE> ( const c_smile::VALUE& v  )
  {
    if ( v.v_type == DT_STRING )
    {
      unsigned int h = 0, g;
      const char *pc = (const char *) CSTR ( v.v.v_string );
      while ( *pc )
      {
        h = ( h << 4 ) + *pc++;
        if ( ( g = h & 0xF0000000 ) != 0 )
          h ^= g >> 24;
        h &= ~g;
      }
      return h;
    }
    else
      return (unsigned int) v.v.v_integer;
  }

};

namespace c_smile
{
  // MAP CLASS

  CLASS *MAP::INSTANCE::klass = 0;

  void
    MAP::INSTANCE::mark ()
  {
    THING::mark ();

    for ( int i = 0; i < _map.size (); i++ )
    {
      _map.elements () [ i ].key.mark ();
      _map.elements () [ i ].element.mark ();
    }
  }

  VALUE
    MAP::ctor ( int argc, VALUE *argv )
  {
    INSTANCE *me = new INSTANCE ();
    return VALUE ( me );
  }

  VALUE
    MAP::tostring ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;
    return VALUE ();
  }

  VALUE
    MAP::add ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 2 )
    {
      VALUE *v;
      if ( me->_map.find ( argv [ 0 ], v ) )
        VM::error ( "item already exists" );
      me->_map [ argv [ 0 ] ] = argv [ 1 ];
    }
    else
      error_parameters ();

    return VM::undefined;
  }

  VALUE
    MAP::exist ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;
    VALUE *v;

    if ( argc == 1 )
      return me->_map.find ( argv [ 0 ], v );
    else
      error_parameters ();

    return VM::undefined;
  }

  VALUE
    MAP::item ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;
    VALUE *v;

    if ( argc == 2 )
    {
      me->_map [ argv [ 0 ] ] = argv [ 1 ];
      return argv [ 1 ];
    }

    if ( me->_map.find ( argv [ 0 ], v ) )
      return *v;
    else
      return VM::undefined;
  }

  VALUE
    MAP::remove ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 )
    {
      VALUE v;
      if ( me->_map.find ( argv [ 0 ], v ) )
      {
        me->_map.remove ( argv [ 0 ] );
        return v;
      }
      VM::error ( "item not found" );
    }
    else
      error_parameters ();

    return VM::undefined;
  }

  VALUE
    MAP::length ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;
    if ( argc == 0 )
      return me->_map.size ();
    else
      error_read_only ();

    return VM::undefined;
  }

  VALUE
    MAP::literal ( int argc, VALUE *argv )
  {
    INSTANCE *me = new INSTANCE ();
    assert ( ( argc & 1 ) == 0 );
    for ( int i = 0; i < argc; i += 2 )
    {
      VALUE *v;
      if ( me->_map.find ( argv [ i ], v ) )
        VM::error ( "item already exists" );
      me->_map [ argv [ i ]] = argv [ i+1 ];
    }
    return VALUE ( me );
  }

  VALUE
    MAP::key ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc != 1 && !argv [ 0 ].is_int () )
      error_parameters ();

    int idx = argv [ 0 ].v.v_integer;

    if ( idx < 0 || idx >= me->_map.size () )
      return VM::undefined;

    return me->_map.elements () [ idx ].key;
  }

  VALUE
    MAP::value ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;
    if ( argc != 1 && !argv [ 0 ].is_int () )
      error_parameters ();

    int idx = argv [ 0 ].v.v_integer;

    if ( idx < 0 || idx >= me->_map.size () )
      return VM::undefined;

    return me->_map.elements () [ idx ].element;
  }

  VALUE
    MAP::clear ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc )
      error_parameters ();

    me->_map.clear ();

    return VM::undefined;
  }

  THING *
    MAP::load ( archive *arc )
  {
    INSTANCE *me = new INSTANCE ();
    int sz = 0;
    arc->read ( sz );

    while ( sz-- )
    {
      VALUE key;
      VALUE value;
      arc->read_literal ( key );
      arc->read_literal ( value );
      me->_map [ key ] = value;
    }

    return me;
  }

  bool
    MAP::save ( THING *t, archive *arc )
  {
    INSTANCE *me = (INSTANCE *) t;
    int sz = me->_map.size ();
    arc->write ( sz );

    for ( int i = 0; i < sz; i++ )
    {
      arc->write_literal ( me->_map.key ( i ) );
      arc->write_literal ( me->_map.value ( i ) );
    }
    return true;
  }

};
