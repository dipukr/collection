/*
*
* threads.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov - kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

#include "c-smile.h"
#include "vm.h"
#include "rtl.h"
#include "task.h"

#if !defined ( _WIN32 ) 
#include <unistd.h>
#endif

namespace c_smile
{
#ifdef THIS
#undef THIS
#endif

#define THIS argv [ -1 ] 

  THREAD::INSTANCE::INSTANCE ( CODE *code, int argc, VALUE *argv ) 
  {

    vm = new VM ();

    vm->thread = this;
    vm->setup_stack ();
    vm->start_call ( code, argc, argv );
    vm->create_thread ();
  }

  THREAD::INSTANCE::INSTANCE ( CODE *code, THING *object, int argc, VALUE *argv ) 
  {

    vm = new VM ();

    vm->thread = this;
    vm->setup_stack ();
    vm->start_call ( code, object, argc, argv );
    vm->create_thread ();
  }

  THREAD::INSTANCE::~INSTANCE () 
  {
    assert ( !vm->running );
    if ( vm )
      delete vm;
  }


  VALUE
    THREAD::ctor ( int argc, VALUE *argv ) 
  {

    if ( argc >= 1 )
    {
      switch ( argv [ 0 ] .v_type ) 
      {
      case DT_CODE:
        {
          return VALUE ( new THREAD::INSTANCE ( argv [ 0 ].v.v_code,
                                                argc - 1,
                                                argv + 1 ) );
        }
      case DT_OBJECT_METHOD:
        {
          return VALUE ( new THREAD::INSTANCE ( argv [ 0 ].v.v_om.code,
                                                argv [ 0 ] .v.v_om.thing,
                                                argc - 1,
                                                argv + 1 ) );
        }
      }
    }

    return VALUE ();
  }



  VALUE
    THREAD::stop ( int argc, VALUE *argv ) 
  {
#pragma TODO ( "thread stop!" ) 

    return VM::undefined;
  }


  VALUE
    THREAD::send ( int argc, VALUE *argv ) 
  {

    THREAD::INSTANCE  * _this = (THREAD::INSTANCE *) THIS.v.v_thing;
    if ( argc == 0 )
      error_parameters ();

    for ( int i = 0; i < argc; i++ )
      _this->vm->queue.send ( argv [ i ] );
      
    return VM::undefined;
  }

  VALUE
    THREAD::wait ( int argc, VALUE *argv ) 
  {
    VM  * _current = VM::current ();
    VALUE v;

    if ( argc == 0 )
      _current->queue.wait ( v );
    else if ( argc == 1 ) 
    {
      if ( !argv [ 0 ].is_int () )
        error_parameters ();

      _current->queue.wait ( v, int ( argv [ 0 ] ) );
    }
    else
      error_parameters ();

    return v;
  }

  VALUE
    THREAD::sleep ( int argc, VALUE *argv ) 
  {
    if ( argc == 1 && argv [ 0 ].is_int () ) 
    {
#ifdef _WIN32
      Sleep ( int ( argv [ 0 ] ) );
#else
      usleep ( int ( argv [ 0 ] ) );
#endif
    }
    else
      error_parameters ();

    return VM::undefined;
  }

  VALUE
    THREAD::running ( int argc, VALUE *argv ) 
  {
    THREAD::INSTANCE  * _this = (THREAD::INSTANCE *) THIS.v.v_thing;

#ifdef _WIN32
    Sleep ( 0 );
#else
    ::sleep ( 0 );
#endif

    return VALUE ( _this->vm != 0 );
  }

  THREAD::THREAD ( PACKAGE *pkg ) : CLASS ( "thread", 0,  pkg ) 
  {
    add_function ( "thread",   ctor    );
    add_function ( "stop",     stop    );
    add_property ( "running",  running );

    add_function ( "send",     send    );
    add_static_function ( "wait",  wait  );
    add_static_function ( "sleep", sleep );


    THREAD::INSTANCE::klass = this;
  }

  CLASS * THREAD::INSTANCE::klass = 0;

  MUTEX::MUTEX ( PACKAGE *pkg ) : CLASS ( "mutex", 0,  pkg ) 
  {
    add_function ( "mutex", ctor );

    MUTEX::INSTANCE::klass = this;
  }

  CLASS* MUTEX::INSTANCE::klass = 0;

  VALUE MUTEX::ctor ( int argc,  VALUE *argv ) 
  {
    argcount ( argc, 0 );

    return new INSTANCE ();
  }

  void
    MSG_QUEUE::send ( const VALUE &v ) 
  {
    sal::critical_section cs ( _mutex );

    add_to_head ( v );
    if ( _awaiting > 0 ) 
    {

      _awaiting--;
      _event.signal ();
    }
  }

  bool MSG_QUEUE::wait ( VALUE &v, int timeout ) 
  {
    sal::critical_section cs ( _mutex );

    while ( size () == 0 ) 
    {
      _awaiting++;

      if ( timeout < 0 )
        _event.wait ();
      else
      {
        if ( _event.wait_with_timeout ( timeout ) )
          return false;
      }
    }

    v = tail ();
    remove_tail ();

    return true;
  }

  void
    MSG_QUEUE::mark () 
  {
    sal::critical_section cs ( _mutex );
    iterator<VALUE>       it ( *this );

    foreach ( it ) 
    {
      ( *it ).mark ();
    }
  }

};
