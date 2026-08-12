/*
*
* rtl.h
*
* Copyright (c) 2001, 2002
* Andrew Fedoniouk - andrew@terra-informatica.org
* Portions: Serge Kuznetsov -  kuznetsov@deeptown.org
*
* See the file "COPYING" for information on usage 
* and redistribution of this file
*
*/
#ifndef __rtl_h
#define __rtl_h

#include "c-smile.h"
#include "vm.h"
#include "tool.h"
#include "streams.h"

namespace c_smile
{
#define argcount(n,cnt)  { if ( (n ) != ( cnt ) ) VM::wrongcnt ( n, cnt ); }
#define _this_  argv [-1]
#define chktype(o,t)  { if ( argv [ o ].v_type != t ) badtype ( argv [ o ], t ); }

  char *  nameoftype ( int type );
  void    badtype ( VALUE& vl, int type ); // badtype - report a bad operand type

  void    error_parameters ();
  void    error_read_only ();
  void    error_type ( CLASS *cls );

  string  format ( const char *fmt, int argc, VALUE *argv );

  class ARRAY_CLASS: public CLASS
  {
  public:
    ARRAY_CLASS ( PACKAGE* package ) : CLASS ( "array", 0, package  )
    {
      add_function ( "toString", tostring );
      add_function ( "[]",       item     );
      add_function ( "array",    ctor     );
      add_property ( "length",   length   );
      add_function ( "push",     push     );
      add_function ( "pop",      pop      );
      add_function ( "sort",     sort     );
      add_function ( "slice",    slice    );
      add_function ( "concat",   concat   );
      add_function ( "shift",    shift    );
      add_function ( "unshift",  unshift  );
      add_function ( "clear",    clear    );
      add_static_function ( "[", literal  );
      add_function ( "remove",   remove   );
    }

    static VALUE ctor     ( int argc, VALUE *argv );
    static VALUE length   ( int argc, VALUE *argv );
    static VALUE push     ( int argc, VALUE *argv );
    static VALUE pop      ( int argc, VALUE *argv );
    static VALUE sort     ( int argc, VALUE *argv );
    static VALUE slice    ( int argc, VALUE *argv );
    static VALUE concat   ( int argc, VALUE *argv );
    static VALUE shift    ( int argc, VALUE *argv );
    static VALUE unshift  ( int argc, VALUE *argv );
    static VALUE tostring ( int argc, VALUE *argv );
    static VALUE item     ( int argc, VALUE *argv );
    static VALUE literal  ( int argc, VALUE *argv );
    static VALUE clear    ( int argc, VALUE *argv );
    static VALUE remove   ( int argc, VALUE *argv );

  };

  class STRING_CLASS: public CLASS
  {
  public:
    STRING_CLASS ( PACKAGE* package ) : CLASS ( "string", 0, package  )
    {
      add_function ( "toString", tostring );
      add_function ( "string",   ctor     );
      add_property ( "length",   length   );
      add_function ( "substr",   substr   );
      add_function ( "slice",    slice    );
      add_function ( "to_lower", tolcase  );
      add_function ( "to_upper", toucase  );
      add_function ( "like",     like     );
      add_function ( "match",    match    );
      add_function ( "[]",       item     );
      add_function ( "clear",    clear    );
      add_function ( "split",    split    );
      add_function ( "trim",     trim     );
      add_function ( "replace",  replace  );
      add_static_function ( "printf", printf );
      add_static_function ( "cast", cast  );
    }

    static VALUE ctor     ( int argc, VALUE *argv );
    static VALUE length   ( int argc, VALUE *argv );
    static VALUE tostring ( int argc, VALUE *argv );
    static VALUE substr   ( int argc, VALUE *argv );
    static VALUE slice    ( int argc, VALUE *argv );
    static VALUE tolcase  ( int argc, VALUE *argv );
    static VALUE toucase  ( int argc, VALUE *argv );
    static VALUE like     ( int argc, VALUE *argv );
    static VALUE match    ( int argc, VALUE *argv );
    static VALUE item     ( int argc, VALUE *argv );
    static VALUE clear    ( int argc, VALUE *argv );
    static VALUE printf   ( int argc, VALUE *argv );
    static VALUE split    ( int argc, VALUE *argv );
    static VALUE cast     ( int argc, VALUE *argv );
    static VALUE trim     ( int argc, VALUE *argv );
    static VALUE replace  ( int argc, VALUE *argv );

  };

  class DATE: public CLASS
  {
  public:

    class  INSTANCE : public THING
    {
    public:
      date_time _dt;

      INSTANCE ()
      {
      }

      ~INSTANCE ()
      {
      }

