/*
*
* rtl_blob.cpp
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
  // BLOB CLASS
  CLASS *BLOB::INSTANCE::klass = 0;

  VALUE
    BLOB::ctor ( int argc, VALUE *argv )
  {
    INSTANCE *me = new INSTANCE ();
    return VALUE ( me );
  }

  VALUE
    BLOB::length ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;
    if ( argc == 1 ) // set
    {
      chktype ( 0, DT_INTEGER );
      int sz = int ( argv [ 0 ] );
      if ( sz < 0 )
        sz = 0;
      byte zero = '\0';
      me->data.size ( sz, &zero );
    }
    return VALUE ( (int) me->data.size () );
  }

  VALUE
    BLOB::item ( int argc, VALUE *argv )
  {
    INSTANCE *me = (INSTANCE *) _this_.v.v_thing;
    chktype ( 0, DT_INTEGER );
    int idx = int ( argv [ 0 ] );
    if ( idx < 0 || idx >= ( int ) me->data.size () )
      VM::error ( "Index out of bounds" );

    if ( argc == 2 ) // set
    {
      chktype ( 1, DT_INTEGER );
      me->data [ idx ] = ( char ) ( int ) argv [ 1 ];
    }
    return int ( me->data [ idx ] );
  }

  VALUE
    BLOB::item_type ( int argc, VALUE *argv )
  {
    return VM::undefined;
  }

};
