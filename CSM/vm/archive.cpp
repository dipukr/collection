/*
*
* archive.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#include "c-smile.h"
#include "vm.h"
#include "streams.h"
#include "osfile.h"

namespace c_smile
{
  symbol_t
    archive::map ( symbol_t extern_sym )
  {
    if ( extern_sym == undefined_symbol )
      return undefined_symbol;
    for ( int i = 0; i < _symbols.size(); ++i )
      if ( _symbols [ i ] == extern_sym )
        return symbol_t ( i );

    _symbols.push ( extern_sym );
    return symbol_t ( _symbols.size () - 1 );
  }

  void
    archive::write ( const char *buf, size_t sz )
  {
    int s ( sz );
    if ( !_body->write ( buf, s ) || (size_t) s != sz )
      VM::error ( (char *) (const char *) _body->err_message );
  }

  void
    archive::read ( char *buf, size_t sz )
  {
    int s ( sz );
    if ( !_body->read ( buf, s ) || (size_t) s != sz )
      VM::error ( (char *) (const char *) _body->err_message );
  }

  void
    archive::write_class_ref ( const CLASS *cls )
  {
    symbol_t package_name = ( ( cls->package ) ?
                                cls->package->name : undefined_symbol );
    symbol_t class_name = cls->name;
    write ( package_name );
    write ( class_name );
  }

  void
    archive::read_class_ref ( CLASS * &cls )
  {
    symbol_t package_name, class_name;
    read ( package_name );
    read ( class_name );
    if ( package_name == undefined_symbol )
    {
      // package
      cls = VM::find_package ( class_name );
    }
    else
    {
      cls = VM::find_class ( package_name, class_name );
    }
  }

  //|
  //| bool
  //|
  void
    archive::read ( bool & v )
  {
    read ( (char *) &v, sizeof ( v ) );
  }

  void
    archive::write ( bool v )
  {
    write ( (char *) &v, sizeof ( v ) );
  }

  //|
  //| int
  //|
  void
    archive::read ( int & v )
  {
    read ( (char *) &v, sizeof ( v ) );
  }

  void
    archive::write ( int v )
  {
    write ( (char *) &v, sizeof ( v ) );
  }

  //|
  //| double
  //|
  void
    archive::read ( double & v )
  {
    read ( (char *) &v, sizeof ( v ) );
  }

  void
    archive::write ( double v )
  {
    write ( (char *) &v, sizeof ( v ) );
  }

  //|
  //| string
  //|
  void
    archive::read ( string & s )
  {
    int l;
    read ( l );
    assert ( l < 1024 );
    string ss ( ' ', l );
    read ( ss.buffer(), l );
    s = ss;
  }

  void
    archive::write ( const char *s )
  {
    int l = strlen ( s );
    write ( l );
    write ( s, l );
  }

  //|
  //| DICTIONARY
  //|
  void
    archive::read ( DICTIONARY* &dict )
  {
    int sz = 0;
    read ( sz );
    if ( sz == -1 )
      dict = 0;
    else
    {
      dict = DICTIONARY::create ( sz );
      for ( int i = 0; i < sz; i++ )
      {
        DICTIONARY::item& it = ( *dict ) [ i ];
        read ( it.symbol );
        read ( it.type   );
        read ( it.value  );
      }
    }
  }

  void
    archive::write ( const DICTIONARY * dict )
  {
    if ( dict == 0 )
      write ( -1 );
    else
    {
      write ( dict->size () );
      for ( int i = 0; i < dict->size(); i++ )
      {
        const DICTIONARY::item& it = ( *dict ) [ i ];
        write ( it.symbol );
        write ( it.type   );
        write ( it.value  );
      }
    }
  }

  //|
  //| STRING
  //|
  void
    archive::write ( const STRING * str )
  {
    if ( str == 0 )
      write ( -1 );
    else
    {
      int sz = (int) str->size();
      write ( sz );
      if ( sz )
        write ( CSTR ( str ), (int) str->size() );
    }
  }

  void
    archive::read ( STRING* &str )
  {
    int l = 0;
    read ( l );
    if ( l == -1 )
      str = 0;
    else
    {
      string s ( ' ', l );
      read ( s.buffer(), l );
      str = new STRING ( s );
    }
  }

  //|
  //| ARRAY
  //|
  void
    archive::write ( const ARRAY * vec )
  {
    if ( vec == 0 )
      write ( -1 );
    else
    {
      write ( vec->size() );
      for ( int i = 0; i < vec->size(); ++i )
        write ( ( *vec ) [ i ] );
    }
  }

  void
    archive::read ( ARRAY* &vec )
  {
    int l;
    read ( l );
    if ( l < 0 )
      vec = 0;
    else
    {
      vec = new ARRAY ( l );
      for ( int i = 0; i < l; ++i )
        read ( ( *vec ) [ i ] );
    }
  }

  //|
  //| CLASS
  //|
#define PACKAGE_EXISTS 0x01
#define BASE_EXISTS 0x02

  void
    archive::read_class ( CLASS *cls )
  {
    int i_sz = 0;
    read ( i_sz );

    cls->instance_size = i_sz;

    int flags = 0;
    read ( flags );
    if ( flags & PACKAGE_EXISTS )
    {
      symbol_t sym = undefined_symbol;
      read ( sym );
      cls->package = VM::find_package ( sym );
    }
    if ( flags & BASE_EXISTS )
    {
      read_class_ref ( cls->base );
    }
    read ( cls->members );
    for ( int i = 0; i < cls->members->size(); i++ )
    {
      DICTIONARY::item& it = ( *cls->members ) [ i ];
      if ( it.value.is_bytecode() )
      {
        CODE *pbc = it.value.v.v_code;
        pbc->_klass = cls;
        pbc->_name = it.symbol;

        cls->check_name ( pbc->_name, pbc, it.type );
      }
    }
  }

  void
    archive::write_class ( const CLASS *cls )
  {
    write ( (int) cls->instance_size );

    int flags = 0;
    if ( cls->package )
      flags |= PACKAGE_EXISTS;
    if ( cls->base )
      flags |= BASE_EXISTS;

    write ( flags );

    if ( cls->package )
      write ( cls->package->name );
    if ( cls->base )
      write_class_ref ( cls->base );

    write ( cls->members );
  }

  void
    archive::write ( const CLASS * cls )
  {
    write ( cls->name );
    write_class ( cls );
  }

  void
    archive::read ( CLASS* &cls )
  {
    symbol_t t;
    read ( t );
    cls = new CLASS ();
    cls->name = t;
    read_class ( cls );
  }

  //|
  //| PACKAGE
  //|
  void
    archive::write ( const PACKAGE * pkg )
  {
    write ( pkg->name );
    write_class ( (CLASS *) pkg );

    write ( pkg->literals->size () );
    for ( int i = 0; i < pkg->literals->size (); i++ )
      write_literal ( ( *pkg->literals ) [ i ] );

    write ( pkg->init_code );
    write ( pkg->file_name );
  }

  void
    archive::read ( PACKAGE* &pkg )
  {
    symbol_t t;
    read ( t );
    pkg = new PACKAGE ( t );
    VM::add_package ( pkg );
    
    read_class ( (CLASS *) pkg );

    int sz = 0;
    read ( sz );
    pkg->literals = new ARRAY ( sz );
    for ( int i = 0; i < sz; i++ )
    read_literal ( ( *pkg->literals ) [ i ] );

    read ( pkg->init_code );
    pkg->init_code->klass ( pkg );
    read ( pkg->file_name );

    VM::run_init_code ( pkg );

  }

  //|
  //| BYTECODE
  //|
  void
    archive::read ( CODE* &c )
  {
    c = new CODE ();
    bool bc = false;
    read ( bc );
    c->_is_native = !bc;
    if ( bc )
    {
      int sz = 0; read ( sz );
      c->_code.bc = new BUFFER ( sz );
      read ( (char *) &( *c->_code.bc ) [ 0 ], sz );
    }
  }

  //|
  //| BYTECODE
  //|
  void
    archive::read_code_ref ( CODE* &c )
  {
    CLASS *cls = 0;
    read_class_ref ( cls );
    assert ( cls );
    symbol_t sym = undefined_symbol;
    read ( sym );
    ENTRY e = cls->find ( sym );
    assert ( e.is_valid () );
    assert ( e.type () == ST_FUNCTION || e.type () == ST_SFUNCTION );
    c = e.value ()->v.v_code;
  }

  void
    archive::write ( const CODE* c )
  {
    write ( c->is_bytecode () );
    if ( c->is_bytecode () )
    {
      write ( (int) c->_code.bc->size () );
      write ( (char *) &( *c->_code.bc ) [ 0 ], c->_code.bc->size () );
    }
  }

  void
    archive::write_code_ref ( const CODE* c )
  {
    write_class_ref ( c->_klass );
    write ( c->_name );
  }

  //|
  //| ENTRY
  //|
  void
    archive::read ( ENTRY &var )
  {
    read_class_ref ( var.klass );
    int idx = 0;
    read ( var.index );
  }

  void
    archive::write ( const ENTRY &var )
  {
    write_class_ref ( var.klass );
    write ( var.index );
  }

  //|
  //| SYMBOL
  //|
  void
    archive::read ( symbol_t& sym )
  {
    int i;
    read ( i ); //i - local (archive index);
    if ( (symbol_t) i == undefined_symbol )
      sym = undefined_symbol;
    else sym = _symbols [ i ];
  }
  
  void
    archive::write ( symbol_t sym )
  {
    int i = map ( sym );
    write ( i );
  }

  //|
  //| OBJECT
  //|
  void
    archive::read ( OBJECT* &obj )
  {
    CLASS *cls;
    read_class_ref ( cls );
    obj = new OBJECT ( cls );
    for ( int i = 0; i < obj->klass->instance_size; i++ )
      read ( obj->members [ i ] );
  }

  void
    archive::write ( const OBJECT * obj )
  {
    write_class_ref ( obj->klass );
    for ( int i = 0; i < obj->klass->instance_size; i++ )
      write ( obj->members [ i ] );
  }

  //|
  //| VALUE
  //|
  void
    archive::read ( VALUE &val )
  {
    read ( (char *) &val.v_type, 1 );
    switch ( val.v_type )
    {
    case DT_NULL:
      break;
    case DT_FLOAT:
      read ( val.v.v_float );
      break;
    case DT_INTEGER:
      read ( val.v.v_integer );
      break;
    case DT_STRING:
      read ( val.v.v_string );
      break;
    case DT_CLASS:
      read ( val.v.v_class );
      break;
    case DT_OBJECT:
      read ( val.v.v_object );
      break;
    case DT_ARRAY:
      read ( val.v.v_vector );
      break;
    case DT_CODE:
      read ( val.v.v_code );
      break;
    case DT_VAR:
      read ( val.v.v_var );
      break;
    case DT_EXT:
      {
        CLASS *cls = 0;
        read_class_ref ( cls );
        val.v.v_thing = cls->load ( this );
        if(!val.v.v_thing) val.v_type = DT_NULL;
        //assert ( val.v.v_thing != 0 );
      }
      break;
    case DT_SYMBOL:
      read ( val.v.v_symbol );
      break;
    case DT_OBJECT_METHOD:
      assert ( false );
      break;
    }
  }

  void  
    archive::write ( const VALUE &val )
  {
    write ( (char *) &val.v_type, 1 );
    switch ( val.v_type )
    {
    case DT_NULL:
      break;
    case DT_FLOAT:
      write ( val.v.v_float );
      break;
    case DT_INTEGER:
      write ( val.v.v_integer );
      break;
    case DT_STRING:
      write ( val.v.v_string );
      break;
    case DT_CLASS:
      write ( val.v.v_class );
      break;
    case DT_OBJECT:
      write ( val.v.v_object );
      break;
    case DT_ARRAY:
      write ( val.v.v_vector );
      break;
    case DT_CODE:
      write ( val.v.v_code );
      break;
    case DT_VAR:
      write ( val.v.v_var );
      break;
    case DT_EXT:
      {
        CLASS *cls = val.v.v_thing->get_class ();
        write_class_ref ( cls );
        cls->save ( val.v.v_thing, this );
      }
      break;
    case DT_SYMBOL:
      write ( val.v.v_symbol );
      break;
    case DT_OBJECT_METHOD:
      assert ( false );
      break;
    }
  }


  //|
  //| literal
  //|
  void
    archive::read_literal ( VALUE &val )
  {
    read ( (char *) &val.v_type, 1 );
    switch ( val.v_type )
    {
    case DT_NULL:
      break;
    case DT_FLOAT:
      read ( val.v.v_float );
      break;
    case DT_INTEGER:
      read ( val.v.v_integer );
      break;
    case DT_STRING:
      read ( val.v.v_string );
      break;
    case DT_CLASS:
      read_class_ref ( val.v.v_class );
      break;
    case DT_OBJECT:
      read ( val.v.v_object );
      break;
    case DT_ARRAY:
      read ( val.v.v_vector );
      break;
    case DT_CODE:
      read_code_ref ( val.v.v_code );
      break;
    case DT_VAR:
      read ( val.v.v_var );
      break;
    case DT_EXT:
      {
        CLASS *cls = 0;
        read_class_ref ( cls );
        val.v.v_thing = cls->load ( this );
        assert ( val.v.v_thing != 0 );
      }
      break;
    case DT_SYMBOL:
      read ( val.v.v_symbol );
      break;
    case DT_OBJECT_METHOD:
      assert ( false );
      break;
    }
  }

  extern char *nameoftype ( int );

  void
    archive::write_literal ( const VALUE &val )
  {
    write ( (char *) &val.v_type, 1 );
    switch ( val.v_type )
    {
    case DT_NULL:
      break;
    case DT_FLOAT:
      write ( val.v.v_float );
      break;
    case DT_INTEGER:
      write ( val.v.v_integer );
      break;
    case DT_STRING:
      write ( val.v.v_string );
      break;
    case DT_CLASS:
      write_class_ref ( val.v.v_class );
      break;
    case DT_OBJECT:
      write ( val.v.v_object );
      break;
    case DT_ARRAY:
      write ( val.v.v_vector );
      break;
    case DT_CODE:
      write_code_ref ( val.v.v_code );
      break;
    case DT_VAR:
      write ( val.v.v_var );
      break;
    case DT_EXT:
      {
        CLASS *cls = val.v.v_thing->get_class ();
        write_class_ref ( cls );
        bool r = cls->save ( val.v.v_thing, this );
        assert ( r );
      }
      break;
    case DT_SYMBOL:
      write ( val.v.v_symbol );
      break;
    case DT_OBJECT_METHOD:
      assert ( false );
      break;
    }

  }

  const char *magic = "c-smile";
  const size_t magic_length = 7;
  const int version = 0x100000;
  enum  arch_type
  {
    vm_package = 1,
    vm_data = 2,
    vm_package_bundle = 3
  };

  void  
    archive::save ( io_stream* ios, PACKAGE *pkg )
  {
    int i = 0;
    int size = 0;
    char  buffer [ 1024 ];

    in_memory_stream ms;
    _body = &ms;
    _symbols.clear ();

    if ( !ios->write ( magic, magic_length ) )
      goto problem_write;
    if ( !ios->write ( version ) )
      goto problem_write;

    if ( !ios->write ( (int) vm_package ) )
      goto problem_write;

    write ( pkg );

    if ( !ios->write ( _symbols.size () ) )
      goto problem_write;
    for ( i = 0; i < _symbols.size(); i++ )
      if ( !ios->write ( (const char *) VM::voc [ _symbols [ i ] ] ) )
        goto problem_write;

    _body->position ( 0 );
    for ( size = _body->size (); size > 0; size -= 1024 )
    {
      int length = min ( size, 1024 );
      if ( !_body->read ( buffer, length ) )
        goto problem_read;
      if ( !ios->write ( buffer, length ) )
        goto problem_write;
    }
    return;

  problem_write:
    // raise error here
    VM::error ( ios->err_message );
    return;
  problem_read:
    // raise error here
    VM::error ( _body->err_message );
    return;
  }

  void
    archive::save ( io_stream* ios, symbol_t mainpackage )
  {
    int       i = 0;
    int       size = 0;
    symbol_t  std_sym = VM::voc [ "std" ];
    char  buffer [ 1024 ];

    array<PACKAGE *> bundle;
    int iter_cnt = 0;
    int mainpackage_idx = 0;

    in_memory_stream ms;
    _body = &ms;
    _symbols.clear ();

    hash_table<string, string> natives;

    if ( !ios->write ( magic, magic_length ) )
      goto problem_write;

    if ( !ios->write ( version ) )
      goto problem_write;

    if ( !ios->write ( (int) vm_package_bundle ) )
      goto problem_write;

    // make list of packages
    for ( i = 0; i < VM::packages->size (); i++ )
    {
      PACKAGE *pkg = (PACKAGE *) ( *VM::packages ) [ i ].value.v.v_class;
      if ( pkg->is_native () )
      {
        // register all external packages
        if ( pkg != VM::std )
        {
          string pname = VM::voc [ pkg->name ];
          natives [ pname ] = pname;
        }
      }
      else  // this is bytecode (soft) package
      {
        bundle.push ( pkg );
        VM::info ( "packaging %s\n", VM::voc [ pkg->name ] );
      }
    }

    // sort it
    for ( i=0; i < bundle.size () && iter_cnt < 1000; iter_cnt++ )
    {
      int      ref  = -1;
      PACKAGE *pack = bundle [ i ];
      for ( int j = i + 1; j < bundle.size(); j++ )
      {
        if ( pack->has_reference_to ( bundle [ j ]->name ) )
          ref = j;
      }
      if ( ref >= 0 )
      { // move
        PACKAGE *pkg = bundle [ i ]; bundle.remove ( i );
        bundle.insert ( ref, pkg );
      }
      else ++i;
    }
    if ( iter_cnt >= 1000 )
      VM::info ( "WARNING:package bundle not properly ordered (cyclic reference?)" );

    // write one by one soft packages into intermediate buffer;
    for ( i = 0; i < bundle.size () ;i++ )
    {
      write ( bundle [ i ] );
      if ( bundle [ i ]->name == mainpackage )
        mainpackage_idx = i;
    }

    // write one by one external native packages names
    if ( !ios->write ( (int) natives.size () ) )
      goto problem_write;

    for ( i = 0; i < natives.size(); i++ )
      if ( !ios->write ( (const char *) natives.elements() [ i ] ) )
        goto problem_write;

    // write soft packages
    if ( !ios->write ( (int) mainpackage_idx ) )
      goto problem_write;

    if ( !ios->write ( (int)bundle.size () ) )
      goto problem_write;

    if ( !ios->write ( _symbols.size () ) )
      goto problem_write;

    for ( i = 0; i < _symbols.size (); i++ )
      if  ( !ios->write ( (const char *) VM::voc [ _symbols [ i ] ] ) )
        goto problem_write;

    _body->position ( 0 );
    while ( true )
    {
      int length;
      _body->read ( buffer, 1024, &length );
      if ( length == 0 )
        break;
      if ( !ios->write ( buffer, length ) )
        goto problem_write;
    }
    return;

  problem_write:
    // raise error here
    VM::error ( ios->err_message );
  }


  VALUE
    archive::load ( io_stream* ios )
  {
    VALUE   val;
    int     ver;
    char    buffer [ magic_length ];
    int     syms = 0;
    string  s;
    int     type;
    int mainpackage = -1;
    int bundle_size = 0;

    if ( !ios->read ( buffer, magic_length ) )
      goto problem_read;

    if ( memcmp ( buffer, magic, magic_length ) != 0 )
      VM::error ( "bad archive signature" );

    if( !ios->read ( ver ) )
      goto problem_read;

    if ( ver > version )
      VM::error ( "unsupported archive version" );

    if ( !ios->read ( type ) )
      goto problem_read;

    if ( type == vm_package_bundle )
    {
      // read one by one external native packages names
      int natives_num = 0;

      if ( !ios->read ( natives_num ) )
        goto problem_read;

      for ( int i = 0; i < natives_num; i++ )
      {
        string native_name;
        if ( !ios->read ( native_name ) )
          goto problem_read;
        VM::load_native_module ( native_name );
      }

      if ( !ios->read ( mainpackage ) )
        goto problem_read;

      if ( !ios->read ( bundle_size ) )
        goto problem_read;
    }

    if ( !ios->read ( syms ) )
      goto problem_read;

    _symbols.clear ();

    for ( ; syms > 0; syms-- )
    {
      if ( !ios->read ( s ) )
        goto problem_read;

      _symbols.push ( VM::voc [ s ] );
    }

    _body = ios;

    switch ( type )
    {
    case vm_package:
      {
        PACKAGE *pkg;
        read ( pkg );
        val = pkg;
      }
      break;
    case vm_package_bundle:
      {
        PACKAGE *pkg;
        for ( int i = 0; i < bundle_size; i++ )
        {
          read ( pkg );
          if ( i == mainpackage )
            val = pkg;
        }
      }
      break;
    default:
      assert ( false );
      break;
    }

    return val;

  problem_read:
    VM::error ( ios->err_message );
    return val;
  }

};
