/*
*
* rtl.cpp
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
#include "scanner.h" // for TKNSIZE
#include "streams.h"
#include "arithmetic.h"
#include "rtl.h"
#include <locale.h>

namespace c_smile
{
  // badtype - report a bad operand type
  void
    badtype ( VALUE& vl, int type )
  {
    char tn1 [ 20 ];
    strcpy ( tn1, nameoftype ( vl.v_type ) );
    VM::error ( "Bad argument type" );
  }

  static VALUE xtypeof ( int argc, VALUE *argv );
  static VALUE xgc     ( int argc, VALUE *argv );
  static VALUE xsizeof ( int argc, VALUE *argv );
  static VALUE xprint  ( int argc, VALUE *argv );
  static VALUE xsystem ( int argc, VALUE *argv );

  void
    error_parameters ()
  {
    VM::error ( "Bad parameters." );
  }

  void
    error_read_only ()
  {
    VM::error ( "Property is read-only" );
  }

  void
    error_type ( CLASS *cls  )
  {
    VM::error ( "Must be an object of '%s' class or <null>.",
                (const char *) cls->full_name () );
  }


  /* xtypeof - get the data type of a value */
  static VALUE
    xtypeof ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv [ 0 ].v_type );
  }

  // xgc - invoke the garbage collector
  static VALUE
    xgc ( int argc, VALUE *argv )
  {
    argcount ( argc, 0 );
    return VALUE ( (int) memory.gc () );
  }

  // xsizeof - get the size of a vector or string
  static VALUE
    xsizeof ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    switch ( argv [ 0 ].v_type )
    {
    case DT_ARRAY:
      return VALUE ( argv [ 0 ].v.v_vector->size () );
    case DT_STRING:
      return VALUE ( (int) argv [ 0 ].v.v_string->size () );
    default:
      break;
    }
    return VALUE ( 0 );
  }


  // xprint - generic print function
  static VALUE
    xprint ( int argc, VALUE *argv )
  {
    string s;
    for ( int i = 0; i < argc; ++i )
    {
      s = argv [ i ];
      io_stream *ios = VM::sout;
      ios->put ( (const char *) s );
    }
    return VALUE ();
  }

  // xint - will make int value
  static VALUE
    xint ( int argc, VALUE *argv )
  {
    if ( argc == 0 )
      return 0;

    char *p;
    switch ( argv->v_type  )
    {
    case DT_NULL:     return 0;
    case DT_INTEGER:  return argv->v.v_integer;
    case DT_FLOAT:    return int ( argv->v.v_float );
    case DT_STRING:
      if ( argc == 1 )
        return atoi ( CSTR ( argv->v.v_string ) );
      else
        return strtol ( CSTR ( argv->v.v_string ), &p, int ( argv [ 1 ] ) );
    default:          return int ( argv->v.v_thing );
    }
  }

  // xfloat - will make float value
  static VALUE
    xfloat ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    switch ( argv->v_type )
    {
    case DT_NULL:     return 0.0;
    case DT_INTEGER:  return double ( argv->v.v_integer );
    case DT_FLOAT:    return argv->v.v_float;
    case DT_STRING:   return atof ( CSTR ( argv->v.v_string ) );
    default:          return (double) int ( argv->v.v_thing );
    }
  }


  // xnumber - will make either float or int value
  static VALUE
    xnumber ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    switch ( argv->v_type )
    {
    case DT_NULL:     return 0;
    case DT_INTEGER:
    case DT_FLOAT:    return argv [ 0 ];
    case DT_STRING:
      {
        string s ( CSTR ( argv->v.v_string ) );
        char *endptr;
        long lv = strtol ( s, &endptr, 0 );
        if ( *endptr != 0 )
        {
          double dv = strtod ( s, &endptr );
          if ( *endptr != 0 )
            return VM::undefined;
          return dv;
        }
        return VALUE ( lv );
      }
    default:          return int ( argv->v.v_thing );
    }
  }

  // xstring - will make string value
  static VALUE
    xstring ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->to_STRING () );
  }

  bool
    VALUE::is_bytecode ()  const
  {
    return ( v_type == DT_CODE ) && v.v_code->is_bytecode ();
  }

  bool
    VALUE::is_nativecode ()  const
  {
    return ( v_type == DT_CODE ) && v.v_code->is_native ();
  }

  VALUE::operator string ()  const
  {
    STRING *s = to_STRING ();
    return string ( CSTR ( s ) );
  }

  STRING *
    VALUE::to_STRING ()  const
  {
    if ( is_string () )
      return v.v_string;
    CLASS * cls = VM::get_class ( this );
    if ( cls == 0 )
    {
      string ss = to_string ();
      return new STRING ( (const char *) ss );
    }
    VALUE v = cls->to_string ( this );
    if ( !v.is_string () )
    {
      VM::error ( "%s.toString ()  returns '%s' instead of 'string'",
                  (const char *) cls->full_name (), nameoftype ( v.v_type ) );
    }
    return v.v.v_string;
  }

  string
    VALUE::to_string ()  const
  {
    string r;
    switch ( v_type )
    {
    case DT_NULL:
      if ( v.v_integer == 0 )
        return "<undefined>";
      else
        return "<null>";

    case DT_CLASS:
      {
        string classname = v.v_class->full_name ();
        r.printf ( "<class-%s>", (const char *) classname );
        return r;
      }
    case DT_OBJECT:
      {
        string classname = v.v_object->klass->full_name ();
        r.printf ( "<object of class-%s>", (const char *) classname );
        return r;
      }
    case DT_ARRAY:
      r.printf ( "<array [ %d ]>", v.v_vector->size () );
      return r;

    case DT_INTEGER:
      r.printf ( "%ld", v.v_integer );
      return r;

    case DT_FLOAT:
      r.printf ( "%f", v.v_float );
      return r;

    case DT_STRING:
      r = v.v_string->cstr ();
      return  r;

    case DT_CODE:
      {
        string classname = v.v_code->klass()->full_name ();
        string funcname = VM::voc [ v.v_code->name () ];
        r.printf ( "<%s method %s of class %s>",
                    v.v_code->is_bytecode () ? "bytecode":"native",
                    (const char *) funcname, (const char *) classname );
        return r;
      }
    case DT_VAR:
      {
        string classname = v.v_var.klass->full_name ();
        const char * entryname = VM::voc [ v.v_var.symbol () ];
        r.printf ( "<%s::%s>", (const char *) classname, entryname );
        return r;
      }
    case DT_SYMBOL:
      r.printf ( "<symbol-%d '%s'", v.v_symbol, VM::voc [ v.v_symbol ] );
      return r;

    case DT_OBJECT_METHOD:
      return "<object.method reference>";

    case DT_EXT:
      return "<extender>";

    }

    r.printf ( "undefined type: %d", v_type );
    return r;

  }

  bool
    operator == ( const VALUE& vl, const VALUE& vr )
  {
    if ( vl.v_type != vr.v_type )
      return false;
    if ( vl.v.data == vr.v.data )
      return true;
    if ( vl.v_type == DT_STRING )
      return VALUE ( *vl.v.v_string == *vr.v.v_string );
    return false;
  }

  // xsystem - execute a system command
  static VALUE
    xsystem ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    chktype ( 0, DT_STRING );
    return VALUE ( system ( CSTR ( argv [ 0 ].v.v_string ) ) );
  }

  static VALUE
    is_string ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->is_string () );
  }

  static VALUE
    is_array ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->is_array () );
  }

  static VALUE
    is_number ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( ( argv->v_type == DT_INTEGER ) ||
                   ( argv->v_type == DT_FLOAT ) );
  }

  static VALUE
    is_integer ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->is_int () );
  }

  static VALUE
    is_float ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->is_float () );
  }

  static VALUE
    is_null ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->v_type == DT_NULL );
  }

  static VALUE
    is_undefined ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( ( argv->v_type == DT_NULL ) && ( argv->v.v_integer == 0 ) );
  }

  static VALUE
    is_function ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->v_type == DT_CODE );
  }

  static VALUE
    is_method ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->v_type == DT_OBJECT_METHOD );
  }

  static VALUE
    is_object ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->v_type == DT_STRING ||
                   argv->v_type == DT_OBJECT ||
                   argv->v_type == DT_ARRAY ||
                   argv->v_type == DT_EXT );
  }

  static VALUE
    is_class ( int argc, VALUE *argv )
  {
    argcount ( argc, 1 );
    return VALUE ( argv->v_type == DT_CLASS );
  }

  class STD: public PACKAGE
  {
  public:
    STD ();

    static VALUE xlocale ( int argc, VALUE* argv );
  };


  STD::STD () : PACKAGE ( "std", "c-smile.cpp" )
  {
    add_static_function ( "typeof", xtypeof );
    add_static_function ( "gc",     xgc );
    add_static_function ( "sizeof", xsizeof );
    add_static_function ( "print",  xprint );
    add_static_function ( "system", xsystem );
    add_static_function ( "locale", xlocale );
    add_static_function ( "int",    xint );
    add_static_function ( "float",  xfloat );
    add_static_function ( "number", xnumber );

    add_static_function ( "is_string",    is_string );
    add_static_function ( "is_number",    is_number );
    add_static_function ( "is_array",     is_array );
    add_static_function ( "is_int",       is_integer );
    add_static_function ( "is_float",     is_float );
    add_static_function ( "is_null",      is_null );
    add_static_function ( "is_undefined", is_undefined );
    add_static_function ( "is_function",  is_function );
    add_static_function ( "is_method",    is_method );
    add_static_function ( "is_object",    is_object );
    add_static_function ( "is_class",     is_class );

    // setup the standard i/o streams

    add_static_data ( "in",  VALUE ( new STREAM::INSTANCE ( VM::sin  ) ) );
    add_static_data ( "out", VALUE ( new STREAM::INSTANCE ( VM::sout ) ) );
    add_static_data ( "err", VALUE ( new STREAM::INSTANCE ( VM::serr ) ) );

    add_static_data ( "true",  VALUE ( true  ) );
    add_static_data ( "false", VALUE ( false ) );

  }

  VALUE
    STD::xlocale ( int argc, VALUE *argv )
  {
    if ( argc ) //set
    {
      if ( argv [ 0 ].is_string () )
      {
        setlocale ( LC_ALL, CSTR ( argv [ 0 ].v.v_string ) );
      }
      else
        badtype ( argv [ 0 ], DT_STRING );
    }
    else
      error_parameters ();

    return VM::undefined;
  }

  class TYPE: public CLASS
  {
  public:
    TYPE ( PACKAGE* package ) : CLASS ( "type", 0, package )
    {
      add_static_data ( "NULL",     VALUE ( DT_NULL    ) );
      add_static_data ( "STRING",   VALUE ( DT_STRING  ) );
      add_static_data ( "FLOAT",    VALUE ( DT_FLOAT   ) );
      add_static_data ( "INTEGER",  VALUE ( DT_INTEGER ) );
      add_static_data ( "CLASS",    VALUE ( DT_CLASS   ) );

      add_static_data ( "OBJECT",   VALUE ( DT_OBJECT  ) );
      add_static_data ( "ARRAY",    VALUE ( DT_ARRAY   ) );
      add_static_data ( "FUNCTION", VALUE ( DT_CODE    ) );
      add_static_data ( "METHOD",   VALUE ( DT_OBJECT_METHOD ) );
    }
  };

  extern void init_threads ();

  void
    VM::init_std_package ()
  {
    std = new STD ();
    add_package ( std );

    VM::class_array = new ARRAY_CLASS ( std );
    VM::class_string = new STRING_CLASS ( std );

    new THREAD ( std );
    new MUTEX  ( std );
    new STREAM ( std );
    new DATE   ( std );
    new MAP    ( std );
    new SOCKET ( std );
    new BLOB   ( std );
    new NODE   ( std );
    new TYPE   ( std );
    new REGEXP ( std );

  }

  string
    format ( const char *fmt, int argc, VALUE *argv )
  {
    string out;
    string formatted;
    const char *pc = fmt, *pcold = fmt;
    int argi = 0;
    string cfmt;

    for ( ; *pc;  ++pc )
    {
      while ( *pc && *pc != '%' )
        out += *pc++;

      if ( *pc == 0 )
        break;

      if ( *++pc == '%' )
        out += '%';
      else if ( argi < argc )
        for ( cfmt = "%"; *pc ;++pc )
        {
          if ( *pc == '*' )
          {
            if ( argv [ argi ].is_int () )
              cfmt += argv [ argi++ ].to_string ();
            else
              VM::error ( "printf:argument %d is not an integer", argi );
          }
          else
          {
            cfmt += *pc;
            if ( *pc == 's' )
            {
              out += formatted.printf ( cfmt,
                                        (const char *)
                                        string ( argv [ argi++ ] )
                                      );
              break;
            }
            else if ( *pc == 'c'  )
            {
              if ( argv [ argi ].is_number () )
                out += formatted.printf ( cfmt, char ( 0xFF & int ( argv [ argi++ ] ) ) );
              else
                out += "<NaN>";
                break;
            }
            else if ( strchr ( "dioxXbu", *pc ) )
            {
              if ( argv [ argi ].is_number () )
                out += formatted.printf ( cfmt, int ( argv [ argi++ ] ) );
              else
                out += "<NaN>";
              break;
            }
            else if ( strchr ( "fgGeE", *pc ) )
            {
              if ( argv [ argi ].is_number () )
                out += formatted.printf ( cfmt, double ( argv [ argi++ ] ) );
              else
                out += "<NaN>";
              break;
            }
            else if ( *pc == 0 || *pc == '%' )
              break;
          }
        }
    }
    return out;
  }

};
