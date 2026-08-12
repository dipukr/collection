/*
*
* things.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#include <stdlib.h>
#include "c-smile.h"
#include "vm.h"
#include "mm.h"
#include "compiler.h"

namespace c_smile
{
  void*
    THING::operator new ( size_t size  )
  {
    return memory.alloc_thing ( size );
  }

  void*
    THING::operator new ( size_t size, int moresize  )
  {
    return memory.alloc_thing ( ( size_t ) ( (int ) size + moresize ) );
  }

  void
    THING::operator delete ( void* p )
  {
    /*do nothing*/
  }

  void
    THING::operator delete ( void* p, int moresize )
  {
    /*do nothing*/
  }

  THING::operator VALUE ()
  {
    return VALUE ( this );
  }

  bool
    THING::instance_of ( CLASS *cls )
  {
    CLASS *klass = get_class ();
    while ( klass  )
    {
      if ( klass == cls )
        return true;
      klass = klass->base;
    }
    return false;
  }

  STRING *
    STRING::slice ( int start, int end  )
  {
    int st = start >= 0 ? start : ( size () + start );
    int en = end >= 0 ? end : ( size () + end );

    if ( en > ( int ) size () ) en = size ();

    int length = en - st;

    if ( length <= 0 || st >= ( int ) size () ) return new STRING ();

    STRING *s = new STRING ( _data.substr ( st, length ) );

    return s;
  }

  CLASS *
    STRING::get_class  ()
  {
    return VM::class_string;
  }

  OBJECT::OBJECT ( CLASS *k ) : klass ( k ), tag ( 0 )
  {
    int n = klass->instance_size;
    VALUE *p = members;
    for ( int i = 0; i < n; ++i, ++p )
      p->init ();
  }

  void
    OBJECT::mark ()
  {
      THING::mark ();
      VALUE *p;
      int n;
      p = members;
      n = klass->instance_size;
      while ( --n >= 0 )
        ( p++ )->mark ();
      mark_thing ( klass );

  }

  CLASS *
    OBJECT::get_class  ()
  {
    return klass;
  }

  CODE::CODE ( symbol_t name, BUFFER *bytecode, CLASS *klass )
      : _name ( name ), _klass ( klass )
  {
    _code.bc = bytecode;
    _is_native = false;
  }

  CODE::CODE ( symbol_t name, BUILTIN_FUNC *code, CLASS *klass )
      : _name ( name ), _klass ( klass )
  {
      _code.native = code;
      _is_native = true;
  }

  void
    CODE::mark ()
  {
      THING::mark ();
      mark_thing ( _klass );
  }

  CLASS *
    CODE::get_class  ()
  {
      return 0;
  }

  string
    CODE::full_name ()
  {
      string t;
      t.printf ( "%s::%s", (const char *) _klass->full_name (),
                           VM::voc [ _name ] );
      return t;
  }

    // class
  CLASS::CLASS ( const char *name, CLASS *base, PACKAGE *package )
       : instance_size ( 0 ), ctor_function ( 0 ), item_function ( 0 ),
         cast_function ( 0 ), dtor_function ( 0 )
  {
    this->instance_size = 0;
    this->base = base;
    this->package = package;
    this->name = name ? VM::voc [ name ] : undefined_symbol;

    if ( base  )
    {
      ctor_function = base->ctor_function;
      item_function = base->item_function;
      cast_function = base->cast_function;
    }

    members = new DICTIONARY ();
    if ( package )
    {
      ENTRY e = package->add ( name, ST_SDATA );
      *( e.value () ) = this;
    }
  }

  void
    CLASS::mark ()
  {
    THING::mark ();

    mark_thing ( base );
    mark_thing ( members );
    mark_thing ( package );

    mark_thing ( ctor_function );
    mark_thing ( item_function );
    mark_thing ( cast_function );
    mark_thing ( dtor_function );

  }

  VALUE
    CLASS::create_instance ()
  {
    return VALUE ( new ( ( instance_size - 1 ) * sizeof ( VALUE ) )
                       OBJECT ( this ) );
  }

  string
    CLASS::full_name ()
  {
    string s;
    if ( package )
    {
      s += package->full_name ();
      s += "::";
    }
    s += VM::voc [ name ];
    return s;
  }

  CLASS *
    CLASS::get_class  ()
  {
    return 0;
  }

