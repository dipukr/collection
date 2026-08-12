/*
*
* compiler.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov - kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

#include "vm.h"
#include "tool.h"
#include "scanner.h"
#include "sym_table.h"

namespace c_smile
{
  /* limits */
#define CMAX		32767	/* code buffer size */

  /* token definitions */
#define T_NOTOKEN     -1
#define T_EOF         0

  /* non-character tokens */
#define _TMIN         256
#define T_STRING      256
#define T_IDENTIFIER  257
#define T_NUMBER	    258
#define T_CLASS	      259
#define T_STATIC	    260
#define T_IF		      261
#define T_ELSE	      262
#define T_WHILE		    263
#define T_RETURN	    264
#define T_FOR	        265
#define T_BREAK		    266
#define T_CONTINUE	  267
#define T_DO		      268
#define T_NEW	        269
#define T_NULL		    270
#define T_LE		      271	/* '< = ' */
#define T_EQ      	  272	/* '= = ' */
#define T_NE      	  273	/* '! = ' */
#define T_GE      	  274	/* '> = ' */
#define T_SHL	        275	/* '<<' */
#define T_SHR         276	/* '>>' */
#define T_AND         277	/* '&&' */
#define T_OR          278	/* '||' */
#define T_INC         279	/* '++' */
#define T_DEC         280	/* '--' */
#define T_ADDEQ       281	/* '+ = ' */
#define T_SUBEQ       282	/* '- = ' */
#define T_MULEQ       283	/* '* = ' */
#define T_DIVEQ       284	/* '/ = ' */
#define T_REMEQ       285	/* '% = ' */
#define T_ANDEQ       286	/* '& = ' */
#define T_OREQ        287	/* '| = ' */
#define T_XOREQ       288	/* '^ = ' */
#define T_SHLEQ       289	/* '<< = ' */
#define T_SHREQ       290	/* '>> = ' */
#define T_CC          291	/* '::' */
#define T_MEMREF      292	/* '.' */
#define T_VAR         293
#define T_FUNCTION    294
#define T_PACKAGE     295
#define T_TRY         296
#define T_CATCH       297
#define T_THROW       298
#define T_PROPERTY    299
#define T_IMPORT      300
#define T_INSTANCEOF  301
#define T_UNDEFINED   302
#define T_ARGUMENT    303
#define T_ARGUMENTS   304
#define T_SWITCH      305
#define T_CASE        306
#define T_DEFAULT     307
#define T_MAKEREF     308
#define T_SYNCHRO     309
#define T_CONST       310

#define _TMAX         310

  class compiler;
  // partial value structure

  // break/continue stacks size
#define SSIZE	10

  class VM;

  class name_space
  {
    friend class compiler;
    sym_table     symbols;
    name_space *  parent;
    size_t        offset;
  public:

    name_space ( name_space *p = 0 ) : parent ( p ),  offset ( 0 ) 
    {
      offset = get_offset ();
    }

    ~name_space () 
    {
    }

    symbol_t
      find ( const char * name ) 
    {
      symbol_t sym;
      if ( symbols.find ( name, sym ) ) 
      {
        if ( parent ) 
          return offset + sym;
        else
          return sym;
      }
      if ( parent ) 
        return parent->find ( name );
      return undefined_symbol;
    }

    symbol_t
      add ( const char*name ) 
    {
      if ( symbols.exists ( name ) ) 
      {
        return undefined_symbol;
      }
      // error:already defined
      symbol_t t = symbols [ name ];

      if ( parent )
        return set_total ( offset + t );
      return set_total ( t );
    }

    size_t
      set_total ( size_t t ) 
    {
      if ( parent )
        parent->set_total ( t );
      // root
      else
      {
        size_t tt = t+1;
        if ( tt > offset )
          offset = tt;
      }
      return t;
    }

    size_t
      get_offset () 
    {
      if ( parent )
        return parent->get_offset () + parent->symbols.size ();
      else return 0;
    }

    size_t  get_total () 
    {
      if ( parent )
        return parent->get_total ();
      else
        return offset;
    }

    size_t
      size () 
    {
      return symbols.size ();
    }

    void
      clear () 
    {
      symbols.clear ();
      offset = 0;
    }

    const char *
      operator [] ( size_t idx ) 
    {
      return symbols [ idx ];
    }

  };