      CLASS *
        get_class ()
      {
        return klass;
      }

      virtual size_t
        allocated_size ()
      {
        return sizeof ( INSTANCE );
      }

      static CLASS * klass;
    };


    DATE ( PACKAGE* package ) : CLASS ( "date", 0, package )
    {
      add_function ( "toString",     tostring    );
      add_function ( "date",         ctor        );
      add_property ( "day",          day         );
      add_property ( "month",        month       );
      add_property ( "year",         year        );
      add_property ( "day_of_week",  day_of_week );
      add_property ( "day_of_year",  day_of_year );
      add_property ( "hours",        hours       );
      add_property ( "minutes",      minutes     );
      add_property ( "seconds",      seconds     );
      add_property ( "time",         abstime     );
      add_static_function ( "cast",  cast        );
      add_function ( "format",       format      );
      add_property ( "is_leap_year", is_leap     );

      INSTANCE::klass = this;
    }

    static VALUE ctor        ( int argc, VALUE *argv );
    static VALUE tostring    ( int argc, VALUE *argv );
    static VALUE day         ( int argc, VALUE *argv );
    static VALUE month       ( int argc, VALUE *argv );
    static VALUE year        ( int argc, VALUE *argv );
    static VALUE day_of_week ( int argc, VALUE *argv );
    static VALUE day_of_year ( int argc, VALUE *argv );
    static VALUE hours       ( int argc, VALUE *argv );
    static VALUE minutes     ( int argc, VALUE *argv );
    static VALUE is_leap     ( int argc, VALUE *argv );
    static VALUE seconds     ( int argc, VALUE *argv );
    static VALUE abstime     ( int argc, VALUE *argv );
    static VALUE cast        ( int argc, VALUE *argv );
    static VALUE format      ( int argc, VALUE *argv );

  };

  class MAP: public CLASS
  {
  public:
    class  INSTANCE : public THING
    {
    public:
      dictionary<VALUE, VALUE> _map;

      INSTANCE  ()
      {
      }

      ~INSTANCE  ()
      {
      }

      CLASS *
        get_class ()
      {
        return klass;
      }

      virtual void mark ();

      virtual size_t
        allocated_size ()
      {
        return sizeof ( INSTANCE );
      }

      static CLASS * klass;
    };

    MAP ( PACKAGE* package ) : CLASS ( "map", 0, package  )
    {
      add_function ( "toString", tostring );
      add_function ( "map",      ctor     );
      add_function ( "add",      add      );
      add_function ( "exist",    exist    );
      add_function ( "[]",       item     );
      add_function ( "remove",   remove   );
      add_function ( "clear",    clear    );
      add_property ( "length",   length   );
      add_static_function ( "{", literal  );
      add_function ( "key",      key      );
      add_function ( "value",    value    );
      INSTANCE::klass = this;
    }

    static VALUE    ctor     ( int argc, VALUE *argv );
    static VALUE    tostring ( int argc, VALUE *argv );
    static VALUE    add      ( int argc, VALUE *argv );
    static VALUE    exist    ( int argc, VALUE *argv );
    static VALUE    item     ( int argc, VALUE *argv );
    static VALUE    remove   ( int argc, VALUE *argv );
    static VALUE    clear    ( int argc, VALUE *argv );
    static VALUE    length   ( int argc, VALUE *argv );
    static VALUE    literal  ( int argc, VALUE *argv );
    static VALUE    key      ( int argc, VALUE *argv );
    static VALUE    value    ( int argc, VALUE *argv );

    virtual THING * load     ( archive *arc );
    virtual bool    save     ( THING *me, archive *arc );

  };


#define MAX_STRING_BUFFER 0x7FFF

  class STREAM: public CLASS
  {
  public:
    class  INSTANCE : public THING
    {
    public:
      THING *     base; // base object: string, file, socket
      io_stream * ios;
      int         last_match_arg; // index of arg in get ( arg0, arg1, arg2, ... )
      bool        is_eof;
      string      buf;

      virtual void
        mark ()
      {
        THING::mark ();
        mark_thing ( base );
      }

      INSTANCE ( io_stream * srcs, THING * src = 0 ) : base ( src ),
                                                       ios ( srcs ),
                                                       last_match_arg ( -1 )

      {
        is_eof = ( srcs == 0 );
      }

      ~INSTANCE  ()
      {
        if ( ios )
          ios->close ();
        delete ios;
        ios = 0;
      }

      CLASS *
        get_class ()
      {
        return klass;
      }

      virtual size_t
        allocated_size  ()
      {
        return sizeof ( INSTANCE );
      }

