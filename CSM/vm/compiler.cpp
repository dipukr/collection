/*
*
* compiler.cpp
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

#include <setjmp.h>
#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "c-smile.h"
#include "compiler.h"
#include "streams.h"

#ifdef _WIN32
#define snprintf  _snprintf
#endif

namespace c_smile
{
#ifdef COMPILER
  // variable access function codes
#define LOAD	1
#define STORE	2
#define PUSH	3
#define DUP	  4

  compiler::compiler () : temporaries ( 0 ), methodclass ( 0 ), report_stream ( 0 )
  {
  }

  // mark_compiler - mark compiler variables
  void
    compiler::mark ()
  {
    mark_thing ( methodclass );
    mark_thing ( package );
    temp.mark ();
  }

  void
    compiler::report ( const char * format, ... )
  {
    if ( report_stream == 0 )
      return;
  }

  // compile_definitions - compile klass or function definitions
  PACKAGE *
    compiler::compile ( io_stream *input )
  {
    file_name = input->name ();

    int idx = max ( file_name.last_index_of ( '/' ),
                    file_name.last_index_of ( '\\' ) );
    assert ( idx > 0 );
    dir_name = file_name.substr ( 0, idx );

    int tkn;

    package = new PACKAGE ( 0, (const char *) file_name );

    unsigned char stack_buff [ CMAX ];
    cbuff = stack_buff;
    cptr = 0;

    inConstructor = false;

    bool package_reported = false;

    putcbyte ( OP_PUSH );

    // trap errors
    try
    {
      // initialize
      scan.init ( this, input );
      bsp = &bstack [ -1 ];
      csp = &cstack [ -1 ];


      // process statements until end of file
      while ( (tkn = scan.token () ) != T_EOF )
      {
        switch ( tkn )
        {
        case T_PACKAGE:
          do_package ();
          if ( VM::packages->find ( package->name ) >= 0 )
          {
            delete package;
            throw parse_error ( this, "Package '%s' already defined.",
                                VM::voc [ package->name ] );
          }
          break;
        case T_IMPORT:
          if ( package->name == 0 )
            throw parse_error ( this, "Expecting package declaration" );
          do_import ();
          break;
        default:
          scan.stoken ( tkn );
          goto stage1;
        }
      }
    stage1:
      report ( "package '%s'\n", VM::voc [ package->name ] );
      VM::info ( "compiling '%s' ... ", VM::voc [ package->name ] );

      // process statements until end of file
      while ( ( tkn = scan.token () ) != T_EOF )
      {
        switch ( tkn )
        {
        case T_STATIC:
          tkn = scan.token ();
          if ( tkn == T_VAR )
          {
            do_static_vardecl ( (CLASS *) package, false );
            break;
          }
          else if ( tkn != T_FUNCTION )
            throw parse_error ( this, "Expecting 'var' | 'function'" );
          // otherwise function
        case T_FUNCTION:
          do_function ();
          break;

        case T_CONST:
          do_static_vardecl ( (CLASS *) package, true );
          break;

        case T_VAR:
          do_static_vardecl ( (CLASS *) package, false );
          break;

        case T_CLASS:
          do_class ();
          break;
        default:
          throw parse_error ( this,
                              "Expecting function | class | var declaration" );
          break;
        }

      }
      putcbyte ( OP_RETURN );

      BUFFER *bc = new BUFFER ( cptr );
      // build the package init code
      package->init_code = new CODE ( VM::voc [ "<init>" ],
                                      bc, ( CLASS*) package );
      // create the code string
      unsigned char *src = cbuff, *dst = &( *bc ) [ 0 ];
      while ( --cptr >= 0 )
        *dst++ = *src++;

#ifdef DECODE_TRACE
      // show the generated code
      if ( VM::decode )
        VM::decode_procedure ( package->init_code );
#endif

      report ( "package '%s' done", VM::voc [ package->name ] );
      VM::info ( "done.\n" );

    }
    catch ( parse_error& pe )
    {
      while ( temporaries )
      {
        name_space *t = temporaries;
        temporaries = temporaries->parent;
      }
      delete package;
      package = 0;
      throw pe;
    }
    return package;
  }

  void
    compiler::do_package ()
  {
    // get the package name
    frequire ( T_IDENTIFIER );

    if ( package->name != undefined_symbol  )
      throw parse_error ( this, "package already defined" );

    package->name = VM::voc [ scan.t_token ];

    frequire ( ';' );
  }

  void
    compiler::do_import ()
  {
    // get the package name
    while ( true )
    {
      frequire ( T_IDENTIFIER );
      string package_name = scan.t_token;

      PACKAGE *ipackage =
              VM::find_package ( VM::voc [ (const char *) package_name ],
                                 dir_name );

      int tkn = scan.token ();
      if ( tkn == ';' )
        break;
      if ( tkn != ',' )
        throw parse_error ( this, "Expecting ',' | ';'" );
    }
  }

  // do_class - handle class declarations
  void
    compiler::do_class ()
  {
    string cname;
    int type, tkn;

    // get the klass name
    frequire ( T_IDENTIFIER );
    cname = scan.t_token;

    VALUE tv;
    CLASS *cl = 0;

    // get the optional base klass
    if ( ( tkn = scan.token () ) == ':' )
    {
      frequire ( T_IDENTIFIER );
      string    cls_name = scan.t_token;
      PACKAGE * pkg = 0;
      if ( ( tkn = scan.token () ) == T_CC )
      {
        pkg = get_package ( cls_name, true );
        frequire ( T_IDENTIFIER );
        cls_name = scan.t_token;
      }
      else
        scan.stoken ( tkn );

      tv = get_class ( cls_name, pkg );
      cl = tv.v.v_class;
      report ( "\tclass '%s' base class '%s'",(const char *) cname,
                                              VM::voc [ cl->name ] );
    }
    else
    {
      scan.stoken ( tkn );
      report ( "\tclass '%s'", (const char *) cname );
    }
    frequire ( '{' );

    // create the new class object
    tv = cl = new CLASS ( cname, cl, package );

    cl->instance_size = ( cl->base ) ? cl->base->instance_size : 0;

    // handle each variable declaration
    while ( ( tkn = scan.token  () ) != '}' )
    {
      // check for members
      int var = 0;
      type = 0;

      if ( tkn == T_STATIC )
      {
        type = T_STATIC;
        var = T_VAR;
        tkn = scan.token ();
      }

      if ( tkn == T_VAR )
      {
        var = T_VAR;
      }
      else if ( tkn == T_CONST )
      {
        type = T_STATIC;
        var = T_CONST;
      }
      else if ( tkn == T_FUNCTION )
      {
        var = T_FUNCTION;
      }
      else if ( tkn == T_PROPERTY )
      {
        var = T_PROPERTY;
      }

      if ( var == 0 )
        throw parse_error ( this,
          "Expecting 'static' | 'const' | ['var' | 'function' | 'property'  ]" );

      // check for a member function declaration
      if ( var == T_FUNCTION )
        do_member_function ( cl, type );
      // check for a member function declaration
      else if ( var == T_PROPERTY )
        do_property_function ( cl, type );
      // handle data members
      else if ( var == T_VAR )
      {
        if ( type == T_STATIC )
          do_static_vardecl ( cl, false );
        else
          do_member_data ( cl, type );
      }
      else if ( var == T_CONST )
      {
        do_static_vardecl ( cl, true );
      }
    }
  }

  // do_member_data - parse a member data declarations
  // token - clause token
  void
    compiler::do_member_data ( CLASS *klass, int token )
  {
    int tkn;
    do
    {
      frequire ( T_IDENTIFIER );
      ENTRY en = klass->add ( scan.t_token, ST_DATA );
      if ( !en.is_valid () )
        throw parse_error ( this, "Variable '%s' already defined.",
                            (const char *) scan.t_token );

      VALUE *v = en.value ();
      *v = klass->instance_size++;
    }
    while ( ( tkn = scan.token () ) == ',' );
    scan.stoken ( tkn );
    frequire ( ';' );
  }

  // do_member_function - parse a member function definition
  void
    compiler::do_member_function ( CLASS *klass, int type )
  {
    int tkn = scan.token ();
    string selector;

    if ( tkn == '[' )
    {
      if ( ( tkn = scan.token () ) != ']' )
        throw parse_error ( this, "Expecting ]" );
      selector = "[]";
    }
    else if ( tkn == T_IDENTIFIER )
    {
      selector = scan.t_token;
    }
    else
      throw parse_error ( this, "Expecting identifier | [] | delete" );

    const char * class_name = VM::voc [ klass->name ];

    report ( "\t\tfunction '%s::%s'", class_name, (const char *) selector );

    tkn = scan.token ();
    if ( tkn != '(' )
      throw parse_error ( this, "Expecting '('" );

    int stype = type == T_STATIC ? ST_SFUNCTION : ST_FUNCTION;

    bool musthavebody = false;

    ENTRY e = klass->find ( selector );
    if ( e.is_valid () )
    {
      if ( e.value()->is_null() )
        throw parse_error ( this, "'%s::%s' already defined.", class_name,
                            (const char *) selector );
      if ( e.type () != stype )
        throw parse_error ( this,
                           "'%s::%s' previously defined having different type.",
                           class_name, (const char *) selector );
      musthavebody = true;
    }
    else
    {
      e = klass->add ( selector, stype );
    }

    if ( stype == ST_FUNCTION )
      inConstructor = ( klass->name == VM::voc [ selector ] );

    VALUE *v = e.value ();
    *v = do_code ( selector, klass, stype );

    inConstructor = false;
  }

  // do_property_function - parse a member property function definition
  void
    compiler::do_property_function ( CLASS *klass, int type )
  {
    int tkn = scan.token ();
    if ( tkn != T_IDENTIFIER )
      throw parse_error ( this, "Expecting identifier" );

    string selector = scan.t_token;
    const char * class_name = VM::voc [ klass->name ];

    report ( "\t\tproperty '%s::%s'", class_name, ( const char * ) selector );

    tkn = scan.token ();
    if ( tkn != '(' )
      throw parse_error ( this, "Expecting '('" );

    int stype = type == T_STATIC ? ST_SPROPERTY : ST_PROPERTY;
    ENTRY e = klass->add ( selector, stype );
    if ( !e.is_valid () )
      throw parse_error ( this, "'%s::%s' already defined.", class_name,
                          (const char *) selector );

    VALUE *v = e.value ();
    *v = do_code ( selector, klass, stype );

  }

  // findmember - find a klass member
  ENTRY
    compiler::findmember ( CLASS *klass, const char *name )
  {
    return klass->find ( name );
  }

  // rfindmember - recursive findmember
  ENTRY
    compiler::rfindmember ( CLASS *klass, const char *name )
  {
    ENTRY e;
    while ( klass )
    {
      e = klass->find ( name );
      if ( e.is_valid () )
        return e;
      klass = klass->base;
    }
    return e;
  }

  // do_function - handle function declarations
  void
    compiler::do_function ()
  {
    frequire ( T_IDENTIFIER );
    string name = scan.t_token;

    int tkn = scan.token ();
    if ( tkn == '(' )
      do_regular_function ( name );
    else if ( tkn == T_CC )
    {
      string class_name = name;
      ENTRY e = package->find ( class_name );
      if ( !e.is_valid () )
        throw parse_error ( this, "Class '%s' not found.",
                            (const char *) class_name );
      else if ( e.value()->v_type != DT_CLASS )
        throw parse_error ( this, "'%s' is not a class.",
                            (const char *) class_name );
      CLASS *klass = e.value()->v.v_class;
      frequire ( T_IDENTIFIER );
      string name = scan.t_token;
      e = klass->find ( name );
      if ( !e.is_valid () ||
           !( ( e.type ()  == ST_FUNCTION ) ||
              ( e.type ()  == ST_SFUNCTION )
            )
         )
        throw parse_error ( this,
                           "'%s' has to be declared as function in '%s' class.",
                           (const char *) name, (const char *) class_name );

      frequire ( '(' );

      *( e.value () ) = do_code ( name, klass, e.type () );
    }
    else
      throw parse_error ( this, "Expecting '(' | '::'" );
  }

  // do_regular_function - parse a regular function definition
  void
    compiler::do_regular_function ( const char *name )
  {
    report ( "\tfunction '%s'", name );

    VALUE *v = package->add ( name, ST_SFUNCTION ).value ();
    if ( !v )
      throw parse_error ( this, "Function '%s' already defined.", name );

    *v = do_code ( name, package, ST_SFUNCTION );
  }

  // do_code - compile the code part of a function or method
  CODE *
    compiler::do_code ( const char *name, CLASS *klass, int stype )
  {

    unsigned char *old_cbuff = cbuff;	 	    // code buffer
    int            old_cptr  = cptr;	 	 		// code pointer

    int tcnt = 1;

    unsigned char stack_cbuff [ CMAX ];

    // add the implicit 'this' argument for member functions
    if ( klass )
    {
      (void) arguments [ "this" ];
      methodclass = klass;
    }
    else
      methodclass = 0;

    // get the argument arglist
    get_id_list ( arguments, ")" );

    frequire ( ')' );

    int tkn = scan.token ();
    if ( tkn == ';' )
    {
      arguments.clear ();
      return 0;
    }
    else if ( tkn != '{' )
      throw parse_error ( this, "Expecting '{' | ';'" );

    // initialize
    cbuff = stack_cbuff;
    cptr = 0;

    // reserve space for the temporaries
    putcbyte ( OP_TSPACE );
    int c_tspace = cptr;
    putcbyte ( tcnt );

    // compile the code
    putcbyte ( OP_PUSH );
    int last_statement = do_block ();
    if ( last_statement != T_RETURN )
    {
      if ( inConstructor )
      {
        compiler::PVAL pv;
        findvariable ( "this", &pv );
        rvalue ( &pv );
      }
      putcbyte ( OP_RETURN );
    }

    // update OP_TSPACE operand
    tcnt = temporaries->get_total ();
    cbuff [ c_tspace ] = tcnt;

    delete temporaries; // unwind all stack please;
    temporaries = 0;

    BUFFER *bc = new BUFFER ( cptr );

    // build the function
    CODE *bytecode = new CODE ( VM::voc [ name ], bc, klass );

    // create the code string
    unsigned char *src = cbuff, *dst = &( *bc ) [ 0 ];
    while ( --cptr >= 0 )
      *dst++ = *src++;

#ifdef DECODE_TRACE
    // show the generated code
    VM::decode_procedure ( bytecode );
#endif
    //restore
    cbuff = old_cbuff;  // code buffer
    cptr  = old_cptr;   // code pointer

    arguments.clear ();

    if ( klass )
      klass->check_name ( VM::voc [ name ], bytecode, stype );
    // return the code object
    return bytecode;
  }

  // do_init_code - compile the code part of a package
  void
    compiler::do_static_vardecl ( CLASS *klass_or_package, bool constant )
  {
    int tkn;
    string id;
    compiler::PVAL rhs;
    for ( ; ; )
    {
      tkn = scan.token ();
      if ( tkn != T_IDENTIFIER )
        throw parse_error ( this, "Expecting an identifier" );
      id = scan.t_token;

      //code_variable
      ENTRY entry = klass_or_package->add ( id, constant ? ST_CONST
                                                         : ST_SDATA );

      if ( !entry.is_valid () )
      {
        throw parse_error ( this, "Variable or constant '%s' already defined",
                            (const char *) id );
      }

      int lit_num = package->add_literal ( VALUE ( entry ) );

      tkn = scan.token ();
      if ( tkn == '=' )
      {
        code_variable ( this, PUSH, 0 );
        do_expr2 ( &rhs );
        rvalue ( &rhs );
        code_variable ( this, STORE, lit_num );
        tkn = scan.token ();
      }
      else
      {
        putcbyte ( OP_UNDEFINED );
        code_variable ( this, STORE, lit_num );
      }

      if ( tkn == ',' )
        continue;
      else if ( tkn == ';' )
      {
        break;
      }
      else
        throw parse_error ( this, "Expecting ',' | ';'" );
    }
  }


  // get_class - get the klass associated with a symbol
  CLASS *
    compiler::get_class ( const char *name, PACKAGE *pkg, bool reportError )
  {
    if ( pkg == 0 )
    {
      CLASS * cls = get_class ( name, package );
      if ( cls )
        return cls;
      cls = get_class ( name, VM::std );
      if ( cls )
        return cls;
      throw parse_error ( this, "'%s' is not a class name", name );
    }
    ENTRY e = pkg->find ( name );
    if ( e.is_valid  () )
    {
      VALUE * v = e.value ();
      if ( v && v->v_type == DT_CLASS )
        return v->v.v_class;
    }
    if ( reportError )
      throw parse_error ( this, "'%s' is not a class name", name );

    return 0;
  }

  // get_class - get the klass associated with a symbol
  PACKAGE *
    compiler::get_package ( const char *name, bool reportError )
  {
    if ( package->name == VM::voc [ name ] )
      return package;
    int idx = VM::packages->find ( VM::voc [ name ] );
    if ( idx < 0 )
    {
      if ( reportError )
        throw parse_error ( this, "'%s' is not a package name", name );
      return 0;
    }
    return (PACKAGE *) ( *( VM::packages ) ) [ idx ].value.v.v_class;
  }

  CLASS *
    compiler::get_class_or_package ( string& class_id, string& package_id )
  {
    PACKAGE *pkg;
    if ( package_id.length () )
    {
      pkg = get_package ( (const char *) package_id, true );
      return get_class ( (const char *) class_id, pkg );
    }
    // first try to test is class_id is a package name
    pkg = get_package ( (const char *) class_id, false );
    if ( pkg )
      return pkg;

    return get_class ( (const char *) class_id );
  }

  // do_statement - compile a single statement
  int
    compiler::do_statement ()
  {
    int tkn;
    switch ( tkn = scan.token () )
    {
    case T_IF:		    do_if ();                           return tkn;
    case T_WHILE:	    do_while ();                        return tkn;
    case T_DO:		    do_dowhile ();                      return tkn;
    case T_FOR:		    do_for ();                          return tkn;
    case T_BREAK:	    do_break (); frequire ( ';' );      return tkn;
    case T_CONTINUE:	do_continue (); frequire ( ';' );   return tkn;
    case T_RETURN:	  do_return ();                       return tkn;
    case T_TRY:       do_try ();                          return tkn;
    case T_THROW:     do_throw ();                        return tkn;
    case T_SYNCHRO:   do_synchro ();                      return tkn;
    case T_SWITCH:    do_switch ();                       return tkn;
    case '{':         do_block ();                        break;
    case ';':         ;                                   return tkn;
    case T_VAR:       do_vardecl ();                      break;

    default:
      scan.stoken ( tkn );
      do_expr ();
      frequire ( ';' );
      break;
    }
    return 0;
  }

  // do_if - compile the IF/ELSE expression
  void
    compiler::do_if ()
  {
    int tkn, nxt, end;

    // compile the test expression
    do_test ();

    // skip around the 'then' clause if the expression is false
    putcbyte ( OP_BRF );
    nxt = putcword ( 0 );

    // compile the 'then' clause
    do_statement ();

    // compile the 'else' clause
    if ( ( tkn = scan.token () ) == T_ELSE )
    {
      putcbyte ( OP_BR );
      end = putcword ( 0 );
      fixup ( nxt, cptr );
      do_statement ();
      nxt = end;
    }
    else
      scan.stoken ( tkn );

    // handle the end of the statement
    fixup ( nxt, cptr );
  }

  void
    compiler::do_switch ()
  {
    compiler::PVAL pv;
    int nxt = 0, nxtbody = 0, end = 0, dflt = 0;

    int *old_break;

    frequire ( '(' );
    do_expr1 ( &pv );
    rvalue   ( &pv );
    frequire ( ')' );

    frequire ( '{' );

    old_break = addbreak ( 0 );

    putcbyte ( OP_PUSH );

    int tkn; int last_stm = 0;
    while ( (tkn = scan.token () ) != T_EOF )
    {
      if ( tkn == '}' )
        break;
      if ( tkn == T_CASE )
      {
        if ( nxt )
        {
          if ( last_stm != T_BREAK && last_stm != T_RETURN )
          {
            putcbyte ( OP_BR );
            nxtbody = putcword ( 0 );
          }
          fixup ( nxt, cptr );
        }
        else
          nxtbody = 0;
        putcbyte   ( OP_COPY );
        putcbyte   ( OP_PUSH );
        do_primary ( &pv );
        rvalue     ( &pv );
        frequire   ( ':' );
        putcbyte   ( OP_EQ );
        putcbyte   ( OP_BRF );
        nxt = putcword ( 0 );
        if ( nxtbody )
        {
          fixup ( nxtbody, cptr );
          nxtbody = 0;
        }
      }
      else if ( tkn == T_DEFAULT )
      {
        frequire ( ':' );
        if ( nxt == 0 )
        {
          putcbyte ( OP_BR );
          nxt = putcword ( 0 );
        }
        dflt = cptr;
      }
      else
      {
        if ( nxt == 0 )
          throw parse_error ( this, "Expecting 'case' | 'default'" );
        scan.stoken ( tkn );
        int ls = do_statement ();
        if ( ls != ';')
          last_stm = ls;
      }
    }

    end = rembreak ( old_break, end );

    if ( nxt )
    {
      if ( dflt )
        fixup ( nxt, dflt );
      else
        fixup ( nxt, cptr );
    }

    // handle the end of the statement
    fixup ( end, cptr );
    putcbyte ( OP_POP );
  }

  void
    compiler::do_try ()
  {
    int nxt, end;

    frequire ( '{' );

    putcbyte ( OP_EH_PUSH );
    nxt = putcword ( 0 );

    do_block ();

    putcbyte ( OP_EH_POP );
    end = putcword ( 0 );

    fixup ( nxt, cptr );

    frequire ( T_CATCH );
    frequire ( '(' );
    frequire ( T_IDENTIFIER );
    string ev = scan.t_token;
    frequire ( ')' );
    frequire ( '{' );
    do_block ( ev  );
    // handle the end of the statement
    fixup ( end, cptr );
  }

  void
    compiler::do_throw ()
  {
    do_expr  ();
    frequire ( ';' );
    putcbyte ( OP_THROW );
  }

  void
    compiler::do_synchro ()
  {
    compiler::PVAL pv;
    do_primary ( &pv );
    rvalue     ( &pv );
    frequire   ( '{' );
    putcbyte   ( OP_ENTER );
    do_block   ();
    putcbyte   ( OP_LEAVE );
  }

  // addbreak - add a break level to the stack
  int *
    compiler::addbreak ( int lbl )
  {
    int *old = bsp;
    if ( ++bsp < &bstack [ SSIZE ] )
      *bsp = lbl;
    else
      throw parse_error ( this, "Too many nested loops" );
    return ( old );
  }

  // rembreak - remove a break level from the stack
  int
    compiler::rembreak ( int *old, int lbl )
  {
    return ( bsp > old ? *bsp-- : lbl );
  }

  // addcontinue - add a continue level to the stack
  int *
    compiler::addcontinue ( int lbl )
  {
    int *old = csp;
    if ( ++csp < &cstack [ SSIZE ] )
      *csp = lbl;
    else
      throw parse_error ( this, "Too many nested loops" );
    return ( old );
  }

  // remcontinue - remove a continue level from the stack
  void
    compiler::remcontinue ( int *old )
  {
    csp = old;
  }

  // do_while - compile the WHILE expression
  void
    compiler::do_while ()
  {
    int nxt, end, *ob, *oc;

    // compile the test expression
    nxt = cptr;
    do_test ();

    // skip around the loop body if the expression is false
    putcbyte ( OP_BRF );
    end = putcword ( 0 );

    // compile the loop body
    ob = addbreak ( end );
    oc = addcontinue ( nxt );
    do_statement ();
    end = rembreak ( ob, end );
    remcontinue ( oc );

    // branch back to the start of the loop
    putcbyte ( OP_BR );
    putcword ( nxt );

    // handle the end of the statement
    fixup ( end, cptr );
  }

  // do_dowhile - compile the DO/WHILE expression
  void
    compiler::do_dowhile ()
  {
    int nxt, end = 0, *ob, *oc;

    // remember the start of the loop
    nxt = cptr;

    // compile the loop body
    ob = addbreak ( 0 );
    oc = addcontinue ( nxt );
    do_statement ();
    end = rembreak ( ob, end );
    remcontinue ( oc );

    // compile the test expression
    frequire ( T_WHILE );
    do_test ();
    frequire ( ';' );

    // branch to the top if the expression is true
    putcbyte ( OP_BRT );
    putcword ( nxt );

    // handle the end of the statement
    fixup ( end, cptr );
  }

  // do_for - compile the FOR statement
  void
    compiler::do_for ()
  {
    int tkn, nxt, end, body, update, *ob, *oc;

    // compile the initialization expression
    frequire ( '(' );
    if ( (tkn = scan.token () ) != ';' )
    {
      if ( tkn == T_VAR )
        do_vardecl ();
      else
      {
        scan.stoken ( tkn );
        do_expr ();
      }
      frequire ( ';' );
    }

    // compile the test expression
    nxt = cptr;
    if ( ( tkn = scan.token () ) != ';' )
    {
      scan.stoken ( tkn );
      do_expr ();
      frequire ( ';' );
    }

    // branch to the loop body if the expression is true
    putcbyte ( OP_BRT );
    body = putcword ( 0 );

    // branch to the end if the expression is false
    putcbyte ( OP_BR );
    end = putcword ( 0 );

    // compile the update expression
    update = cptr;
    if ( ( tkn = scan.token () ) != ')' )
    {
      scan.stoken ( tkn );
      do_expr ();
      frequire ( ')' );
    }

    // branch back to the test code
    putcbyte ( OP_BR );
    putcword ( nxt );

    // compile the loop body
    fixup ( body, cptr );
    ob = addbreak ( end );
    oc = addcontinue ( update );
    do_statement ();
    end = rembreak ( ob, end );
    remcontinue ( oc );

    // branch back to the update code
    putcbyte ( OP_BR );
    putcword ( update );

    // handle the end of the statement
    fixup ( end, cptr );
  }

  // do_break - compile the BREAK statement
  void
    compiler::do_break ()
  {
    if ( bsp >= bstack )
    {
      putcbyte ( OP_BR );
      *bsp = putcword ( *bsp );
    }
    else
      throw parse_error ( this, "Break outside of loop" );
  }

  // do_continue - compile the CONTINUE statement
  void
    compiler::do_continue ()
  {
    if ( csp >= cstack )
    {
      putcbyte ( OP_BR );
      putcword ( *csp );
    }
    else
      throw parse_error ( this, "Continue outside of loop" );
  }

  // do_block - compile the {} expression
  int
    compiler::do_block ( const char *parameter )
  {
    name_space *save_temp = temporaries;
    temporaries = new name_space ( temporaries );

    if ( parameter )
    {
      int temp_var_index = temporaries->add ( parameter );
      code_temporary ( this, STORE, temp_var_index );
    }

    int tkn;
    int last_stat = 0;
    if ( (tkn = scan.token () ) != '}' )
    {
      do
      {
        scan.stoken ( tkn );
        last_stat = do_statement ();
      }
      while ( (tkn = scan.token () ) != '}' );
    }
    else
      putcbyte ( OP_UNDEFINED );

    if ( save_temp )
    {
      delete temporaries;
      temporaries = save_temp;
    }

    return last_stat;
  }

  // do_return - handle the RETURN expression
  void
    compiler::do_return ()
  {
    if ( inConstructor )
    {
      compiler::PVAL pv;
      findvariable ( "this", &pv );
      rvalue ( &pv );
    }
    else
    {
      int tkn = scan.token ();
      scan.stoken ( tkn );
      if ( tkn == ';')
        putcbyte ( OP_UNDEFINED );
      else do_expr ();
    }
    frequire ( ';' );
    putcbyte ( OP_RETURN );
  }

  // do_test - compile a test expression
  void
    compiler::do_test ()
  {
    frequire ( '(' );
    do_expr  ();
    frequire ( ')' );
  }

  // do_expr - parse an expression
  void
    compiler::do_expr ()
  {
    compiler::PVAL pv;
    do_expr1 ( &pv );
    rvalue   ( &pv );
  }

  // rvalue - get the rvalue of a partial expression
  void
    compiler::rvalue ( compiler::PVAL *pv )
  {
    if ( pv->fcn )
    {
      ( *pv->fcn ) ( this, LOAD, pv->val );
      pv->fcn = NULL;
    }
  }

  // chklvalue - make sure we've got an lvalue
  void
    compiler::chklvalue ( compiler::PVAL *pv )
  {
    if ( !pv->fcn )
      throw parse_error ( this, "Expecting an lvalue" );
  }

  // do_expr1 - handle the ',' operator
  void
    compiler::do_expr1 ( compiler::PVAL *pv )
  {
    int tkn;
    do_expr2 ( pv );
    while ( ( tkn = scan.token () ) == ',' )
    {
      rvalue   ( pv );
      do_expr1 ( pv );
      rvalue   ( pv );
    }
    scan.stoken ( tkn );
  }

  // do_vardecl - var declaration
  void
    compiler::do_vardecl ( void )
  {
    int tkn;
    string id;
    compiler::PVAL rhs;
    for ( ; ; )
    {
      tkn = scan.token ();
      if ( tkn != T_IDENTIFIER )
        throw parse_error ( this, "Expecting an identifier" );
      id = scan.t_token;
      int temp_var_num = temporaries->add ( id );

      if ( temp_var_num < 0  )
      {
        char msg [ 100 ];
        sprintf ( msg, "Variable '%s' already defined", (const char *) id );
        throw parse_error ( this, msg );
      }

      tkn = scan.token ();
      if ( tkn == '=' )
      {
        code_temporary ( this, PUSH, 0 );
        do_expr2 ( &rhs );
        rvalue   ( &rhs );
        code_temporary ( this, STORE, temp_var_num );
        tkn = scan.token ();
      }
      else
      {
        putcbyte ( OP_UNDEFINED );
        code_temporary ( this, STORE, temp_var_num );
      }

      if ( tkn == ',' )
        continue;
      else if ( tkn == ';' )
      {
        scan.stoken ( tkn );
        break;
      }
      else
        throw parse_error ( this, "Expecting ',' | ';'" );
    }
  }

  // do_expr2 - handle the assignment operators
  void
    compiler::do_expr2 ( compiler::PVAL *pv )
  {
    int tkn;
    compiler::PVAL rhs;
    do_expr3 ( pv );
    while ( (tkn = scan.token () ) == '='     ||
             tkn == T_ADDEQ || tkn == T_SUBEQ ||
             tkn == T_MULEQ || tkn == T_DIVEQ ||
             tkn == T_REMEQ || tkn == T_ANDEQ ||
             tkn == T_OREQ  || tkn == T_XOREQ ||
             tkn == T_SHLEQ || tkn == T_SHLEQ  )
    {
      chklvalue ( pv );
      switch ( tkn )
      {
      case '=':
        ( *pv->fcn ) ( this, PUSH, 0 );
        do_expr1 ( &rhs );
        rvalue   ( &rhs );
        ( *pv->fcn ) ( this, STORE, pv->val );
        break;
      case T_ADDEQ:	    do_assignment ( pv, OP_ADD );	    break;
      case T_SUBEQ:	    do_assignment ( pv, OP_SUB );	    break;
      case T_MULEQ:	    do_assignment ( pv, OP_MUL );	    break;
      case T_DIVEQ:	    do_assignment ( pv, OP_DIV );	    break;
      case T_REMEQ:	    do_assignment ( pv, OP_REM );	    break;
      case T_ANDEQ:	    do_assignment ( pv, OP_BAND );	  break;
      case T_OREQ:	    do_assignment ( pv, OP_BOR );	    break;
      case T_XOREQ:	    do_assignment ( pv, OP_XOR );	    break;
      case T_SHLEQ:	    do_assignment ( pv, OP_SHL );	    break;
      case T_SHREQ:	    do_assignment ( pv, OP_SHR );	    break;
      }
      pv->fcn = NULL;
    }
    scan.stoken ( tkn );
  }

  // do_assignment - handle assignment operations
  void
    compiler::do_assignment ( compiler::PVAL *pv, int op )
  {
    compiler::PVAL rhs;
    ( *pv->fcn ) ( this, DUP, 0 );
    ( *pv->fcn ) ( this, LOAD, pv->val );
    putcbyte ( OP_PUSH );
    do_expr1 ( &rhs );
    rvalue   ( &rhs );
    putcbyte ( op );
    ( *pv->fcn ) ( this, STORE, pv->val );
  }

  // do_expr3 - handle the '?:' operator
  void
    compiler::do_expr3 ( compiler::PVAL *pv )
  {
    int tkn, nxt, end;
    do_expr4 ( pv );
    while ( (tkn = scan.token  () ) == '?' )
    {
      rvalue ( pv );
      putcbyte ( OP_BRF );
      nxt = putcword ( 0 );
      do_expr1 ( pv );
      rvalue ( pv );
      frequire ( ':' );
      putcbyte ( OP_BR );
      end = putcword ( 0 );
      fixup ( nxt, cptr );
      do_expr1 ( pv );
      rvalue ( pv );
      fixup ( end, cptr );
    }
    scan.stoken ( tkn );
  }

  // do_expr4 - handle the '||' operator
  void
    compiler::do_expr4 ( compiler::PVAL *pv )
  {
    int tkn, end = 0;
    do_expr5 ( pv );
    while ( ( tkn = scan.token () ) == T_OR )
    {
      rvalue ( pv );
      putcbyte ( OP_BRT );
      end = putcword ( end );
      do_expr5 ( pv );
      rvalue ( pv );
    }
    fixup ( end, cptr );
    scan.stoken ( tkn );
  }

  // do_expr5 - handle the '&&' operator
  void
    compiler::do_expr5 ( compiler::PVAL *pv )
  {
    int tkn, end = 0;
    do_expr6 ( pv );
    while ( ( tkn = scan.token () ) == T_AND )
    {
      rvalue ( pv );
      putcbyte ( OP_BRF );
      end = putcword ( end );
      do_expr6 ( pv );
      rvalue ( pv );
    }
    fixup ( end, cptr );
    scan.stoken ( tkn );
  }

  // do_expr6 - handle the '|' operator
  void
    compiler::do_expr6 ( compiler::PVAL *pv )
  {
    int tkn;
    do_expr7 ( pv );
    while ( ( tkn = scan.token () ) == '|' )
    {
      rvalue ( pv );
      putcbyte ( OP_PUSH );
      do_expr7 ( pv );
      rvalue   ( pv );
      putcbyte ( OP_BOR );
    }
    scan.stoken ( tkn );
  }

  // do_expr7 - handle the '^' operator
  void
    compiler::do_expr7 ( compiler::PVAL *pv )
  {
    int tkn;
    do_expr8 ( pv );
    while ( ( tkn = scan.token () ) == '^' )
    {
      rvalue ( pv );
      putcbyte ( OP_PUSH );
      do_expr8 ( pv );
      rvalue   ( pv );
      putcbyte ( OP_XOR );
    }
    scan.stoken ( tkn );
  }

  // do_expr8 - handle the '&' operator
  void
    compiler::do_expr8 ( compiler::PVAL *pv )
  {
    int tkn;
    do_expr9 ( pv );
    while ( ( tkn = scan.token () ) == '&' )
    {
      rvalue ( pv );
      putcbyte ( OP_PUSH );
      do_expr9 ( pv );
      rvalue   ( pv );
      putcbyte ( OP_BAND );
    }
    scan.stoken ( tkn );
  }

  // do_expr9 - handle the '==' and '!=' operators
  void
    compiler::do_expr9 ( compiler::PVAL *pv )
  {
    int tkn, op;
    do_expr10 ( pv );
    while ( ( tkn = scan.token () ) == T_EQ || tkn == T_NE )
    {
      switch ( tkn )
      {
      case T_EQ: op = OP_EQ; break;
      case T_NE: op = OP_NE; break;
      }
      rvalue ( pv );
      putcbyte ( OP_PUSH );
      do_expr10 ( pv );
      rvalue ( pv );
      putcbyte ( op );
    }
    scan.stoken ( tkn );
  }

  // do_expr10 - handle the '<', '<=', '>=' and '>' operators
  void
    compiler::do_expr10 ( compiler::PVAL *pv )
  {
    int tkn, op;
    do_expr11 ( pv );
    while ( ( tkn = scan.token () ) == '<' ||
              tkn == T_LE || tkn == T_GE || tkn == '>' )
    {
      switch ( tkn )
      {
      case '<':  op = OP_LT; break;
      case T_LE: op = OP_LE; break;
      case T_GE: op = OP_GE; break;
      case '>':  op = OP_GT; break;
      }
      rvalue ( pv );
      putcbyte ( OP_PUSH );
      do_expr11 ( pv );
      rvalue    ( pv );
      putcbyte  ( op );
    }
    scan.stoken ( tkn );
  }

  // do_expr11 - handle the '<<' and '>>' operators
  void
    compiler::do_expr11 ( compiler::PVAL *pv )
  {
    int tkn, op;
    do_expr12 ( pv );
    while ( ( tkn = scan.token () ) == T_SHL || tkn == T_SHR )
    {
      switch ( tkn  )
      {
      case T_SHL: op = OP_SHL; break;
      case T_SHR: op = OP_SHR; break;
      }
      rvalue ( pv );
      putcbyte ( OP_PUSH );
      do_expr12 ( pv );
      rvalue ( pv );
      putcbyte ( op );
    }
    scan.stoken ( tkn );
  }

  // do_expr12 - handle the '+' and '-' operators
  void
    compiler::do_expr12 ( compiler::PVAL *pv )
  {
    int tkn, op;
    do_expr13 ( pv );
    while ( ( tkn = scan.token () ) == '+' || tkn == '-' )
    {
      switch ( tkn )
      {
      case '+': op = OP_ADD; break;
      case '-': op = OP_SUB; break;
      }
      rvalue ( pv );
      putcbyte ( OP_PUSH );
      do_expr13 ( pv );
      rvalue    ( pv );
      putcbyte  ( op );
    }
    scan.stoken ( tkn );
  }

  // do_expr13 - handle the '*' and '/' operators
  void
    compiler::do_expr13 ( compiler::PVAL *pv )
  {
    int tkn, op;
    do_expr14 ( pv );
    while ( ( tkn = scan.token () ) == '*' || tkn == '/' ||
              tkn == '%' || tkn == T_MAKEREF || tkn == T_INSTANCEOF )
    {
      switch ( tkn )
      {
      case '*':           op = OP_MUL;        break;
      case '/':           op = OP_DIV;        break;
      case '%':           op = OP_REM;        break;
      case T_MAKEREF:     op = OP_MAKEREF;    break;
      case T_INSTANCEOF:  op = OP_INSTANCEOF; break;
      }
      rvalue ( pv );
      putcbyte ( OP_PUSH );
      do_expr14 ( pv );
      rvalue    ( pv );
      putcbyte  ( op );
    }
    scan.stoken ( tkn );
  }

  // do_expr14 - handle unary operators
  void
    compiler::do_expr14 ( compiler::PVAL *pv )
  {
    int tkn;
    switch ( tkn = scan.token () )
    {
    case '-':
      do_expr15 ( pv );
      rvalue    ( pv );
      putcbyte  ( OP_NEG );
      break;
    case '!':
      do_expr15 ( pv );
      rvalue    ( pv );
      putcbyte  ( OP_NOT );
      break;
    case '~':
      do_expr15 ( pv );
      rvalue    ( pv );
      putcbyte  ( OP_BNOT );
      break;
    case T_INC:
      do_preincrement ( pv, OP_INC );
      break;
    case T_DEC:
      do_preincrement ( pv, OP_DEC );
      break;
    case T_NEW:
      do_new ( pv );
      break;
    default:
      scan.stoken ( tkn );
      do_expr15   ( pv );
      return;
    }
  }

  void
    compiler::do_lit_array ( compiler::PVAL *pv )
  {
    compiler::PVAL pc;
    CLASS *klass = get_class ( "array", VM::std );
    findclassvariable ( klass, "[", &pc );
    rvalue ( &pc );
    // put <undefined> as this
    putcbyte ( OP_PUSH );
    int tkn, n = 1;
    // compile each argument expression
    if ( ( tkn = scan.token () ) != ']' )
    {
      scan.stoken ( tkn );
      do
      {
        putcbyte ( OP_PUSH );
        compiler::PVAL ai;
        do_expr2 ( &ai );
        rvalue ( &ai );
        ++n;
        if ( n >= 10000 ) throw parse_error ( this, "Too many literal items" );
      }
      while ( ( tkn = scan.token () ) == ',' );
    }
    require ( tkn, ']' );
    putcbyte ( OP_CALL );
    putcword ( n );

    // we've got an rvalue now
    pv->fcn = NULL;
  }

  void
    compiler::do_lit_map ( compiler::PVAL *pv )
  {
    compiler::PVAL pc;
    CLASS *klass = get_class ( "map", VM::std );
    findclassvariable ( klass, "{", &pc );
    rvalue ( &pc );
    // put <undefined> as this
    putcbyte ( OP_PUSH );
    int tkn, n = 1;
    // compile each argument expression
    if ( ( tkn = scan.token () ) != '}' )
    {
      scan.stoken ( tkn );
      do
      {
        compiler::PVAL l, r;
        putcbyte ( OP_PUSH );
        do_expr2 ( &l );
        rvalue ( &l );
        tkn = scan.token ();
        require ( tkn, ':' );
        putcbyte ( OP_PUSH );
        do_expr2 ( &r );
        rvalue ( &r );
        n += 2;
        if ( n >= 10000 )
          throw parse_error ( this, "Too many literal items" );
      }
      while ( ( tkn = scan.token () ) == ',' );
    }
    require  ( tkn, '}' );
    putcbyte ( OP_CALL );
    putcword ( n );

    // we've got an rvalue now
    pv->fcn = NULL;
  }

  // do_preincrement - handle prefix '++' and '--'
  void
    compiler::do_preincrement ( compiler::PVAL *pv, int op )
  {
    do_expr15 ( pv );
    chklvalue ( pv );
    ( *pv->fcn ) ( this, DUP, 0 );
    ( *pv->fcn ) ( this, LOAD, pv->val );
    putcbyte ( op );
    ( *pv->fcn ) ( this, STORE, pv->val );
    pv->fcn = NULL;
  }

  // do_postincrement - handle postfix '++' and '--'
  void
    compiler::do_postincrement ( compiler::PVAL *pv, int op )
  {
    chklvalue    ( pv );
    ( *pv->fcn ) ( this, DUP, 0 );
    ( *pv->fcn ) ( this, LOAD, pv->val );
    putcbyte     ( OP_TSTORE );
    putcbyte     ( op );

    ( *pv->fcn ) ( this, STORE, pv->val );
    putcbyte ( OP_TRESTORE );

    pv->fcn = NULL;
  }

  // do_new - handle the 'new' operator
  void
    compiler::do_new ( compiler::PVAL *pv )
  {
    string  package_name, class_name;
    int     tkn;
    CLASS * klass = 0;

    frequire ( T_IDENTIFIER );
    class_name = scan.t_token;

    if ( ( tkn = scan.token () ) == T_CC )
    {
      frequire ( T_IDENTIFIER );
      package_name = class_name;
      class_name = scan.t_token;

      PACKAGE *pkg = get_package ( package_name, true );
      klass = get_class ( class_name, pkg, true );
    }
    else
    {
      scan.stoken ( tkn );
      klass = get_class ( class_name, 0, true );
    }

    code_literal ( this, package->add_literal ( VALUE ( klass ) ) );
    pv->fcn = NULL;

    int n = 1;

    // generate code to push the selector
    putcbyte ( OP_PUSH );
    code_literal ( this,
                   package->add_literal (
                                VM::voc [ (const char *) class_name ] ) );

    if ( ( tkn = scan.token () ) == '(' )
    {
      // compile the argument arglist
      if ( ( tkn = scan.token () ) != ')' )
      {
        scan.stoken ( tkn );
        do
        {
          putcbyte ( OP_PUSH );
          do_expr2 ( pv );
          rvalue   ( pv );
          ++n;
        }
        while ( ( tkn = scan.token () ) == ',' );
      }
      require ( tkn, ')' );
    }
    else
      scan.stoken ( tkn );

    // send the message
    putcbyte ( OP_NEW );
    putcbyte ( n );

    // we've got an rvalue now
    pv->fcn = NULL;

  }

  // do_expr15 - handle function calls
  void
    compiler::do_expr15 ( compiler::PVAL *pv )
  {
    string selector;
    int tkn;
    do_primary ( pv );
    while ( ( tkn = scan.token () ) == '('  ||
              tkn == '[' || tkn == T_MEMREF ||
              tkn == T_INC || tkn == T_DEC )
      switch ( tkn  )
      {
      case '(':
        do_call ( pv );
        break;
      case '[':
        do_index ( pv );
        break;
      case T_MEMREF:
        frequire ( T_IDENTIFIER );
        selector = scan.t_token;
        do_memref ( selector, pv );
        break;
      case T_INC:
        do_postincrement ( pv, OP_INC );
        break;
      case T_DEC:
        do_postincrement ( pv, OP_DEC );
        break;
      }
    scan.stoken ( tkn );
  }

  // do_primary - parse a primary expression and unary operators
  void
    compiler::do_primary ( compiler::PVAL *pv )
  {
    CLASS *klass;
    int tkn;
    switch ( scan.token () )
    {
    case '(':
      do_expr1 ( pv  );
      frequire ( ')' );
      break;
    case T_NUMBER:
      do_lit_number ( scan.t_value );
      pv->fcn = NULL;
      break;
    case T_STRING:
      do_lit_string ( scan.t_token );
      pv->fcn = NULL;
      break;
    case T_NULL:
      putcbyte ( OP_NULL );
      pv->fcn = NULL;
      break;
    case T_UNDEFINED:
      putcbyte ( OP_UNDEFINED );
      pv->fcn = NULL;
      break;
    case T_ARGUMENTS:
      putcbyte ( OP_ARGUMENTS );
      pv->fcn = NULL;
      break;
    case T_ARGUMENT:
      do_expr2 ( pv );
      rvalue ( pv );
      putcbyte ( OP_ARGUMENT );
      pv->fcn = NULL;
      break;
    case '[':
      do_lit_array ( pv );
      break;
    case '{':
      do_lit_map ( pv );
      break;
    case T_IDENTIFIER:
      {
        string member_id = scan.t_token;
        if ( ( tkn = scan.token () ) == T_CC )
        {
          frequire ( T_IDENTIFIER );

          string package_id;
          string class_id = member_id;
          member_id = scan.t_token;

          if ( ( tkn = scan.token  () ) == T_CC )
          {
            frequire ( T_IDENTIFIER );
            package_id = class_id;
            class_id = member_id;
            member_id = scan.t_token;
          }
          else
          {
            scan.stoken ( tkn );
          }

          klass = get_class_or_package ( class_id, package_id );
          assert ( klass );

          if ( ! findclassvariable ( klass, (const char *) member_id, pv ) )
            throw parse_error ( this, "'%s' is not a member of class '%s'",
                                (const char *) member_id,
                                (const char *) klass->full_name () );
        }
        else
        {
          scan.stoken ( tkn );
          findvariable ( (const char *) member_id, pv );
        }
      }
      break;
    default:
      throw parse_error ( this, "Expecting a primary expression" );
      break;
    }
  }


  // do_call - compile a function call
  void
    compiler::do_call ( compiler::PVAL *pv )
  {
    int tkn, n = 1;

    // get the value of the function
    rvalue ( pv );

    // put <undefined> as this
    putcbyte ( OP_PUSH );

    // compile each argument expression
    if ( ( tkn = scan.token () ) != ')' )
    {
      scan.stoken ( tkn );
      do
      {
        putcbyte ( OP_PUSH );
        do_expr2 ( pv );
        rvalue   ( pv );
        if ( ++n >= 10000 )
          throw parse_error ( this, "Too many parameters" );
      }
      while ( ( tkn = scan.token () ) == ',' );
    }
    require  ( tkn, ')' );
    putcbyte ( OP_CALL );
    putcword ( n );

    // we've got an rvalue now
    pv->fcn = NULL;
  }

  // do_send - compile a message sending expression
  void
    compiler::do_memref ( const char *selector, compiler::PVAL *pv )
  {
    // get the receiver value
    rvalue ( pv );
    // generate code to push the selector
    putcbyte ( OP_PUSH );
    code_literal ( this, package->add_literal ( VM::voc [ selector ] ) );
    pv->fcn = code_prop_member;
  }


  // do_index - compile an indexing operation
  void
    compiler::do_index ( compiler::PVAL *pv )
  {
    rvalue   ( pv );
    putcbyte ( OP_PUSH );
    do_expr  ();
    frequire ( ']' );
    pv->fcn = code_index;
  }


  // get_id_list - get a comma separated arglist of identifiers
  int
    compiler::get_id_list ( sym_table& st, const char *term )
  {
    int tkn, cnt = 0; symbol_t t;
    tkn = scan.token ();
    if ( ! strchr ( term, tkn ) )
    {
      scan.stoken ( tkn );
      do
      {
        frequire ( T_IDENTIFIER );
        if ( st.find ( scan.t_token, t ) )
        {
          char buf [ 100 ];
          sprintf ( buf, "'%s' already defined.",
                    (const char *) scan.t_token );
          throw parse_error ( this, buf );
        }
        else
          (void) st [ scan.t_token ];

      }
      while ( ( tkn = scan.token () ) == ',' );
    }
    scan.stoken ( tkn );
    return st.size ();
  }


  // findarg - find an argument offset
  int
    compiler::findarg ( const char *name )
  {
    symbol_t t;
    return arguments.find ( name, t ) ? (int) t : -1;
  }

  // findtmp - find a temporary variable offset
  int
    compiler::findtmp ( const char *name )
  {
    if ( temporaries == 0 )
      return -1;
    return (int) temporaries->find ( name );
  }

  // finddatamember - find a klass data member
  ENTRY
    compiler::finddatamember ( const char *name )
  {
    CLASS *klass = methodclass;
    if ( klass )
    {
      do
      {
        ENTRY e = klass->find ( name );
        if ( e.is_valid () )
          return e;
        klass = klass->base;
      }
      while ( klass );
    }
    return ENTRY::undefined ();
  }

  // frequire - fetch a token and check it
  void
    compiler::frequire ( int rtkn )
  {
    require ( scan.token (), rtkn );
  }

  // require - check for a required token
  void
    compiler::require ( int tkn, int rtkn )
  {
    char msg [ 100 ], tknbuf [ 100 ];
    if ( tkn != rtkn  )
    {
      strncpy ( tknbuf, scan.tkn_name ( rtkn ), 100 );
      snprintf ( msg, 100, "Expecting '%s', found '%s'", tknbuf, scan.tkn_name ( tkn ) );
      throw parse_error ( this, msg );
    }
  }

  // do_lit_integer - compile a literal number
  void
    compiler::do_lit_number ( VALUE v )
  {
    code_literal ( this, package->add_literal ( v ) );
  }

  // do_lit_string - compile a literal string
  void
    compiler::do_lit_string ( const char *str )
  {
    code_literal ( this, package->add_literal ( str ) );
  }

  // make_lit_symbol - make a literal string

  int
    compiler::make_lit_symbol ( const char *str )
  {
    return package->add_literal ( VM::voc [ str ] );
  }

  // make_lit_string - make a literal string
  int
    compiler::make_lit_string ( const char *str )
  {
    return package->add_literal ( str );
  }

  // make_lit_variable - make a literal reference to a variable
  int
    compiler::make_lit_variable ( ENTRY e )
  {
    return package->add_literal ( VALUE ( e ) );
  }

  // findvariable - find a variable
  void
    compiler::findvariable ( const char *id, compiler::PVAL *pv )
  {
    int n;
    if ( ( n = findarg ( id ) ) >= 0 )
    {
      pv->fcn = code_argument;
      pv->val = n;
    }
    else if ( ( n = findtmp ( id ) ) >= 0 )
    {
      pv->fcn = code_temporary;
      pv->val = n;
    }
    else if ( methodclass && findclassvariable ( methodclass, id, pv ) )
    {
      ;
    }
    else if ( findclassvariable ( package, id, pv ) )
    {
      ;
    }
    else if ( findclassvariable ( VM::std, id, pv ) )
    {
      ;
    }
    else
      throw parse_error ( this, "Variable or method '%s' not found", id );
  }

  // findclassvariable - find a klass member variable
  int
    compiler::findclassvariable ( CLASS *klass, const char *name,
                                  compiler::PVAL *pv )
  {
    ENTRY e = rfindmember ( klass, name );
    if ( !e.is_valid () )
      return false;
    switch ( e.type () )
    {
    case ST_DATA:
      pv->fcn = code_member;
      pv->val = e.value()->v.v_integer;
      break;
    case ST_SDATA:
      pv->fcn = code_variable;
      pv->val = make_lit_variable ( e );
      break;
    case ST_CONST:
      pv->fcn = code_const;
      pv->val = make_lit_variable ( e );
      break;
    case ST_FUNCTION:
      if ( findarg ( "this" ) >= 0 )
      {
        findvariable ( "this", pv );
        do_memref ( name, pv );
        break;
      }

    case ST_SFUNCTION:
      code_variable ( this, LOAD, make_lit_variable ( e ) );
      pv->fcn = NULL;
      break;
    }
    return true;
  }

  // code_argument - compile an argument reference
  void
    compiler::code_argument ( compiler *c, int fcn, int n )
  {
    switch ( fcn )
    {
    case LOAD:  c->putcbyte ( OP_AREF ); c->putcbyte ( n ); break;
    case STORE: c->putcbyte ( OP_ASET ); c->putcbyte ( n ); break;
    }
  }

  // code_temporary - compile a temporary variable reference
  void
    compiler::code_temporary ( compiler *c, int fcn, int n )
  {
    switch ( fcn )
    {
    case LOAD:  c->putcbyte ( OP_TREF ); c->putcbyte ( n ); break;
    case STORE: c->putcbyte ( OP_TSET ); c->putcbyte ( n ); break;
    }
  }

  // code_member - compile a data member reference
  void
    compiler::code_member ( compiler *c, int fcn, int n )
  {
    switch ( fcn  )
    {
    case LOAD:  c->putcbyte ( OP_MREF ); c->putcbyte ( n ); break;
    case STORE: c->putcbyte ( OP_MSET ); c->putcbyte ( n ); break;
    }
  }

  // code_variable - compile a variable reference
  void
    compiler::code_variable ( compiler *c, int fcn, int n )
  {
    switch ( fcn )
    {
    case LOAD:  c->putcbyte ( OP_REF ); c->putcword ( n ); break;
    case STORE: c->putcbyte ( OP_SET ); c->putcword ( n ); break;
    }
  }

  // code_variable - compile a variable reference
  void
    compiler::code_const ( compiler *c, int fcn, int n )
  {
    switch ( fcn  )
    {
    case LOAD:	c->putcbyte ( OP_REF ); c->putcword ( n ); break;
    case STORE:	throw parse_error ( c, "attempt to change constant" );
    }
  }

  // code_index - compile an indexed reference
  void
    compiler::code_index ( compiler *c, int fcn, int n )
  {
    switch ( fcn )
    {
    case LOAD:  c->putcbyte ( OP_VREF ); break;
    case STORE: c->putcbyte ( OP_VSET ); break;
    case PUSH:  c->putcbyte ( OP_PUSH ); break;
    case DUP:   c->putcbyte ( OP_DUP2 ); break;
    }
  }

  // code_prop_member - compile an object property reference
  void
    compiler::code_prop_member ( compiler *c, int fcn, int n )
  {
    switch ( fcn )
    {
    case LOAD:  c->putcbyte ( OP_PMREF ); break;
    case STORE: c->putcbyte ( OP_PMSET ); break;
    case PUSH:  c->putcbyte ( OP_PUSH );  break;
    case DUP:   c->putcbyte ( OP_DUP2 );  break;
    }
  }

  // code_literal - compile a literal reference
  void
    compiler::code_literal ( compiler *c, int n )
  {
    c->putcbyte ( OP_LIT );
    c->putcword ( n );
  }

  // putcbyte - put a code byte into data space
  int
    compiler::putcbyte ( int b )
  {
    if ( file_line != scan.lnum )
    {
      file_line = scan.lnum;
      _putcbyte ( OP_LINE );
      putcword ( file_line );
    }
    return _putcbyte ( b );
  }


  // putcbyte - put a code byte into data space
  int
    compiler::_putcbyte ( int b )
  {
    if ( cptr >= CMAX )
      throw parse_error ( this, "Insufficient code space" );
    cbuff [ cptr ] = b;
    return ( cptr++ );
  }


  // putcword - put a code word into data space
  int
    compiler::putcword ( int w )
  {
    _putcbyte ( w );
    _putcbyte ( w >> 8 );
    return ( cptr - 2 );
  }

  // fixup - fixup a reference chain
  void
    compiler::fixup ( int chn, int val )
  {
    int hval, nxt;
    for ( hval = val >> 8; chn != 0; chn = nxt )
    {
      nxt = ( cbuff [ chn ] & 0xFF ) | ( cbuff [ chn + 1 ] << 8 );
      cbuff [ chn     ] = val;
      cbuff [ chn + 1 ] = hval;
    }
  }
#endif //COMPILER

};