#define DICT_GRAN 8

  DICTIONARY *
    DICTIONARY::create ( int size )
  {
    int gran_size = ( size / DICT_GRAN + 1 ) * DICT_GRAN - 1;
    DICTIONARY *d = new ( sizeof ( DICTIONARY::item ) * gran_size )
                        DICTIONARY ();
    d->_size = size;
    d->_allocated_size = gran_size + 1;
    for ( int i = 0; i < d->_size; i++ ) d->_items [ i ].init ();
      return d;
  }

  DICTIONARY *
    DICTIONARY::realloc ( int newsize )
  {
    if ( newsize < _allocated_size  )
    {
      _size = newsize;
      return this;
    }
    else
    {
      DICTIONARY * nd = DICTIONARY::create ( newsize );
      for ( int i = 0; i < _size; i++ ) nd->_items [ i ] = _items [ i ];
        return nd;
    }
  }

  void
    DICTIONARY::mark  ()
  {
    THING::mark ();
    for ( int i = 0; i < _size; i++ )
      _items [ i ].value.mark ();
  }

  CLASS *
    DICTIONARY::get_class ()
  {
      return 0;
  }

  ENTRY
    CLASS::add ( const char *name, int type )
  {
    ENTRY e;
    symbol_t sym = VM::voc [ name ];

    if ( members->find ( sym ) >= 0 )
      return ENTRY::undefined (); //already defined

    e.index = members->size ();
    e.klass = this;

    members = members->realloc ( e.index + 1 );
    members->_items [ e.index  ].type = type;
    members->_items [ e.index  ].symbol = sym;

    return e;
  }

  ENTRY
    CLASS::find ( const char *name )
  {
    return find ( VM::voc [ name ] );
  }

  ENTRY
    CLASS::find ( symbol_t sym )
  {
    int idx = members->find ( sym );
    if ( idx < 0 )
      return ENTRY::undefined ();

    ENTRY e;
    e.index = idx;
    e.klass = this;
    return e;
  }

  VALUE
    CLASS::get ( unsigned int idx  )
  {
    const DICTIONARY::item& item = members->get ( idx );
    assert ( item.type != ST_DATA );
    return item.value;
  }

  int
    CLASS::get_type ( unsigned int idx  )
  {
    const DICTIONARY::item& item = members->get ( idx );
    return item.type;
  }

  void
    CLASS::set ( unsigned int idx, const VALUE& v  )
  {
    DICTIONARY::item& item = members->get ( idx );
    assert ( item.type == ST_SDATA );
    item.value = v;
  }

  int
    CLASS::add_function ( const char *name, BUILTIN_FUNC *fcn  )
  {
    VALUE *v = add ( name, ST_FUNCTION ) .value ();
    symbol_t sym = VM::voc [ name ];

    assert ( v );
    CODE *c = new CODE ( sym, fcn, this );
    *v = c;
    check_name ( sym, c, ST_FUNCTION );
    return members->find ( sym );
  }

  void
    CLASS::check_name ( symbol_t ns, CODE *c, int symbol_type )
  {
    switch ( ns  )
    {
    case sym_item:
      if ( symbol_type == ST_FUNCTION )
        item_function = c;
      break;
    case sym_cast:
      if ( symbol_type == ST_SFUNCTION )
        cast_function = c;
      break;
    default:
      if ( ns == name )
        ctor_function = c;
      break;
    }
  }

  int
    CLASS::add_static_function ( const char *name, BUILTIN_FUNC *fcn  )
  {
    VALUE *v = add ( name, ST_SFUNCTION ).value ();
    symbol_t sym = VM::voc [ name ];
    assert ( v );
    CODE *c = new CODE ( sym, fcn, this );
    *v = c;
    check_name ( sym, c, ST_SFUNCTION );
    return members->find ( sym );
  }

  int
    CLASS::add_property ( const char *name, BUILTIN_FUNC *fcn  )
  {
    VALUE *v = add ( name, ST_PROPERTY ).value ();
    assert ( v );
    symbol_t sym = VM::voc [ name ];
    *v = new CODE ( sym, fcn, this );
    return members->find ( sym );
  }

  int
    CLASS::add_static_data ( const char *name, const VALUE& vd  )
  {
    symbol_t sym = VM::voc [ name ];
    VALUE *v = add ( name, ST_SDATA ).value ();
    assert ( v );
    *v = vd;
    return members->find ( sym );
  }

  int
    CLASS::add_const ( const char *name, const VALUE& vd  )
  {
    VALUE *v = add ( name, ST_CONST ).value ();
    assert ( v );
    *v = vd;
    return members->find ( VM::voc [ name ] );
  }

  int
    CLASS::add_data ( const char *name, const VALUE& /*not used so far*/ )
  {
    ENTRY en = add ( name, ST_DATA );
    if ( !en.is_valid () )
      VM::error ( "Field '%s' already defined.", (const char *) name );
    VALUE *v = en.value ();
    int n = instance_size++;
    *v = n;
    return n;
  }

  int
    CLASS::add_static_property ( const char *name, BUILTIN_FUNC *fcn )
  {
    VALUE *v = add ( name, ST_SPROPERTY ).value ();
    assert ( v );
    *v = fcn;
    return members->find ( VM::voc [ name ] );
  }

  VALUE
    CLASS::to_string ( const VALUE *v  )
  {
    VM *vm = VM::current ();
    if ( vm )
    {
      return vm->send ( *(const_cast<VALUE *> ( v ) ), sym_to_string );
    }
    else
      return "<toString() undefined>";
  }

  VALUE::VALUE ( const char *c ) : v_type ( DT_STRING  )
  {
    v.v_string = new STRING ( c );
  }

  VALUE::VALUE ( const string& s ) : v_type ( DT_STRING )
  {
    v.v_string = new STRING ( s );
  }

  void
    VALUE::mark ()
  {
    switch ( v_type & 0xF  )
    {
    case DT_CLASS:
    case DT_OBJECT:
    case DT_ARRAY:
    case DT_CODE:
    case DT_STRING:
    case DT_EXT:
      mark_thing ( v.v_thing );
      break;
    case DT_VAR:
      v.v_var.mark ();
      break;
    case DT_OBJECT_METHOD:
      v.v_om.mark ();
    }
  }

  THING *
    VALUE::ext ( CLASS *klass ) const
  {
    if ( ( v_type == DT_EXT ) && ( v.v_thing->get_class () == klass ) )
      return v.v_thing;
    return 0;
  }

  void
    VALUE::OBJECT_METHOD::mark  ()
  {
    mark_thing ( thing );
    mark_thing ( code );
  }

  VALUE::operator bool ()  const
  {
    switch ( v_type )
    {
    case DT_NULL:	return false;
    case DT_STRING: return ( v.v_string->size ()  > 0 );
    case DT_FLOAT:  return ( v.v_float != 0.0 );
    case DT_INTEGER: return ( v.v_integer != 0 );
    }
    return true;
  }

  VALUE *
    ENTRY::value ()
  {
    if ( is_valid ()  )
      return ( &klass->members->get ( index ) .value );
    return 0;
  }

  bool
    ENTRY::is_valid ()
  {
    if ( klass )
      return ( (int) index < klass->members->size () );
    return false;
  }

  int &
    ENTRY::type  ()
  {
    assert ( klass );
    return klass->members->get ( index ) .type;
  }

  symbol_t &
    ENTRY::symbol  ()
  {
    assert ( klass );
    return klass->members->get ( index ).symbol;
  }

  symbol_t
    ENTRY::symbol () const
  {
    assert ( klass );
    return klass->members->get ( index ).symbol;
  }

  void
    ENTRY::mark ()
  {
    mark_thing ( klass );
  }

  // package
  PACKAGE::PACKAGE ( const char *name, const char *src ) : CLASS ( name, 0, 0 ), init_code ( 0 )
  {
    file_name = src? new STRING ( src ) : 0;
    literals = new ARRAY ( 0 );
  }

  // package
  PACKAGE::PACKAGE ( symbol_t t ) : CLASS ( t ), init_code ( 0 )
  {
    file_name = 0;
    literals = 0;
  }

  void
    PACKAGE::mark ()
  {
    CLASS::mark ();
    mark_thing ( literals );
    mark_thing ( file_name );
    mark_thing ( init_code );
  }

  int
    PACKAGE::add_literal ( const char *vs  )
  {
    size_t len = strlen ( vs );
    for ( int i = 0; i < literals->size (); i++  )
    {
      VALUE& v = ( *literals ) [ i ];
      if ( v.v_type == DT_STRING )
        if ( CSTR ( v.v.v_string ) == vs )
          return i;
    }

    return literals->push ( VALUE ( new STRING ( vs ) ) );
  }

  int
    PACKAGE::add_literal ( symbol_t vs  )
  {
    for ( int i = 0; i < literals->size (); i++  )
    {
      VALUE& v = ( *literals ) [ i ];
      if ( v.v_type == DT_SYMBOL && v.v.v_symbol == vs )
        return i;
    }

    VALUE v;
    v.set_symbol ( vs );
    return literals->push ( v );
  }

  int
    PACKAGE::add_literal ( const VALUE& vv  )
  {
    for ( int i = 0; i < literals->size (); i++  )
    {
      VALUE& v = ( *literals ) [ i ];
      if ( vv == v )
        return i;
    }

    return literals->push ( vv );
  }

  bool
    PACKAGE::has_reference_to ( symbol_t package_symbol )
  {
    for ( int i = 0; i < literals->size (); i++  )
    {
      VALUE& v = ( *literals ) [ i ];
      if ( v.v_type == DT_CLASS &&
           v.v.v_class->get_package ()  == 0 && // this is a package
           v.v.v_class->name == package_symbol  )
        return true;
    }
    return false;
  }

};
