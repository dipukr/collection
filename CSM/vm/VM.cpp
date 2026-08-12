/*
*
* VM.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov - kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

#include <stdarg.h>
#include <locale.h>

#include "c-smile.h"
#include "arithmetic.h"
#include "vm.h"
#include "tool.h"
#include "compiler.h"
#include "streams.h"
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#include <pthread.h>
#define _stat		stat
#define _vsnprintf	vsnprintf
#define MAX_PATH ( PATH_MAX + 1 ) 
#endif

extern bool load_ni_extension ( const char *dl_path );

namespace c_smile
{

#ifdef _WIN32
  const char * VM::PATH_SEP = "\\";
  const char * VM::dll_ext = "dll";
#else
  const char * VM::PATH_SEP = "/";
  const char * VM::dll_ext = "so";
#endif

  const char * CSMILE_LIB_PATH = "CSMILE_LIB_PATH";

  sym_table        VM::voc;      // symbols vocabulary

  PACKAGE *        VM::std = 0;          // standard package
  DICTIONARY *     VM::packages = 0;     // loaded packages
  CLASS *          VM::class_string = 0;
  CLASS *          VM::class_array = 0;

  console_stream * VM::sin = 0;
  console_stream * VM::sout = 0;
  console_stream * VM::serr = 0;

  VALUE            VM::null;		  // the null value
  VALUE            VM::undefined; // the undefined value

  list<VM*>        VM::all;       // VM pool
  sal::mutex       VM::all_guard; // VM pool guard

#ifdef _WIN32
  unsigned int     VM::thread_slot_id = 0; // for TlsGetValue/TlsSetValue
#else
  pthread_key_t    VM::thread_slot_id = 0;
#endif

  string           VM::app_path;
  string           VM::start_path;

  array<string>    VM::lib_paths;


  void
    VM::mark_std_classes () 
  {
    mark_thing ( class_string );
    mark_thing ( class_array );
  }

  //|
  //| format here is a string consisting of 'i' 'd' 'o' 's' 'a' 'v'
  //| where i-integer f-float o-object s-string a-any //v-vector
  //|
  //| sample:
  //|
  //| int i = 0;
  //| double d = 0.0;
  //| string s;
  //| VALUE a;
  //|
  //| vm->arguments ( "idsa", &i, &d, &s, &a ) 
  //|

  int
    VM::arguments ( int argc,  VALUE *argv,  const char *format,  ... ) 
  {
    const char *p = format;

    va_list marker;
    va_start ( marker,  format );

    VALUE *v; int n = 0;

    for ( int i = 0; i < argc; ++i ) 
    {
      v = &argv [ i ];
      void *arg = va_arg ( marker, void * );

      switch ( *p++ ) 
      {
      case 'i':
        checktype ( *v, DT_INTEGER, n );
        *( (int *) arg ) = int ( *v );
        break;
      case 'd':
        checktype ( *v, DT_FLOAT, n );
        *( (double *) arg ) = double ( *v );
        break;
      case 'S':
        checktype ( *v, DT_STRING, n );
        *( (string *) arg ) = CSTR ( v->v.v_string );
        break;
      case 's':
        checktype ( *v, DT_STRING, n );
        *( (STRING **) arg ) = v->v.v_string;
        break;
      case 'a':
        *( (VALUE *) arg ) = *v;
        break;
      case 'c':
        checktype ( *v, DT_CLASS, n );
        *( (CLASS **) arg ) = v->v.v_class;
        break;
      case 'v':
        checktype ( *v, DT_ARRAY, n );
        *( (ARRAY **) arg ) = v->v.v_vector;
        break;
      default:
      case '\0':
        goto enough;
      }
      n++;
    }
  enough:
    va_end ( marker );
    return n;
  }

  CLASS *
    VM::find_class ( symbol_t package_name, symbol_t class_name ) 
  {
    PACKAGE *pkg = find_package ( package_name );

    ENTRY e = pkg->find ( class_name );
    if ( e.is_valid () ) 
    {
      if ( ( e.type () != ST_SDATA ) || ( e.value () ->v_type != DT_CLASS ) ) 
        error ( "'%s' is not a class in '%s'", class_name, package_name );
      return e.value()->v.v_class;
    }
    return 0;
  }

  PACKAGE *
    VM::find_package ( symbol_t package_name, const char *dir ) 
  {
    int idx = packages->find ( package_name );
    if ( idx >= 0 ) 
      return (PACKAGE *) ( *packages ) [ idx ] .value.v.v_class;
    else
      if ( dir == 0 )
        VM::error ( "package '%s' not found\n",  VM::voc [ package_name ] );
    else
    {
      string path ( dir );
      string pname = VM::voc [ package_name ];
      string name_ext = pname;
      name_ext += ".csp";

      string name_ext_dll = VM::voc [ package_name ];
      name_ext_dll += VM::dll_ext;

      string filename; bool found = false;
      int i = 0;
      while ( true ) 
      {
        filename = path + PATH_SEP + name_ext;
        struct _stat buf;
        int result = _stat ( filename,  &buf );
        if ( ( result == 0 ) && ( ( buf.st_mode & S_IFDIR ) == 0 ) && buf.st_size ) 
        {
          found = true;
          break;
        }
        if ( i >= lib_paths.size () )  break;
        path = lib_paths [ i++ ];
      }
      if ( found )
        return VM::compile_file ( filename );
      else
      {
        if ( VM::load_native_module ( pname ) ) 
          return 0;
      }
      VM::error ( "attempt to find '%s' package files ( %s or %s ) failed\n",
                  (const char *) pname, (const char *) name_ext,
                  (const char *) name_ext_dll );
    }
    return 0; // to make it happy
  }

  //|
  //| load_file - compile source or load bytecode
  //|
  PACKAGE *
    VM::load_file ( const char *name ) 
  {
    PACKAGE *package = 0;
    string in = name;
    file_stream *ifp = new file_stream ();
    ifp->name ( in );
    if ( ifp->open ( "rb" ) ) 
    {
      {
        archive ar;
        VALUE v = ar.load ( ifp );
        assert ( v.v_type == DT_CLASS );
        package = (PACKAGE *) v.v.v_class;
      }
      ifp->close ();
    }
    else
      VM::error ( "'%s' could not be opened", (const char *) name );

    return package;
  }

  //|
  //| compile_file - compile source or load bytecode
  //|
  PACKAGE *
    VM::compile_file ( const char *name ) 
  {
    PACKAGE *package = 0;

#ifdef COMPILER
    string in = name;
    file_stream *ifp = new file_stream ();
    ifp->name ( in );
    if ( ! ifp->open ( "rb" ) ) 
      VM::error ( "'%s' could not be opened", (const char *) in );

    VM *vm = VM::current ();
    compiler *comp = new compiler ();
    try
    {
      package = comp->compile ( ifp );
      delete comp;
    }
    catch ( parse_error& pe ) 
    {
      VM::serr->put ( (const char *) pe.full_report () );
      delete comp;
      return 0;
    }
    if ( package != 0 ) 
    {
      bool b = add_package ( package );
      if ( b ) b = run_init_code ( package );
      assert ( b );
    }
    ifp->close ();

#endif
    return package;
  }

  //|
  //| make_bundle
  //|
  void
    VM::make_bundle ( const PACKAGE *mainp, const char *name, bool make_exe ) 
  {
    string in = name;
    int period = in.last_index_of ( '.' );
#ifdef _WIN32
    string out = in.substr ( 0,  period + 1 ) + (const char *) ( make_exe? "exe" : "cse" );
#else
    string out = in.substr ( 0,  period ) + (const char *) ( make_exe? "" : ".cse" );
#endif

    file_stream outf;
    outf.name ( out );
    if ( !outf.open ( "wb" ) ) 
      VM::error ( "'%s' could not be created", ( const char * ) out );
    else
    {
      archive ar;
      if ( make_exe ) 
      {
        file_stream   exe;
        string        exe_file_name;
#ifdef _WIN32
        {
          char buffer [ MAX_PATH ];
          buffer [ GetModuleFileName ( 
          NULL,        // handle to module
          buffer,      // file name of module
          MAX_PATH ) ] = 0;  // size of buffer
          exe_file_name = buffer;
        }
#else
        exe_file_name = VM::app_path;
#endif
        exe.name ( exe_file_name );
        if ( !exe.open ( "rb" ) )
          error ( exe.err_message );
        char buffer [ 8192 ];
        int read;
        while ( true ) 
        {
          exe.read ( buffer, 8192, &read );
          if ( read == 0 )
            break;
          outf.write ( buffer, read );
        }
        ar.save ( &outf, mainp->name );

        int original_size = ( int ) exe.size ();
        int signature = 0xAFAFAFAF;

        outf.write ( ( char * ) &original_size,  sizeof ( int ) );
        outf.write ( ( char * ) &signature,  sizeof ( int ) );
      }
      else
        ar.save ( &outf, mainp->name );
    }
    outf.close ();
#ifndef _WIN32
    if ( make_exe ) 
      chmod ( ( const char * ) out,  S_IXUSR | S_IXGRP | S_IXOTH );
#endif
  }

  bool
    VM::add_package ( PACKAGE * package ) 
  {
    int idx = packages->find ( package->name );
    if ( idx >= 0 ) return false;
    idx = packages->size ();
    packages = packages->realloc ( idx + 1 );
    ( *packages ) [ idx ].value = (CLASS *) package;
    ( *packages ) [ idx ].symbol = package->name;

    return true;
  }

  bool 
    VM::run_init_code ( PACKAGE * package ) 
  {
      if(VM::current ())
        return VM::current () -> execute_init ( package ); 
      return false;
  }


  //|
  //| info - display progress information
  //|
  void
    VM::info ( char *fmt,  ... ) 
  {
    char buf1 [ 100 ];
    va_list args;
    va_start ( args,  fmt );
    _vsnprintf ( buf1, 100, fmt,  args );
    va_end ( args );
    sout->put ( buf1 );
  }


#ifdef _WIN32
  BOOL WINAPI
    ControlHandler ( DWORD dwCtrlType ) //  control signal type
  {
    VM *main = VM::main ();
    if ( main && main->queue.is_waiting () ) 
    {
      main->queue.send ( VALUE ( int ( dwCtrlType ) ) );
      return TRUE;
    }
    return FALSE;
  }
#endif

  bool
    VM::initialize ( const char *appname ) 
  {

    // init null value
    null.v.v_integer = -1;

    setlocale ( LC_ALL,  ".ACP" );
    // Sets the locale to the ANSI code page obtained from the operating system.
    char startpath [ MAX_PATH ];
    start_path = getcwd ( startpath, MAX_PATH );
    app_path = appname;

#ifdef _WIN32
    BOOL rr = SetConsoleCtrlHandler ( 
    ControlHandler,   // handler function
    TRUE             // add or remove handler
     );
#endif

#ifdef _WIN32
    thread_slot_id = TlsAlloc ();
    GetModuleFileName ( NULL, startpath, MAX_PATH );
    app_path = startpath;

#else
    pthread_key_create ( &thread_slot_id,  NULL );
#endif

    // allocate streams
    sin = new console_stream ( console_stream::STDIN );
    sout = new console_stream ( console_stream::STDOUT );
    serr = new console_stream ( console_stream::STDERR );

    // this has to match
    (void ) voc [ "toString" ];
    (void ) voc [ "valueOf" ];
    (void ) voc [ "[]" ];
    (void ) voc [ "cast" ];
    (void ) voc [ "run" ];
    (void ) voc [ "finalize" ];

    // create packages dict and std package
    packages = DICTIONARY::create ();

    // create std package
    init_std_package ();
    ( new VM ( SMAX ) )->attach_thread ();

    return true;
  }

  bool
    VM::terminate () 
  {
#ifdef _WIN32
    SetConsoleCtrlHandler ( 
    ControlHandler,   // handler function
    FALSE            // add or remove handler
     );
#endif

    // stop all threads
    // ....

    // close streams
    delete sin;
    delete sout;
    delete serr;

    return true;

  }


  VM *
    VM::current () 
  {
#ifdef _WIN32
    return ( VM* ) TlsGetValue ( thread_slot_id );
#else
    return ( VM* ) pthread_getspecific ( thread_slot_id );
#endif
  }

#ifdef _WIN32
  DWORD WINAPI
    thread_func ( void* arg ) 
#else
  void *
    thread_func ( void* arg ) 
#endif
  {
    VM* vm = ( VM* ) arg;
    vm->attach_thread ();

    try
    {
      vm->execute_call ();
    }
    catch ( VM_RTE& er ) 
    {
      VM::serr->put ( er.report () );
    }

    return 0;
  }

#define THREAD_MIN_STACK_SIZE 16*1024

  void
    VM::attach_thread () 
  {
    //|
    //| main app thread. just setup it
    //|
#ifdef _WIN32
    TlsSetValue ( thread_slot_id,  this );
#else
    pthread_setspecific ( thread_slot_id,  this );
#endif
  }

  void
    VM::create_thread () 
  {
    //|
    //| other threads
    //|
#ifdef _WIN32
    HANDLE h = CreateThread ( 
                              NULL,
                              THREAD_MIN_STACK_SIZE,
                              thread_func,
                              this,
                              0,
                              &thread_id );
    CloseHandle ( h );
#else

    pthread_t thr;
    pthread_attr_t attr;

    pthread_attr_init ( &attr );
    pthread_attr_setstacksize ( &attr,  THREAD_MIN_STACK_SIZE );
    pthread_create ( &thr,  &attr,  thread_func,  this );
    pthread_detach ( thr );
    pthread_attr_destroy ( &attr );

#endif
  }

  void
    VM::make_exe ( PACKAGE *pkg,  const char *filename )
  {
  }

  PACKAGE *
    VM::load_attachment () 
  {
    string exe_name = VM::app_path;

    file_stream exefile;
    exefile.name ( exe_name );
    if ( ! exefile.open ( "rb" ) )
      return 0;

    size_t sz = exefile.size ();

    exefile.position ( sz - 2 * sizeof ( int ) );

    int original_size = 0;
    int signature = 0;

    if ( !exefile.read ( ( char * ) &original_size, sizeof ( int ) ) )
      return 0;
    if ( !exefile.read ( ( char * ) &signature, sizeof ( int ) ) )
      return 0;

    if ( signature != ( int ) 0xAFAFAFAF )
      return 0;
    if ( original_size < 0 || original_size >= ( int ) sz )
      return 0;


    exefile.position ( original_size );

    VALUE v;
    {
      archive ar;
      v = ar.load ( &exefile );
    }
    if ( v.v_type != DT_CLASS )
      return 0;
    return ( PACKAGE * ) v.v.v_class;
  }

  bool
    VM::load_native_module ( const char *module_name ) 
  {
    // current / start_path
    string file_name = string::format ( "%s%s%s.%s", 
    (const char * ) start_path, 
    (const char * ) PATH_SEP, 
    (const char * ) module_name, 
    (const char * ) dll_ext );

    if ( load_ni_extension ( file_name ) )  return true;

    char *env = getenv ( CSMILE_LIB_PATH );
    if ( env ) 
    {
      array<string> items = string ( env ).tokens ( ";," );
      for ( int i = 0; i < items.size (); i++ ) 
      {
        file_name = string::format ( "%s%s%s.%s", 
                                     (const char * ) items [ i ], 
                                     (const char * ) PATH_SEP, 
                                     (const char * ) module_name, 
                                     (const char * ) dll_ext );
        if ( load_ni_extension ( file_name ) )
          return true;
      }
    }
    return false;
  }

};