      bool get ( char &c );
      void unget ( char c );

      static CLASS * klass;
    };

    STREAM ( PACKAGE* package );

    static VALUE put       ( int argc, VALUE *argv );
    static VALUE printf    ( int argc, VALUE *argv );
    static VALUE get       ( int argc, VALUE *argv );
    static VALUE read      ( int argc, VALUE *argv );
    static VALUE write     ( int argc, VALUE *argv );
    static VALUE size      ( int argc, VALUE *argv );
    static VALUE position  ( int argc, VALUE *argv );
    static VALUE close     ( int argc, VALUE *argv );
    static VALUE get_match ( int argc, VALUE *argv );
    static VALUE src       ( int argc, VALUE *argv );
    static VALUE eof       ( int argc, VALUE *argv );

    static void fill_params ( array<string>& params, int argc, VALUE *argv );

  };

  class NODE: public CLASS
  {
  public:
    class INSTANCE : public THING
    {
    public:
      string path;

      INSTANCE  ()
      {
      }

      INSTANCE ( const char *p ) : path ( p )
      {
      }

      ~INSTANCE  ()
      {
      }

      CLASS *
        get_class ()
      {
        return klass;
      }

      virtual size_t
        allocated_size ()
      {
        return sizeof ( INSTANCE );
      }

      static CLASS * klass;
    };

    NODE ( PACKAGE* package ) : CLASS ( "node", 0, package )
    {
      add_function ( "toString",    tostring    );
      add_function ( "node",        ctor        );
      add_function ( "remove",      remove      );
      add_function ( "make_folder", make_folder );
      add_function ( "rename",      rename      );
      add_property ( "path",        path        );

      add_function ( "stream",      stream      );
      add_function ( "nodes",       nodes       );

      add_property ( "size",        size        );

      add_property ( "ctime",       ctime       );
      add_property ( "mtime",       mtime       );
      add_property ( "atime",       atime       );

      add_property ( "drive",       drive       );
      add_property ( "dir",         dir         );
      add_property ( "name",        name        );
      add_property ( "extension",   extension   );
      add_property ( "name_extension", name_extension );
      add_property ( "exist",       exist       );

      add_property ( "is_file",     is_file     );
      add_property ( "is_folder",   is_folder   );

      add_property ( "can_read",    can_read    );
      add_property ( "can_write",   can_write   );
      add_property ( "can_exec",    can_exec    );
      add_property ( "is_pipe",     is_pipe     );
      add_property ( "is_link",     is_link     );

#ifdef _WIN32
      add_static_data ( "path_separator", VALUE ( "\\" ) );
#else
      add_static_data ( "path_separator", VALUE ( "/" ) );
#endif

      //enum access_mode
      add_static_data ( "a_read",      VALUE ( int ( sal::file::fa_read ) ) );
      add_static_data ( "a_write",     VALUE ( int ( sal::file::fa_write ) ) );
      add_static_data ( "a_readwrite", VALUE ( int ( sal::file::fa_readwrite ) ) );

      //enum open_mode
      add_static_data ( "o_truncate",  VALUE ( int ( sal::file::fo_truncate ) ) );
      add_static_data ( "o_create",    VALUE ( int ( sal::file::fo_create ) ) );
      add_static_data ( "o_random",    VALUE ( int ( sal::file::fo_random ) ) );
      add_static_data ( "o_exclusive", VALUE ( int ( sal::file::fo_exclusive ) ) );
      add_static_data ( "o_shared",    VALUE ( int ( sal::file::fo_shared ) ) );

      INSTANCE::klass = this;
    }
    static VALUE ctor         ( int argc, VALUE *argv );
    static VALUE size         ( int argc, VALUE *argv );
    static VALUE stream       ( int argc, VALUE *argv );
    static VALUE nodes        ( int argc, VALUE *argv );
    static VALUE ctime        ( int argc, VALUE *argv );
    static VALUE mtime        ( int argc, VALUE *argv );
    static VALUE atime        ( int argc, VALUE *argv );
    static VALUE drive        ( int argc, VALUE *argv );
    static VALUE dir          ( int argc, VALUE *argv );
    static VALUE name         ( int argc, VALUE *argv );
    static VALUE name_extension ( int argc, VALUE *argv );
    static VALUE extension    ( int argc, VALUE *argv );
    static VALUE exist        ( int argc, VALUE *argv );
    static VALUE remove       ( int argc, VALUE *argv );
    static VALUE path         ( int argc, VALUE *argv );
    static VALUE is_file      ( int argc, VALUE *argv );
    static VALUE is_folder    ( int argc, VALUE *argv );
    static VALUE tostring     ( int argc, VALUE *argv );
    static VALUE make_folder  ( int argc, VALUE *argv );
    static VALUE rename       ( int argc, VALUE *argv );
    static VALUE do_mode_prop ( int argc, VALUE *argv, int mask );
    static VALUE can_read     ( int argc, VALUE *argv );
    static VALUE can_write    ( int argc, VALUE *argv );
    static VALUE can_exec     ( int argc, VALUE *argv );
    static VALUE is_pipe      ( int argc, VALUE *argv );
    static VALUE is_link      ( int argc, VALUE *argv );

