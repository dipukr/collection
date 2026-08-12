/*
*
* rtl_stream.cpp
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
  using namespace sal;

  // STREAM CLASS

  bool
    STREAM::INSTANCE::get ( char &c )
  {
    if ( buf.length () )
    {
      c = buf [ buf.length () - 1 ];
      buf.cut ( -1 );
      return true;
    }
    else
      return ios->get ( c );
  }

  void
    STREAM::INSTANCE::unget ( char c )
  {
    buf += c;
  }

  VALUE
    STREAM::close ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( me->ios == 0 )
      return VALUE ( false );

    bool r = me->ios->close ();
    me->base = 0;
    delete me->ios;
    me->ios = 0;

    return VALUE ( r );
  }

  VALUE
    STREAM::put ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( me->ios == 0 )
      return VM::undefined;

    //TODO: raise error
    while ( argc-- )
    {
      switch ( argv->v_type )
      {
      case DT_INTEGER:
        if ( !me->ios->put ( argv->v.v_integer  & 0xff ) )
          return VALUE ( false );
        break;
      case DT_STRING:
        if ( !me->ios->put ( CSTR ( argv->v.v_string ) ) )
          return VALUE ( false );
        break;
      default:
        error_parameters ();
      }
      ++argv;
    }
    return VALUE ( true );
  }

  void
    STREAM::fill_params ( array<string>& params, int argc, VALUE *argv )
  {
    for ( int i = 0; i < argc; ++i )
    {
      if ( argv [ i ].is_int () )
        params.push ( string ( char ( int ( argv [ i ] ) ) ) );
      else if ( argv [ i ].is_string () )
        params.push ( CSTR ( argv [ i ].v.v_string ) );
      else if ( argv [ i ].is_array () )
      {
        ARRAY &arr = *argv [ i ].v.v_vector;
        fill_params ( params, arr.size (), &arr [ 0 ] );
      }
      else
        error_parameters ();
    }
  }

  VALUE
    STREAM::get ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( me->ios == 0 )
      return VALUE ();

    me->last_match_arg = 0;

    //TODO: raise error
    if ( argc == 0 )
    {
      char c;
      if ( !me->get ( c ) )
      {
        me->is_eof = true;
        return VM::undefined;
      }
      return VALUE ( int ( c ) );
    }
    else
    {
      array<string> delimeters;
      fill_params ( delimeters, argc, argv );
      string buffer;
      me->last_match_arg = 0;
      int maxlen = 0;
      for ( int i = 0; i < delimeters.size (); ++i )
        if ( maxlen < delimeters [ i ].length () )
          maxlen = delimeters [ i ].length ();

      char c;
      while ( buffer.length () < maxlen )
      {
        if ( !me->get ( c ) )
        {
          if ( buffer.length () )
          {
            return VALUE ( new STRING ( buffer ) );
          }
          else
          {
            me->is_eof = true;
            return VM::undefined;
          }
        }
        buffer += c;
      }

      while ( ( !me->last_match_arg ) &&
              ( buffer.length () < MAX_STRING_BUFFER )
            )
      {
        for ( int i = 0; i < delimeters.size (); ++i )
        {
          string& dlm = delimeters [ i ];
          int len = min ( dlm.length (), maxlen );
          if ( strncmp ( dlm,
                       ( (const char *) buffer ) + buffer.length ()  - maxlen,
                       len ) == 0 )
          {
            me->last_match_arg = i + 1;
            int idx = buffer.length ()  - maxlen + len;
            for ( int k = buffer.length () - 1; k >= idx; k-- )
              me->unget ( buffer [ k ] );
            buffer.cut ( buffer.length () - maxlen );
            return VALUE ( new STRING ( buffer ) );
          }
        }
        if ( !me->get ( c ) )
        {
          if ( buffer.length () )
            break;
          else
          {
            me->is_eof = true;
            return VM::undefined;
          }
        }
        buffer += c;
      }

      return VALUE ( new STRING ( buffer ) );
    }
  }

  VALUE
    STREAM::get_match ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( argc )
      error_read_only ();

    return me->last_match_arg;
  }

  VALUE
    STREAM::printf ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( argc && argv [ 0 ].is_string () )
    {
      string s = format ( CSTR ( argv [ 0 ].v.v_string ), argc - 1, argv + 1 );
      me->ios->put ( (const char *) s);
    }
    else
      error_parameters ();

    return VALUE ( me );
  }

  VALUE
    STREAM::position ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( argc && argv [ 0 ].is_int () )
    {
      me->ios->position ( int ( argv [ 0 ] ) );
    }
    else if ( argc != 0 )
      error_parameters ();

    return VALUE ( (int) me->ios->position () );
  }

  // STREAM::INSTANCE::length
  VALUE
    STREAM::size ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( me->ios == 0 )
      return VALUE ();
    //TODO: raise error

    if ( argc == 0 ) // get
      return VALUE ( (int) me->ios->size () );
    else if ( argc == 1 ) // set
    {
      int i;
      //TODO: raise error if negative!
      VM::arguments ( argc, argv, "i", &i );
      me->ios->size ( i );
      return VALUE ( (int) me->ios->size () );
    }
    return VALUE ();
  }

  VALUE
    STREAM::read ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( argc != 2 || !argv [ 0 ].is_thing () || !argv [ 1 ].is_int () )
      error_parameters ();

    if ( argv [ 0 ].v.v_thing->get_class () != BLOB::INSTANCE::klass )
      error_parameters ();

    BLOB::INSTANCE *buffer = (BLOB::INSTANCE *) argv [ 0 ].v.v_thing;
    int sz = int ( argv [ 1 ] ), sz_read = 0;
    buffer->data.size ( sz );
    bool r = me->ios->read ( (char * ) &buffer->data [ 0 ], sz, &sz_read );
    buffer->data.size ( sz_read );
    if ( sz_read == 0 )
    {
      me->is_eof = true;
      return VM::undefined;
    }

    return VALUE ( sz_read );
  }

  VALUE
    STREAM::write ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( argc == 0 )
      error_parameters ();

    if ( (argc != 1 ) || ( !argv [ 0 ].is_thing () ) )
      error_parameters ();

    if ( argv [ 0 ].v.v_thing->get_class () != BLOB::INSTANCE::klass )
      error_parameters ();

    BLOB::INSTANCE *buffer = (BLOB::INSTANCE *) argv [ 0 ].v.v_thing;
    int start = 0;
    int length = buffer->data.size ();

    if ( argc > 1 )
    {
      if ( !argv [ 1 ].is_int () )
        error_parameters ();

      start = int ( argv [ 1 ] );
      if ( start < 0 )
        start = 0;

      if ( start > length )
        start = length;

      length = buffer->data.size () - start;
    }

    if ( argc > 2 )
    {
      if ( !argv [ 2 ].is_int () )
        error_parameters ();

      length = int ( argv [ 2 ] );

      if ( start + length > buffer->data.size () )
      {
        length = buffer->data.size () - start;
      }
    }

    if ( length <= 0 )
      VM::error ( "std::stream::write count = %d", length );

    int sz_written = 0;
    bool r = me->ios->write ( (char *) &buffer->data [ start ], length,
                              &sz_written );
    if ( sz_written == 0   )
    {
      return VM::undefined;
    }
    return VALUE ( sz_written );
  }

  // STREAM::INSTANCE::length
  VALUE
    STREAM::src ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( me->ios == 0 )
      return VM::null;

    if ( argc == 0 ) // get
      return VALUE ( *me->base );
    else
      error_read_only ();

    return VALUE ();
  }

  VALUE
    STREAM::eof ( int argc, VALUE *argv )
  {
    STREAM::INSTANCE *me = (STREAM::INSTANCE *) _this_.v.v_thing;

    if ( me->ios == 0 )
      return VM::null;

    if ( argc == 0 ) // get
      return VALUE ( me->is_eof );
    else
      error_read_only ();

    return VALUE ();
  }

  CLASS * STREAM::INSTANCE::klass = 0;


  STREAM::STREAM ( PACKAGE* package ) : CLASS ( "stream", 0, package  )
  {
    add_function ( "put",    put    );
    add_function ( "get",    get    );
    add_function ( "printf", printf );
    add_function ( "read",   read   );
    add_function ( "write",  write  );
    add_function ( "close",  close  );

    //enum whence_mode
    add_const ( "sw_absolute", VALUE ( int ( file::fw_absolute ) ) );
    add_const ( "sw_current",  VALUE ( int ( file::fw_current ) ) );
    add_const ( "sw_end",      VALUE ( int ( file::fw_end ) ) );

    add_property ( "get_match", get_match );
    add_property ( "source",    src );
    add_property ( "eof",       eof );
    add_property ( "position",  position );

    STREAM::INSTANCE::klass = this;

  }

};
