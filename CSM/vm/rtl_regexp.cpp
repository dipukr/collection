/*
*
* rtl_regexp.cpp
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
#include "rtl.h"

namespace c_smile
{
  // REGEXP CLASS

  CLASS *REGEXP::INSTANCE::klass = 0;

  void
    error_noexp ()
  {
    VM::error ( "no expression" );
  }

  VALUE
    REGEXP::ctor ( int argc, VALUE *argv )
  {
    INSTANCE *me = new INSTANCE ();

    if ( argc == 1 && argv [ 0 ].is_string () )
    {
      me->expression = argv [ 0 ];
      try
      {
        me->re.compile ( me->expression );
      }
      catch ( regexp::error& rer )
      {
        VM::error ( rer.description );
      }
    }
    else
      error_parameters ();

    return VALUE ( me );
  }

  VALUE
    REGEXP::compile ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 && argv [ 0 ].is_string () )
    {
      me->expression = argv [ 0 ];
      try
      {
        me->re.compile ( me->expression );
      }
      catch ( regexp::error& rer )
      {
        VM::error ( rer.description );
      }
    }
    else
      error_parameters ();

    return VM::undefined;
  }

  VALUE
    REGEXP::tostring ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    return VALUE ( me->expression );
  }

  VALUE
    REGEXP::item ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 1 )
    {
      return VALUE ( me->re [ int ( argv [ 0 ] ) ]);
    }
    error_read_only ();
    return VM::undefined;
  }

  VALUE
    REGEXP::length ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( argc == 0 )
      return VALUE ( me->re.count () );
    else
      error_read_only ();

    return VM::undefined;
  }

  VALUE
    REGEXP::match ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( ( argc == 1 ) && argv [ 0 ].is_string () )
    {
      if ( me->re.exec ( argv [ 0 ].to_string () ) )
        return VALUE ( me->re [ 0 ] );
      return VM::undefined;
    }
    error_parameters ();
    return VM::undefined;
  }

  VALUE
    REGEXP::test ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( ( argc == 1 ) && argv [ 0 ].is_string () )
    {
      return me->re.exec ( argv [ 0 ].to_string () );
    }
    error_parameters ();

    return VM::undefined;
  }

  VALUE
    REGEXP::split ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;

    if ( ( argc == 1 ) && argv [ 0 ].is_string () )
    {
      string          str = argv [ 0 ].to_string ();
      array<string>   matches;

      if ( me->re.split ( str, matches ) )
      {
        ARRAY *arr = new ARRAY ( matches.size () );
        for ( int i = 0; i < matches.size (); i++ )
          ( *arr ) [ i ] = VALUE ( matches [ i ] );
        return VALUE ( arr );
      }
      return VM::undefined;
    }
    error_parameters ();

    return VM::undefined;
  }

};