#ifdef COMPILER

  class compiler
  {
    friend class parse_error;

    // local variables

    CLASS         *methodclass;	// class of the current method
    unsigned char *cbuff;	 	    // code buffer
    int cptr;	 	 						    // code pointer

    name_space *  temporaries;  // namespace for locals
    sym_table     arguments;    // linear symtable for arguments

    PACKAGE *     package;      // current package
    string        file_name;
    string        dir_name;
    int           file_line;    // line number

    io_stream *   report_stream;

    // break/continue stacks
    int bstack [ SSIZE ];int *bsp;
    int cstack [ SSIZE ];int *csp;

    VALUE   temp;
    bool    inConstructor;

    scanner scan;


  public:

    struct PVAL;

    compiler ();

    ~compiler () 
    {
      //temporaries; // unwind temporaries stack please;
    }

    PACKAGE* compile ( io_stream *input ); //const char *fn,  int ( *getcf ) ( void * ),  void *getcd );
    void mark ();

  protected:
    void do_package ();
    void do_import ();
    void do_class ();
    ENTRY findmember ( CLASS *klass, const char *name );
    ENTRY rfindmember ( CLASS *klass, const char *name );
    void do_function ();
    void do_regular_function ( const char *name );
    void do_member_function ( CLASS *klass,  int type );
    void do_property_function ( CLASS *klass, int type );
    void do_member_data ( CLASS *klass, int type );

    CODE * do_code ( const char *name, CLASS *klass, int stype );

    PACKAGE *  get_package ( const char *name, bool reportError );
    CLASS *    get_class ( const char *name, PACKAGE *pkg = 0, bool reportError = false );
    CLASS *    get_class_or_package ( string& class_id, string& package_id );

    int  do_statement ();
    void do_if ();
    void do_switch ();
    int *addbreak ( int lbl );
    int  rembreak ( int *old, int lbl );
    int *addcontinue ( int lbl );
    void remcontinue ( int *old );
    void do_while ();
    void do_dowhile ();
    void do_for ();
    void do_break ();
    void do_continue ();
    int  do_block ( const char *parameter = 0 );
    void do_return ();
    void do_test ();
    void do_expr ();
    void do_vardecl ();
    void do_static_vardecl ( CLASS *klass,  bool constant );

    void do_try ();
    void do_throw ();

    void do_synchro ();

    void rvalue ( PVAL *pv );
    void chklvalue ( PVAL *pv );

    void do_assignment ( PVAL *pv, int op );

    void do_expr1 ( PVAL *pv );
    void do_expr2 ( PVAL *pv );
    void do_expr3 ( PVAL *pv );
    void do_expr4 ( PVAL *pv );
    void do_expr5 ( PVAL *pv );
    void do_expr6 ( PVAL *pv );
    void do_expr7 ( PVAL *pv );
    void do_expr8 ( PVAL *pv );
    void do_expr9 ( PVAL *pv );
    void do_expr10 ( PVAL *pv );
    void do_expr11 ( PVAL *pv );
    void do_expr12 ( PVAL *pv );
    void do_expr13 ( PVAL *pv );
    void do_expr14 ( PVAL *pv );
    void do_expr15 ( PVAL *pv );

    void do_preincrement ( PVAL *pv, int op );
    void do_postincrement ( PVAL *pv, int op );
    void do_new ( PVAL *pv );
    void do_primary ( PVAL *pv );
    void do_call ( PVAL *pv );

    void do_memref ( const char *selector, PVAL *pv );

    void do_index ( PVAL *pv );
    void do_lit_array ( PVAL *pv );
    void do_lit_map ( PVAL *pv );

    int	get_id_list ( sym_table &st, const char *term );

    int	 findarg ( const char *name );
    int	 findtmp ( const char *name );
    ENTRY finddatamember ( const char *name );

    void frequire ( int rtkn );
    void	require ( int tkn, int rtkn );
    void do_lit_number ( VALUE v );
    void do_lit_string ( const char *str );

    int	make_lit_string ( const char *str );
    int make_lit_symbol ( const char *str );
    int	make_lit_variable ( ENTRY e );
    void findvariable ( const char *id, PVAL *pv );
    int	findclassvariable ( CLASS *klass, const char *name, PVAL *pv );
    int	putcbyte ( int b );
    int	_putcbyte ( int b );
    int	putcword ( int w );
    void fixup ( int chn, int val );

    void report ( const char * format,  ... );

    // PVAL companions
    static void code_argument ( compiler *c,  int fcn, int n );
    static void code_temporary ( compiler *c,  int fcn, int n );
    static void code_member ( compiler *c,  int fcn, int n );
    static void code_variable ( compiler *c,  int fcn, int n );
    static void code_index ( compiler *c,  int fcn, int n );
    static void code_prop_member ( compiler *c, int fcn, int n );
    static void code_literal ( compiler *c,  int n );
    static void code_const ( compiler *c, int fcn, int n );

  public:
    struct PVAL
    {
      void ( *fcn ) ( compiler *comp, int, int );
      int  val;
      PVAL () :   fcn ( 0 ),  val ( 0 ) 
      {
      }
    };

  };
#endif

};

#include "opcodes.h"
