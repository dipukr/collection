/*
*
* cs_main.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
// main.cpp - TIL main routine

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "c-smile.h"
#include "vm.h"
#include "compiler.h"
#include "streams.h"

//|
//| Revision history:
//|
//| 17.04.2001 - initial implementation completed
//|
//|

bool  initialize_ext ();

namespace c_smile
{
  //|
  //| main - the main routine
  //|

  void
    usage ( const char *app_name )
  {
    printf ( "Usage:\n\t%s <flags> <program>\n", app_name );
    printf ( "\t\t = c\tCompilation mode. .csp -> .cse\n" );
    printf ( "\t\t = e\tCOmpile and produce standalone executable file\n" );
    printf ( "\t\t = f\tCompile and run if .csp\n" );
    printf ( "\t\t\tRun bytecode if .cse\n" );
    printf ( "\t\t = l\tLibrary path. ie. path1;path2;etc\n" );
#ifdef DECODE_TRACE
    printf ( "\t\t = d\tDecode mode\n" );
    printf ( "\t\t = t\tTrace mode\n" );
#endif
  }

  //|
  //| main - the main routine
  //|
  int
    cs_main ( int argc, char *argv [] )
  {
    // first of all
    VM::initialize ( argv [ 0 ] );

    initialize_ext ();

    string filename;
    string exefilename;

    VM * vm = VM::main ();

    // display the banner
    vm->sout->put ( BANNER );
    vm->sout->put ( "\n" );

    PACKAGE * package = 0;

    bool compile = false;
    bool make_exe = false;
    bool run = false;

    VALUE vargv [ 32 ];
    int   vargc = 0;

    int   r = 0;
    int   argi = 3;

    if ( argc >= 2 )
    {
      if ( strcmp ( argv [ 1 ], "=c" ) == 0 )
      {
        compile = true;
      }
      else if ( strcmp ( argv [ 1 ], "=e" ) == 0 )
      {
        compile = true;
        make_exe = true;
      }
      else if ( strcmp ( argv [ 1 ], "=f" ) == 0 )
      {
        run = true;
        compile = false;
        make_exe = false;
      }
      else argi = 1;

      if ( argi == 3  )
      {
        filename = argv [ 2 ];
        int idx = filename.last_index_of ( '/' );
        if ( idx < 0 ) idx = filename.last_index_of ( '\\' );
        if ( idx >= 0  )
        {
          if ( ( filename.index_of ( ':' ) >= 0 ) ||
               ( filename [ 0 ] == '/' ) ||
               ( filename [ 0 ] == '\\' ) )
            VM::start_path = filename.substr ( 0, idx );
          else
          {
            VM::start_path += VM::PATH_SEP;
            VM::start_path += filename.substr ( 0, idx );
          }
        }
        filename = VM::start_path + VM::PATH_SEP + filename.substr ( idx + 1 );
      }
    }

    // load and execute code
    for ( int i = argi; i < argc; ++i  )
    {
#ifdef DECODE_TRACE
      if ( strcmp ( argv [ i  ], "=d" ) == 0 )
      {
        vm->decodeMode ( true );
      }
      else if ( strcmp ( argv [ i ], "=t" ) == 0 )
      {
        vm->traceMode ( true );
      }
      else
#endif
      if ( strcmp ( argv [ i ], "=l" ) == 0 && i < argc - 1 )
      {
        string libpath = argv [ ++i ];
        VM::lib_paths = libpath.tokens ( ";" );
      }
      else
      {
        if ( vargc < 32 )
          vargv [ vargc++ ] = new STRING ( argv [ i ] );
      //TODO: WARNING here
      }
    }

    try
    {
      package = VM::load_attachment ();
      if ( package )
      {
        vm->execute ( package, "main", vargc, vargv );
      }
      else if ( ! filename.is_empty () )
      {
        package = ( compile || ( filename.match ( "*.csp" ) >= 0 ) ) ?
                  VM::compile_file ( filename ) :
                  VM::load_file ( filename );
        if ( package )
        {
          if ( !compile )
            vm->execute ( package, "main", vargc, vargv );
          else
          {
            printf ( "\n%s - done!\n", (const char *) filename );
            VM::make_bundle ( package, filename, make_exe );
          }
        }
        else
          printf ( "\n%s - problems!\n", (const char *) filename );
      }
      else
        usage ( (const char *) argv [ 0 ] );
    }
    catch ( VM_RTE rte )
    {
      printf ( "\nERROR:%s\n", (const char *) rte.report () );
      r = -1;
    }

    VM::terminate ();

    return r;
  }

};
