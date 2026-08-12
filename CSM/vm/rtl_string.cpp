/*
*
* rtl_string.cpp
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
  // STRING CLASS

  // string::string
  VALUE
    STRING_CLASS::ctor ( int argc, VALUE *argv )
  {
    if ( argc == 1 )
    {
      if ( argv [ 0 ].is_int () )
      {
        string s ( char ( argv->v.v_integer ), 1 );
        return VALUE ( s );
      }
      return VALUE ( argv->to_STRING () );
    }

    //TODO: conversion

    return VALUE ( new STRING ()  );
  }

  // string::length
  VALUE
    STRING_CLASS::length ( int argc, VALUE *argv )
  {
    if ( argc == 0 ) // get
      return VALUE ( (int) _this_.v.v_string->size () );
    else if ( argc == 1 ) // set
      error_read_only ();

    return VM::undefined;
  }

  // string::toString
  VALUE
    STRING_CLASS::tostring ( int argc, VALUE *argv )
  {
    return _this_;
  }

  // string::substr
  VALUE
    STRING_CLASS::substr ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;

    int start = 0;
    int length = 0;

    if ( argc == 1 && argv [ 0 ].is_number () )
    {
      start = int ( argv [ 0 ] );
    }
    else if ( argc == 2 && argv [ 0 ].is_number () && argv [ 1 ].is_number () )
    {
      start  = int ( argv [ 0 ] );
      length = int ( argv [ 1 ] );
    }
    else
      error_parameters ();

    STRING * s = 0;

    if ( start < 0 )
      start = 0;

    if ( start >= (int) me->size () )
      goto EMPTY;

    if ( start + length > (int) me->size () )
      length = me->size () - start;

    if ( length <= 0 )
      goto EMPTY;

    s = new STRING ( CSTR ( me ).substr ( start, length ) );

    return VALUE ( s );

  EMPTY:
    return VALUE ( new STRING () );
  }

  // string::substr
  VALUE
    STRING_CLASS::slice ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;
    if ( argc == 1 && argv [ 0 ].is_number () )
      return VALUE ( me->slice ( int ( argv [ 0 ] ), me->size () ) );
    else if ( argc == 2 && argv [ 0 ].is_number () && argv [ 1 ].is_number () )
      return VALUE ( me->slice ( int ( argv [ 0 ] ), int ( argv [ 1 ] ) ) );

    error_parameters ();

    return VM::undefined;
  }

  VALUE
    STRING_CLASS::tolcase ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;
    string s ( CSTR ( me ) );
    s.to_lower ();
    return VALUE ( new STRING ( s ) );
  }

  VALUE
    STRING_CLASS::toucase ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;
    string s ( CSTR ( me ) );
    s.to_upper ();
    return VALUE ( new STRING ( s ) );
  }

  VALUE
    STRING_CLASS::like ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;
    STRING *pattern;
    VM::arguments ( argc, argv, "s", &pattern );
    return ( CSTR ( me ) .match ( CSTR ( argv [ 0 ].v.v_string ) ) >= 0 );
  }

  VALUE
    STRING_CLASS::match ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;
    STRING *pattern;
    VM::arguments ( argc, argv, "s", &pattern );
    return CSTR ( me ).match ( CSTR ( argv [ 0 ].v.v_string ) );
  }

  VALUE
    STRING_CLASS::item ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;
    chktype ( 0, DT_INTEGER );
    int idx = int ( argv [ 0 ] );
    if ( idx < 0 || idx >= (int) me->size () )
      VM::error ( "Index out of bounds" );

    if ( argc == 2 ) // set
    {
      chktype ( 1, DT_INTEGER );
      ( *me ) [ idx ] = (unsigned char) int ( argv [ 1 ] );
    }

    return (int) ( (unsigned char) ( *me ) ( idx ) );
  }

  VALUE
    STRING_CLASS::clear ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;
    me->clear ();
    return VM::undefined;
  }

  VALUE
    STRING_CLASS::printf ( int argc, VALUE *argv )
  {
    if ( argc && argv [ 0 ].is_string () )
    {
      STRING *me = new STRING ( format ( CSTR ( argv [ 0 ].v.v_string ),
                                         argc - 1,
                                         argv + 1
                                       )
                              );

      return VALUE ( me );
    }
    else
      error_parameters ();

    return VM::null;
  }

  VALUE
    STRING_CLASS::split ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;

    if ( argc != 1 )
      error_parameters ();

    array<string> tkn;
    if ( argv [ 0 ].is_int () )
    {
      tkn = CSTR ( me ).tokens ( char ( argv [ 0 ].v.v_integer & 0xFF ) );
    }
    else if ( argv [ 0 ].is_string () )
    {
      tkn = CSTR ( me ).tokens ( CSTR ( argv [ 0 ].v.v_string ) );
    }
    else
      error_parameters ();

    ARRAY *a = new ARRAY ( tkn.size () );

    for ( int i = 0; i < tkn.size  (); i++ )
    {
      ( *a ) [ i ] = new STRING ( tkn [ i ] );
    }
    return a;
  }

  VALUE
    STRING_CLASS::trim ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;
    string s ( CSTR ( me ) );
    s.trim ();
    return VALUE ( new STRING ( s ) );
  }

  VALUE
    STRING_CLASS::replace ( int argc, VALUE *argv )
  {
    STRING *me = _this_.v.v_string;
    string what;
    string to;

    if ( (argc % 2 ) != 0 )
      error_parameters ();

    string r = CSTR ( me );

    for ( int i = 0; i < argc; i += 2 )
    {
      what.clear ();
      to.clear ();

      if ( argv [ i ].is_int () )
        what += char ( argv [ i ].v.v_integer & 0xFF );
      else if ( argv [ i ].is_string () )
        what = CSTR ( argv [ i ].v.v_string );
      else error_parameters ();

      int i1 = i + 1;

      if ( argv [ i1 ].is_int () )
        to += char ( argv [ i1 ].v.v_integer & 0xFF );
      else if ( argv [ i1 ].is_string () )
        to = CSTR ( argv [ i1 ].v.v_string );
      else error_parameters ();

      r.replace ( what, to );
    }

    return VALUE ( new STRING ( r ) );
  }

  VALUE
    STRING_CLASS::cast ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->to_STRING () );
  }

};