    static void error_ne      ( INSTANCE *me );
    static void error_nf      ( INSTANCE *me );
    static void error_nd      ( INSTANCE *me );
    static void error_access  ( INSTANCE *me );

  };


  class SOCKET: public CLASS
  {
  public:
    class INSTANCE : public THING
    {
    public:
      sal::socket_t *              socket;
      sal::socket_t::socket_domain domain;
      bool                         server;     // listening socket
      unsigned int                 streams;

      INSTANCE () : socket ( 0 ), streams ( 0 ), server ( false ),
                    domain ( sal::socket_t::sock_any_domain )
      {
      }

      ~INSTANCE  ()
      {
      }

      CLASS *
        get_class ()
      {
        return klass;
      }

      virtual size_t
        allocated_size ()
      {
        return sizeof ( INSTANCE );
      }

      static CLASS * klass;
    };

    SOCKET ( PACKAGE* package );

    static VALUE socket       ( int argc, VALUE *argv );
    static VALUE accept       ( int argc, VALUE *argv );
    static VALUE connect      ( int argc, VALUE *argv );
    static VALUE close        ( int argc, VALUE *argv );
    static VALUE stream       ( int argc, VALUE *argv );
    static VALUE remote_addr  ( int argc, VALUE *argv );
    static VALUE remote_port  ( int argc, VALUE *argv );
    static VALUE addr         ( int argc, VALUE *argv );
    static VALUE port         ( int argc, VALUE *argv );
    static VALUE name_by_addr ( int argc, VALUE *argv );
    static VALUE addr_by_name ( int argc, VALUE *argv );
    static VALUE hostname     ( int argc, VALUE *argv );
    static VALUE shutdown     ( int argc, VALUE *argv );

  };


  class BLOB: public CLASS
  {
  public:
    class INSTANCE : public THING
    {
    public:
      BUFFER data;

      INSTANCE  ()
      {
      }

      ~INSTANCE  ()
      {
      }

      CLASS *
        get_class ()
      {
        return klass;
      }

      virtual size_t
        allocated_size ()
      {
        return sizeof ( INSTANCE );
      }

      static CLASS * klass;
    };

    BLOB ( PACKAGE* package ) : CLASS ( "blob", 0, package  )
    {
      add_function ( "blob",   ctor   );
      add_property ( "length", length );
      add_function ( "[]",     item   );
      INSTANCE::klass = this;
    }

    static VALUE ctor      ( int argc, VALUE *argv );
    static VALUE length    ( int argc, VALUE *argv );
    static VALUE item      ( int argc, VALUE *argv );
    static VALUE item_type ( int argc, VALUE *argv );

  };

  class REGEXP: public CLASS
  {
  public:
    class  INSTANCE : public THING
    {
      friend class REGEXP;

      string expression;
      bool   compiled;
      regexp re;

    public:

      INSTANCE  () : compiled ( false )
      {
      }

      ~INSTANCE  ()
      {
      }

      CLASS *
        get_class ()
      {
        return klass;
      }

      virtual size_t
        allocated_size ()
      {
        return sizeof ( INSTANCE );
      }
      static CLASS * klass;
    };

    REGEXP ( PACKAGE* package ) : CLASS ( "regexp", 0, package )
    {
      add_function ( "toString", tostring );
      add_function ( "regexp",   ctor     );
      add_function ( "match",    match    );
      add_function ( "test",     test     );
      add_function ( "compile",  compile  );
      add_function ( "[]",       item     );
      add_property ( "length",   length   );
      add_function ( "split",    split    );

      INSTANCE::klass = this;
    }

    static VALUE ctor     ( int argc, VALUE *argv );
    static VALUE tostring ( int argc, VALUE *argv );
    static VALUE match    ( int argc, VALUE *argv );
    static VALUE test     ( int argc, VALUE *argv );
    static VALUE compile  ( int argc, VALUE *argv );
    static VALUE item     ( int argc, VALUE *argv );
    static VALUE length   ( int argc, VALUE *argv );
    static VALUE split    ( int argc, VALUE *argv );

  };

};

#endif
