/*
*
* arithmetic.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov - kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

#ifndef __arithmetic_h
#define __arithmetic_h

#include "c-smile.h"
#include "tool.h"
#include "vm.h"

namespace c_smile
{
  inline void
    op_inc ( VM *vm,  VALUE& a ) 
  {
    switch ( a.v_type ) 
    {
    case DT_INTEGER:
      a.v.v_integer++;
      return;
    case DT_FLOAT:
      a.v.v_float++;
      return;
    default:
      vm->error ( "Not a number" );
    }
  }

  inline void
    op_dec ( VM *vm,  VALUE& a ) 
  {
    switch ( a.v_type ) 
    {
    case DT_INTEGER:
      a.v.v_integer--;
      return;
    case DT_FLOAT:
      a.v.v_float--;
      return;
    default:
      vm->error ( "Not a number" );
    }
  }

  inline VALUE
    op_add ( VM *vm,  const VALUE& a,  const VALUE& b ) 
  {
    if ( a.v_type == b.v_type ) 
    {
      switch ( a.v_type ) 
      {
      case DT_INTEGER: return VALUE ( a.v.v_integer + b.v.v_integer );
      case DT_FLOAT:   return VALUE ( a.v.v_float + b.v.v_float );
      case DT_STRING:  return VALUE ( new STRING ( *a.v.v_string + *b.v.v_string ) );
      }
    }
    else if ( a.is_string () || b.is_string () ) 
    {
      VALUE as = a.to_STRING ();
      VALUE bs = b.to_STRING ();
      return VALUE ( new STRING ( *as.v.v_string + *bs.v.v_string ) );
    }
    else
      switch ( a.v_type )
      {
        case DT_INTEGER:
          if ( b.v_type == DT_FLOAT )
            return VALUE ( double ( a.v.v_integer ) + b.v.v_float );
          break;
        case DT_FLOAT:
          if ( b.v_type == DT_INTEGER )
            return VALUE ( a.v.v_float + double ( b.v.v_integer ) );
          break;
      }
    vm->error ( "Incompatible type" );
    return VM::null;
  }

  inline VALUE
    op_sub ( VM *vm,  const VALUE& a,  const VALUE& b ) 
  {
    if ( a.v_type == b.v_type ) 
    {
      switch ( a.v_type ) 
      {
        case DT_INTEGER: return VALUE ( a.v.v_integer - b.v.v_integer );
        case DT_FLOAT:   return VALUE ( a.v.v_float - b.v.v_float );
      }
    }
    else
      switch ( a.v_type )
      {
      case DT_INTEGER:
        if ( b.v_type == DT_FLOAT )
          return VALUE ( double ( a.v.v_integer ) - b.v.v_float );
        break;
      case DT_FLOAT:
        if ( b.v_type == DT_INTEGER )
          return VALUE ( a.v.v_float - double ( b.v.v_integer ) );
        break;
      }
    vm->error ( "Not a number" );
    return VM::null;
  }

  inline VALUE
    op_mul ( VM *vm,  const VALUE& a,  const VALUE& b ) 
  {
    if ( a.v_type == b.v_type ) 
    {
      switch ( a.v_type ) 
      {
        case DT_INTEGER: return VALUE ( a.v.v_integer * b.v.v_integer );
        case DT_FLOAT:   return VALUE ( a.v.v_float * b.v.v_float );
      }
    }
    else
      switch ( a.v_type )
      {
      case DT_INTEGER:
        if ( b.v_type == DT_FLOAT )
          return VALUE ( double ( a.v.v_integer ) * b.v.v_float );
        break;
      case DT_FLOAT:
        if ( b.v_type == DT_INTEGER )
          return VALUE ( a.v.v_float * double ( b.v.v_integer ) );
        break;
      }
    vm->error ( "Not a number" );
    return VM::null;
  }

  inline VALUE
    op_div ( VM *vm,  const VALUE& a,  const VALUE& b ) 
  {
    if ( a.v_type == b.v_type ) 
    {
      switch ( a.v_type ) 
      {
      case DT_INTEGER:
        if ( b.v.v_integer == 0 )
          vm->error ( "Division by 0" );
        return VALUE ( a.v.v_integer / b.v.v_integer );
      case DT_FLOAT:
        if ( b.v.v_float == 0.0 )
          vm->error ( "Division by 0" );
        return VALUE ( a.v.v_float / b.v.v_float );
      }
    }
    else if ( a.is_number () && b.is_number () ) 
      return op_div ( vm,  VALUE ( double ( a ) ), VALUE ( double ( b ) ) );

    vm->error ( "Not a number" );
    return VM::null;
  }

  inline VALUE
    op_rem ( VM *vm,  const VALUE& a,  const VALUE& b ) 
  {
    if ( a.v_type == b.v_type ) 
    {
      switch ( a.v_type ) 
      {
      case DT_INTEGER:
        if ( b.v.v_integer == 0 )
          vm->error ( "Division by 0" );
        return VALUE ( a.v.v_integer % b.v.v_integer );
      }
    }
    else if ( a.is_number () && b.is_number () ) 
      return op_rem ( vm,  VALUE ( int ( a ) ), VALUE ( int ( b ) ) );

    vm->error ( "Not an integer number" );
    return VM::null;
  }

  inline VALUE
    op_band ( VM *vm, const VALUE& a, const VALUE& b ) 
  {
    if ( a.v_type == b.v_type && a.v_type == DT_INTEGER ) 
    {
      return VALUE ( a.v.v_integer & b.v.v_integer );
    }
    vm->error ( "Not an integer number" );
    return VM::null;
  }

  inline VALUE
    op_bor ( VM *vm,  const VALUE& a,  const VALUE& b ) 
  {
    if ( a.v_type == b.v_type && a.v_type == DT_INTEGER ) 
    {
      return VALUE ( a.v.v_integer | b.v.v_integer );
    }
    vm->error ( "Not an integer number" );
    return VM::null;
  }

  inline VALUE
    op_bxor ( VM *vm,  const VALUE& a,  const VALUE& b ) 
  {
    if ( a.v_type == b.v_type && a.v_type == DT_INTEGER ) 
    {
      return VALUE ( a.v.v_integer ^ b.v.v_integer );
    }
    vm->error ( "Not an integer number" );
    return VM::null;
  }

  inline VALUE
    op_bnot ( VM *vm,   const VALUE& a ) 
  {
    if ( a.v_type == DT_INTEGER )
      return VALUE ( ~a.v.v_integer );

    vm->error ( "Not an integer number" );
    return VM::null;
  }

  inline VALUE
    op_lt ( VM *vm,  const VALUE& a,  const VALUE& b ) 
  {
    if ( a.v_type == b.v_type ) 
    {
      switch ( a.v_type ) 
      {
      case DT_INTEGER: return VALUE ( a.v.v_integer < b.v.v_integer );
      case DT_FLOAT:   return VALUE ( a.v.v_float < b.v.v_float );
      case DT_STRING:  return VALUE ( *a.v.v_string < *b.v.v_string );
      }
    }
    else
      switch ( a.v_type )
      {
      case DT_INTEGER:
        if ( b.v_type == DT_FLOAT )
          return VALUE ( double ( a.v.v_integer ) < b.v.v_float );
        break;
      case DT_FLOAT:
        if ( b.v_type == DT_INTEGER )
          return VALUE ( a.v.v_float < double ( b.v.v_integer ) );
        break;
      }
    vm->error ( "Incompatible type" );
    return VM::null;
  }

  inline VALUE
    op_le ( VM *vm,  const VALUE& a,  const VALUE& b ) 
  {
    if ( a.v_type == b.v_type ) 
    {
      switch ( a.v_type ) 
      {
      case DT_INTEGER: return VALUE ( a.v.v_integer <= b.v.v_integer );
      case DT_FLOAT:   return VALUE ( a.v.v_float <= b.v.v_float );
      case DT_STRING:  return VALUE ( *a.v.v_string <= *b.v.v_string );
      }
    }
    else
      switch ( a.v_type )
      {
        case DT_INTEGER:
          if ( b.v_type == DT_FLOAT )
            return VALUE ( double ( a.v.v_integer ) <= b.v.v_float );
          break;
        case DT_FLOAT:
          if ( b.v_type == DT_INTEGER )
            return VALUE ( a.v.v_float <= double ( b.v.v_integer ) );
          break;
      }
    vm->error ( "Incompatible type" );
    return VM::null;
  }

  inline VALUE
    op_neq ( VM *vm,  const VALUE& a,  const VALUE& b ) 
  {
    if ( a.v_type == b.v_type ) 
    {
      switch ( a.v_type ) 
      {
      case DT_NULL:     return VALUE ( false );
      case DT_INTEGER:  return VALUE ( a.v.v_integer != b.v.v_integer );
      case DT_FLOAT:    return VALUE ( a.v.v_float != b.v.v_float );
      case DT_STRING:   return VALUE ( *a.v.v_string != *b.v.v_string );
      default:
        if ( a.is_thing () ) 
          return VALUE ( a.v.v_thing != b.v.v_thing );
        return VALUE ( a.v.data != b.v.data );
      }
    }
    else
      switch ( a.v_type ) 
      {
      case DT_NULL: return VALUE ( true );
      case DT_INTEGER:
        switch ( b.v_type ) 
        {
        case DT_INTEGER: return VALUE ( a.v.v_integer != b.v.v_integer );
        case DT_FLOAT: return VALUE ( double ( a.v.v_integer ) != b.v.v_float );
        }
        break;
      case DT_FLOAT:
        switch ( b.v_type ) 
        {
        case DT_INTEGER: return VALUE ( a.v.v_float != double ( b.v.v_integer ) );
        case DT_FLOAT: return VALUE ( a.v.v_float != b.v.v_float );
        }
      break;
      }
    return VALUE ( false );
  }

};

#endif
