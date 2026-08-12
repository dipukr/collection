/*
*
* vm.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/

#ifndef __cs_VM_H
#define __cs_VM_H

#include "c-smile.h"
#include "opcodes.h"
#include "mm.h"
#include "sym_table.h"


namespace c_smile
{
  class console_stream;
  class VM;

  class THREAD: public CLASS
  {
  public:
    class INSTANCE: public THING
    {
    public:
      VM *vm;

      INSTANCE () : vm ( 0 )
      {
      }

      INSTANCE ( CODE *code, int argc = 0, VALUE *argv = 0 );
      INSTANCE ( CODE *code, THING *object, int argc = 0, VALUE *argv = 0 );

      ~INSTANCE ();

      virtual size_t
        allocated_size ()
      {
        return sizeof ( INSTANCE );
      }

      CLASS *
        get_class ()
      {
        return klass;
      }
      static CLASS * klass;
    };

  public:
    THREAD ( PACKAGE *pkg );
    static VALUE ctor    ( int argc, VALUE *argv );
    static VALUE stop    ( int argc, VALUE *argv );
    static VALUE running ( int argc, VALUE *argv );
    static VALUE send    ( int argc, VALUE *argv );
    static VALUE wait    ( int argc, VALUE *argv );
    static VALUE sleep   ( int argc, VALUE *argv );
  };

  class MUTEX: public CLASS
  {
  public:
    class INSTANCE: public THING
    {
    public:
      sal::mutex _m;

      INSTANCE  ()
      {
      }

      ~INSTANCE  ()
      {
      }

      virtual size_t
        allocated_size ()
      {
        return sizeof ( INSTANCE );
      }

      CLASS *
        get_class ()
      {
        return klass;
      }
      static CLASS * klass;
    };
  public:
    MUTEX ( PACKAGE *pkg );

    static VALUE ctor ( int argc, VALUE *argv );

  };

  class MSG_QUEUE: public list<VALUE>
  {
  private:
    sal::mutex   _mutex;
    sal::eventex _event;
    int     _awaiting;
  public:
    MSG_QUEUE () : _mutex (), _event ( _mutex ), _awaiting ( 0 )
    {
    }

    void send ( const VALUE &v );
    bool wait ( VALUE &v, int timeout = -1 );
    void mark ();
    bool
      is_waiting ()
    {
      return _awaiting != 0;
    }
  };

  typedef MUTEX::INSTANCE * MUTEX_PTR;

  const symbol_t sym_to_string  = 0;
  const symbol_t sym_value_of   = 1;
  const symbol_t sym_item       = 2;
  const symbol_t sym_cast       = 3;
  const symbol_t sym_run        = 4;
  const symbol_t sym_finalize   = 5;

  const symbol_t sym_std_last   = 5;

  class VM
  {
    friend class compiler;
    friend class MEMORY;
    friend class THREAD;
    friend class THREAD::INSTANCE;
    friend class VM_RTE;
    friend class MSG_QUEUE;

  private:

    unsigned char *	cbase;	      // the base code address
    unsigned char *	pc;		        // the program counter
    CODE *	        code;	        // current code vector
    CODE *	        native_code;	// current native code

    VALUE *			    stkbase;    // the runtime stack
    VALUE *			    stktop;	    // the top of the stack
    VALUE *			    sp;		      // the stack pointer
    VALUE *			    fp;		      // the frame pointer

    VALUE           t_register; // temp register

    int             line_num;   // line number ( debuging mode )

    THREAD::INSTANCE * thread;
    bool            running;
    sal::task *     task;
  public:
    MSG_QUEUE       queue;
  private:
    void	          opCALL ();
    bool	          opRETURN ();
    void	          opVREF ();
    void	          opVSET ();
    void	          opADD ();
    void	          opNEW ();
    void            opPMREF ();
    void            opPMSET ();

    bool            start_init ( PACKAGE *pkg );
    bool	          start_call ( PACKAGE *package, symbol_t sym, int argc = 0, VALUE *argv = 0 );
    bool	          start_send ( OBJECT *obj, symbol_t sym, int argc = 0, VALUE *argv = 0 );

    void	          interpret ( int );

    int		          getwoperand ();
    void	          badtype ( int off, int type );
    void	          stackover ();

    void            setup_stack ()  { sp = fp = stkbase; }
    string          stack_trace ();

    struct error_handler
    {
      CODE *	        code;	      // the code vector
      VALUE *			    sp;		      // the stack pointer
      VALUE *			    fp;		      // the frame pointer
      int             m_stack_pos;// synchro stack pos
      int             catch_pc;   //
      VALUE           thrown;     // by throw operator
    };

    error_handler * eh;
    int             eh_pos;
    int             eh_size;

    MUTEX_PTR *     m_stack;
    int             m_stack_pos;
    int             m_stack_size;

    PACKAGE *       package;     // "current" package

    unsigned long   thread_id;

    sal::event      gc_done;
    bool            ready_for_gc;

  public:

    VM ( int smax = SMAX, int eh_smax = ERROR_HANDLER_SMAX,
         int m_smax = SYNCHRO_SMAX );

    ~VM ();

    void
      traceMode ( bool onoff )
    {
      trace = onoff;
    }

    void
      decodeMode ( bool onoff )
    {
      decode = onoff;
    }

    int
      get_line_num ()
    {
      return line_num;
    }

    string          get_file_name ();

    bool            start_call ( CODE *c, THING *object, int argc = 0, VALUE *argv = 0 );
    bool            start_call ( CODE *c, int argc = 0, VALUE *argv = 0 );
    bool            start_call ( CODE *c, VALUE& v_this, int argc = 0, VALUE *argv = 0 );
    bool	          execute_call ();

    VALUE           call ( CODE *c, int argc = 0, VALUE *argv = 0 );
    VALUE           call ( VALUE& c, int argc = 0, VALUE *argv = 0 );
    VALUE           call ( CODE *c, VALUE& v_this, int argc = 0, VALUE *argv = 0 );

    VALUE           invoke_native ( CODE *c, int argc, VALUE* argv );

    VALUE           send ( VALUE& v, symbol_t sym_method, int argc = 0, VALUE *argv = 0 );

    bool	          execute ( PACKAGE *package, const char *name, int argc = 0, VALUE *argv = 0 );
    bool            execute_init ( PACKAGE *package );

    static void     checktype ( VALUE& vl, int type, int number );

    void            attach_thread ();  // attach OS thread to this VM
    void            create_thread ();  // create OS thread for this VM

    void            mark ();           // mark this vm

    //static data:

    static sym_table        voc;      // symbols vocabulary

    static PACKAGE *        std;          // standard package
    static DICTIONARY *     packages;     // loaded packages
    static  CLASS *         class_string;
    static  CLASS *         class_array;

    static console_stream * sin;
    static console_stream * sout;
    static console_stream * serr;

    static VALUE            null;		  // the null value
    static VALUE            undefined;// the undefined value

    static  list<VM*>       all;      // VM pool
    static  sal::mutex      all_guard;// VM pool guard

    static  const char *    dll_ext;  // dll file extensions
    static  const char *    PATH_SEP;

#ifdef _WIN32
    static unsigned int     thread_slot_id; // for TlsGetValue/TlsSetValue
#else
    static pthread_key_t    thread_slot_id;
#endif

    static bool			        trace;	    // variable to control tracing
    static bool             decode;     // show generated asm

    static string           app_path;
    static string           start_path;

    static array<string>    lib_paths;

    //static methods:

    static  void      mark_all ();     // mark all VMs

    static  VM*       current ();
    static  VM*       main    ()  { return all.head (); }

    static  bool      add_package ( PACKAGE * package );
    static  bool      run_init_code ( PACKAGE * package ); 

    static  PACKAGE * load_file ( const char *name );
    static  PACKAGE * compile_file ( const char *name );
    static  void      make_bundle ( const PACKAGE *mainp, const char *name, bool make_exe );
    static  PACKAGE * load_attachment ();

    static  int       arguments ( int argc, VALUE *argv, const char *format, ... );

    static  void      error ( const char *fmt, ... );
    static  void      throw_error ( const VALUE& v );
    static  void	    wrongcnt ( int n, int cnt );

    static void	      info ( char *fmt, ... );

    static  CLASS *   find_class ( symbol_t package_name, symbol_t class_name );
    static  PACKAGE * find_package ( symbol_t package_name, const char *dir = 0 );
    static  void      mark_std_classes ();
    static  void      init_std_package ();
    static  CLASS *   get_class ( const VALUE *v );

    static  bool      load_native_module ( const char *module_name );

#ifdef DECODE_TRACE
    // decode_procedure - decode the instructions in a code object
    static void       decode_procedure ( CODE *code );
    // decode_instruction - decode a single bytecode instruction
    static int        decode_instruction ( CODE *code, int lc );
#endif

    static  bool      initialize ( const char *appname );
    static  bool      terminate ();

    static  void      make_exe ( PACKAGE *pkg, const char *filename );

  };

  inline int
    VM::getwoperand ()
  {
    int b; b = *pc++; return ( ( *pc++ << 8 ) | b );
  }


  // Runtime error
  class VM_RTE
  {
  public:
    VALUE     err_value;
    string    description;
    string    source;
    string    function_name;
    int       line_no;

  public:

    VM_RTE  ()
    {
    }
    VM_RTE ( VM *vm, const char *msg );
    VM_RTE ( VM *vm, const VALUE& ev );
    string report ();
  };

};

#endif
